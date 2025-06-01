#include "core.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

Core::Core(int bits) 
    : bits(bits), state(DISCONNECTED), running(false), keyExchangeComplete(false) {
    encryptor = make_unique<MessageEncryptor>(bits);
    sessionId = generateSessionId();
    updateLastActivity();
    log("SecureCommunicationCore initialized with " + to_string(bits) + " bits");
}

Core::~Core() {
    stop();
    log("SecureCommunicationCore destroyed", IMPORTANT);
}

bool Core::startServer(const string& host, int port) {
    mode = SERVER;
    port = findAvailablePort(port, 10);
    serverHost = host;
    serverPort = port; 

    setState(CONNECTING);
    
    server = make_unique<httplib::Server>();
    setupServerRoutes();
    
    running = true;
    serverThread = make_unique<thread>([this, host, port]() {
        log("Starting server on " + host + ":" + to_string(port), IMPORTANT);
        if (server->listen(host.c_str(), port)) {
            log("Server started successfully");
        } else {
            log("Failed to start server", ERROR);
            setState(DISCONNECTED);
        }
    });
    
    messageThread = make_unique<thread>([this]() {
        processMessageQueue();
    });
    
    // 等待服务器启动
    this_thread::sleep_for(chrono::milliseconds(500));
    setState(CONNECTED);
    
    return true;
}

bool Core::startClient(const string& host, int port) {
    mode = CLIENT;
    serverHost = host;
    serverPort = port;
    setState(CONNECTING);
    
    client = make_unique<httplib::Client>(host, port);
    running = true;
    
    messageThread = make_unique<thread>([this]() {
        processMessageQueue();
    });
    
    log("Establishing connection to server at " + host + ":" + to_string(port));

    // 测试连接
    auto result = client->Get("/status");
    auto response = json::parse(result->body);
    auto bits_ = response["bits"].get<int>();

    if (bits_ != bits) {
        log("Server bits mismatch: expected " + to_string(bits) + ", got " + to_string(bits_), ERROR);
        setState(DISCONNECTED);
        return false;
    }
    
    if (!result || result->status != 200) {
        log("Failed to connect to server", ERROR);
        setState(DISCONNECTED);
        return false;
    }
    
    setState(CONNECTED);
    
    // 开始密钥交换
    if (!performKeyExchangeAsClient()) {
        log("Key exchange failed");
        setState(DISCONNECTED);
        return false;
    }
    
    // 开始轮询消息
    pollingThread = make_unique<thread>([this]() {
        pollMessages();
    });
    
    return true;
}

void Core::setupServerRoutes() {
    // CORS配置
    server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        // 设置CORS头
        res.set_header("Access-Control-Allow-Origin", "*"); // 跨域支持
        res.set_header("Access-Control-Allow-Methods", "GET, POST");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Max-Age", "86400");
                
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // 密钥交换
    server->Post("/api/key_exchange", [this](const httplib::Request& req, httplib::Response& res) {
        handleKeyExchange(req, res);
    });
    
    // 发送消息
    server->Post("/api/send_message", [this](const httplib::Request& req, httplib::Response& res) {
        handleSendMessage(req, res);
    });
    
    // 接收消息
    server->Get("/api/receive_messages", [this](const httplib::Request& req, httplib::Response& res) {
        handleReceiveMessages(req, res);
    });
    
    // 状态查询
    server->Get("/status", [this](const httplib::Request& req, httplib::Response& res) {
        handleStatus(req, res);
    });
}

void Core::handleStatus(const httplib::Request& req, httplib::Response& res) {
    json response;
    response["status"] = "online";
    response["state"] = static_cast<int>(state);
    response["session_id"] = sessionId;
    response["bits"] = bits;
    sendJsonResponse(res, response);
}

void Core::handleKeyExchange(const httplib::Request& req, httplib::Response& res) {
    try {
        auto requestData = json::parse(req.body);
        string type = requestData["type"];
        
        if (type == "public_key") {
            if (receivePublicKey(requestData["data"])) {
                // 发送自己的公钥作为响应
                mpz_t p, g, y;
                mpz_inits(p, g, y, NULL);
                encryptor->SendPKG(p, g, y);
                
                json responseData;
                responseData["p"] = mpz_get_str(nullptr, 10, p);
                responseData["g"] = mpz_get_str(nullptr, 10, g);
                responseData["y"] = mpz_get_str(nullptr, 10, y);
                
                json response = createMessage("public_key", responseData);
                sendJsonResponse(res, response);
                
                mpz_clears(p, g, y, NULL);
            } else {
                sendJsonResponse(res, {{"error", "Failed to process public key"}}, 400);
            }
        } else if (type == "secret") {
            if (receiveSecret(requestData["data"])) {
                // 发送自己的密钥
                mpz_t c1, c2;
                mpz_inits(c1, c2, NULL);
                encryptor->SendSecret(c1, c2);
                
                json responseData;
                responseData["c1"] = mpz_get_str(nullptr, 10, c1);
                responseData["c2"] = mpz_get_str(nullptr, 10, c2);
                
                json response = createMessage("secret", responseData);
                sendJsonResponse(res, response);
                
                completeKeyExchange();
                mpz_clears(c1, c2, NULL);
            } else {
                sendJsonResponse(res, {{"error", "Failed to process secret"}}, 400);
            }
        } else {
            sendJsonResponse(res, {{"error", "Unknown key exchange type"}}, 400);
        }
    } catch (const exception& e) {
        log("Error in key exchange: " + string(e.what()));
        sendJsonResponse(res, {{"error", "Invalid request format"}}, 400);
    }
}

void Core::handleSendMessage(const httplib::Request& req, httplib::Response& res) {
    if (state != READY) {
        sendJsonResponse(res, {{"error", "Not ready for communication"}}, 400);
        return;
    }
    
    try {
        auto requestData = json::parse(req.body);
        string encryptedMessage = requestData["encrypted_message"];
        
        string decryptedMessage;
        encryptor->DecryptMessage(encryptedMessage, decryptedMessage);
        
        log("Received encrypted message, decrypted: " + decryptedMessage);
        
        // 将消息添加到入站队列
        {
            lock_guard<mutex> lock(messageMutex);
            incomingMessageQueue.push(decryptedMessage);
        }
        
        // 通知消息处理器
        if (messageHandler) {
            messageHandler(decryptedMessage);
        }
        
        sendJsonResponse(res, {{"status", "success"}});
        updateLastActivity();
    } catch (const exception& e) {
        log("Error handling message: " + string(e.what()));
        sendJsonResponse(res, {{"error", "Failed to process message"}}, 400);
    }
}

void Core::handleReceiveMessages(const httplib::Request& req, httplib::Response& res) {
    json messages = json::array();
    
    {
        lock_guard<mutex> lock(serverMessageMutex);
        while (!serverToClientMessages.empty()) {
            messages.push_back(serverToClientMessages.front());
            serverToClientMessages.pop();
        }
    }
    
    sendJsonResponse(res, {{"messages", messages}});
    updateLastActivity();
}

bool Core::performKeyExchangeAsClient() {
    auto startTime = chrono::steady_clock::now();
    setState(KEY_EXCHANGING);
    log("Starting key exchange as client");
    
    // 发送公钥
    if (!sendPublicKey()) {
        return false;
    }
    
    // 发送密钥
    if (!sendSecret()) {
        return false;
    }
    
    completeKeyExchange();
    auto endTime = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    cout << GREEN << "Key exchange completed in " << duration << " ms" << RESET << endl;
    return true;
}

bool Core::sendPublicKey() {
    mpz_t p, g, y;
    mpz_inits(p, g, y, NULL);
    
    encryptor->SendPKG(p, g, y);
    
    json data;
    data["p"] = mpz_get_str(nullptr, 10, p);
    data["g"] = mpz_get_str(nullptr, 10, g);
    data["y"] = mpz_get_str(nullptr, 10, y);
    
    json message = createMessage("public_key", data);
    
    auto result = client->Post("/api/key_exchange", message.dump(), "application/json");
    
    if (result && result->status == 200) {
        try {
            auto response = json::parse(result->body);
            bool success = receivePublicKey(response["data"]);
            
            log("Sent public key and received response");
            mpz_clears(p, g, y, NULL);
            return success;
        } catch (const exception& e) {
            log("Error parsing public key response: " + string(e.what()), WARNING);
        }
    } else {
        log("Failed to send public key", WARNING);
    }
    
    mpz_clears(p, g, y, NULL);
    return false;
}

bool Core::receivePublicKey(const json& data) {
    try {
        mpz_t p, g, y;
        mpz_inits(p, g, y, NULL);
        
        mpz_set_str(p, data["p"].get<string>().c_str(), 10);
        mpz_set_str(g, data["g"].get<string>().c_str(), 10);
        mpz_set_str(y, data["y"].get<string>().c_str(), 10);
        
        encryptor->ReceivePKG(p, g, y);
        
        string pStr = data["p"].get<string>();
        string suffix = pStr.length() >= 20 ? pStr.substr(pStr.length() - 20) : pStr;
        log("Received public key: p=..." + suffix);
      
        mpz_clears(p, g, y, NULL);
        return true;
    } catch (const exception& e) {
        log("Error processing public key: " + string(e.what()), WARNING);
        return false;
    }
}

bool Core::sendSecret() {
    mpz_t c1, c2;
    mpz_inits(c1, c2, NULL);
    
    encryptor->SendSecret(c1, c2);
    
    json data;
    data["c1"] = mpz_get_str(nullptr, 10, c1);
    data["c2"] = mpz_get_str(nullptr, 10, c2);
    
    json message = createMessage("secret", data);
    
    auto result = client->Post("/api/key_exchange", message.dump(), "application/json");
    
    if (result && result->status == 200) {
        try {
            auto response = json::parse(result->body);
            bool success = receiveSecret(response["data"]);
            
            log("Sent secret and received response");
            mpz_clears(c1, c2, NULL);
            return success;
        } catch (const exception& e) {
            log("Error parsing secret response: " + string(e.what()), WARNING);
        }
    } else {
        log("Failed to send secret", WARNING);
    }
    
    mpz_clears(c1, c2, NULL);
    return false;
}

bool Core::receiveSecret(const json& data) {
    try {
        mpz_t c1, c2;
        mpz_inits(c1, c2, NULL);
        
        mpz_set_str(c1, data["c1"].get<string>().c_str(), 10);
        mpz_set_str(c2, data["c2"].get<string>().c_str(), 10);
        
        encryptor->ReceiveSecret(c1, c2);
        
        log("Received secret: c1=" + data["c1"].get<string>().substr(0, 20) + "...");
        
        mpz_clears(c1, c2, NULL);
        return true;
    } catch (const exception& e) {
        log("Error processing secret: " + string(e.what()));
        return false;
    }
}

void Core::completeKeyExchange() {
    {
        lock_guard<mutex> lock(keyExchangeMutex);
        keyExchangeComplete = true;
    }
    
    string key1, key2;
    encryptor->GetSM4Key(key1, key2);
    
    log("Key exchange completed!");
    log("key1 = " + key1);
    log("key2 = " + key2);
    
    setState(READY);
}

bool Core::sendMessage(const string& message) {
    if (state != READY) {
        log("Cannot send message: not ready (current state: " + to_string(state) + ")");
        return false;
    }
    
    {
        lock_guard<mutex> lock(messageMutex);
        outgoingMessageQueue.push(message);
    }
    messageCondition.notify_one();
    
    return true;
}

void Core::processMessageQueue() {
    while (running) {
        unique_lock<mutex> lock(messageMutex);
        messageCondition.wait(lock, [this] { return !outgoingMessageQueue.empty() || !running; });
        
        if (!running) break;
        
        while (!outgoingMessageQueue.empty()) {
            string message = outgoingMessageQueue.front();
            outgoingMessageQueue.pop();
            lock.unlock();
            
            // 加密消息
            string encryptedMessage;
            encryptor->EncryptMessage(message, encryptedMessage);
            
            if (mode == SERVER) {
                // 服务器模式：将消息存储到队列中等待客户端轮询
                lock_guard<mutex> serverLock(serverMessageMutex);
                serverToClientMessages.push(encryptedMessage);
                log("Stored encrypted message for client: " + message);
            } else {
                // 客户端模式：直接发送到服务器
                json requestData;
                requestData["encrypted_message"] = encryptedMessage;
                
                auto result = client->Post("/api/send_message", requestData.dump(), "application/json");
                if (result && result->status == 200) {
                    log("Sent encrypted message: " + message);
                } else {
                    log("Failed to send message: " + message, WARNING);
                }
            }
            
            lock.lock();
        }
    }
}

void Core::pollMessages() {
    while (running && mode == CLIENT) {
        if (state == READY) {
            auto result = client->Get("/api/receive_messages");
            if (result && result->status == 200) {
                try {
                    auto response = json::parse(result->body);
                    auto messages = response["messages"];
                    
                    for (const auto& encryptedMessage : messages) {
                        string decryptedMessage;
                        encryptor->DecryptMessage(encryptedMessage.get<string>(), decryptedMessage);
                        
                        log("Received encrypted message, decrypted: " + decryptedMessage);
                        
                        if (messageHandler) {
                            messageHandler(decryptedMessage);
                        }
                    }
                } catch (const exception& e) {
                    log("Error parsing messages: " + string(e.what()), WARNING);
                }
            }
        }
        
        this_thread::sleep_for(chrono::milliseconds(1000)); // 每秒轮询一次
    }
}

json Core::createMessage(const string& type, const json& data) {
    json message;
    message["type"] = type;
    message["data"] = data;
    message["timestamp"] = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()).count();
    message["session_id"] = sessionId;
    return message;
}

void Core::sendJsonResponse(httplib::Response& res, const json& data, int status) {
    res.status = status;
    res.set_content(data.dump(), "application/json");
}

void Core::setMessageHandler(function<void(const string&)> handler) {
    messageHandler = handler;
}

Core::ConnectionState Core::getState() const {
    lock_guard<mutex> lock(stateMutex);
    return state;
}

bool Core::isConnected() const {
    return getState() == READY;
}

void Core::setState(ConnectionState newState) {
    {
        lock_guard<mutex> lock(stateMutex);
        state = newState;
    }
    stateCondition.notify_all();
}

void Core::waitForConnection() {
    unique_lock<mutex> lock(stateMutex);
    stateCondition.wait(lock, [this] { return state == READY || state == DISCONNECTED; });
}

void Core::stop() {
    running = false;
    
    if (server) {
        server->stop();
    }
    
    messageCondition.notify_all();
    
    if (serverThread && serverThread->joinable()) {
        serverThread->join();
    }
    
    if (messageThread && messageThread->joinable()) {
        messageThread->join();
    }
    
    if (pollingThread && pollingThread->joinable()) {
        pollingThread->join();
    }
    
    setState(DISCONNECTED);
    log("Communication core stopped");
}

void Core::log(const string& message, LogLevel type) const {
    auto now = chrono::system_clock::now();
    auto time_t = chrono::system_clock::to_time_t(now);
    auto tm = *localtime(&time_t);
    
    if (type == ERROR) {
        cout << RED << "[" << put_time(&tm, "%H:%M:%S") << "] " << message << RESET << endl;
    } else if (type == WARNING) {
        cout << YELLOW << "[" << put_time(&tm, "%H:%M:%S") << "] " << message << RESET << endl;
    } else if (type == INFO) {
        cout << BLUE << "[" << put_time(&tm, "%H:%M:%S") << "] " << message << RESET << endl;
    } else if (type == IMPORTANT) {
        cout << GREEN << "[" << put_time(&tm, "%H:%M:%S") << "] " << message << RESET << endl;
    } 
}

string Core::generateSessionId() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(0, 15);
    
    string sessionId;
    for (int i = 0; i < 16; ++i) {
        sessionId += "0123456789ABCDEF"[dis(gen)];
    }
    return sessionId;
}

void Core::updateLastActivity() {
    lastActivity = chrono::steady_clock::now();
}

bool Core::isSessionValid() const {
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::minutes>(now - lastActivity);
    return elapsed.count() < 30; // 30分钟超时
}

bool Core::isPortAvailable(int port) const
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return false;
    }
    
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    bool available = (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
    
    close(sockfd);
    
    return available;
}

int Core::findAvailablePort(int port, int Maxoffset)
{
    for (int offset = 0; offset <= Maxoffset; ++offset) {
        log("Checking port " + to_string(port + offset));
        if (isPortAvailable(port + offset)) {
            log("Port " + to_string(port + offset) + " is available.");
            return port + offset;
        }
        if (isPortAvailable(port - offset) && offset != 0) {
            log("Port " + to_string(port - offset) + " is available.");
            return port - offset;
        }
    }
    log("No available port found within the specified range.", WARNING);
    return 0;
}
