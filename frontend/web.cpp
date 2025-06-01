#include "web.hpp"
#include <fstream>
#include <filesystem>

#define BG_PURPL "\033[45;37m"
#define RESET "\033[0m"

WebServer::WebServer(int port) : port(port), state(STOPPED), server(nullptr) {
    this->port = findAvailablePort(port, 10);
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    lock_guard<mutex> lock(stateMutex);
    
    if (state == RUNNING) {
        log("服务器已在运行");
        return true;
    }
    
    setState(STARTING);
    
    try {
        server = make_unique<httplib::Server>();
        setupRoutes();
        
        serverThread = make_unique<thread>([this]() {
            log("启动Web服务器: http://localhost:" + to_string(port));
            setState(RUNNING);
            
            if (!server->listen("0.0.0.0", port)) {
                log("服务器启动失败");
                setState(STOPPED);
            } else {
                log("服务器已停止");
                setState(STOPPED);
            }
        });
        
        // 等待服务器启动
        this_thread::sleep_for(chrono::milliseconds(100));
        return state == RUNNING;
        
    } catch (const exception& e) {
        log("服务器启动异常: " + string(e.what()));
        setState(STOPPED);
        return false;
    }
}

void WebServer::stop() {
    lock_guard<mutex> lock(stateMutex);
    
    if (state == STOPPED) return;
    
    setState(STOPPING);
    
    if (server) {
        server->stop();
    }
    
    if (serverThread && serverThread->joinable()) {
        serverThread->join();
    }
    
    setState(STOPPED);
    log("Web服务器已停止");
}

bool WebServer::isRunning() const {
    lock_guard<mutex> lock(stateMutex);
    return state == RUNNING;
}

WebServer::ServerState WebServer::getState() const {
    lock_guard<mutex> lock(stateMutex);
    return state;
}

void WebServer::setCoreInstance(shared_ptr<Core> coreInstance) {
    core = coreInstance;
    
    // 设置消息处理器来接收来自core的消息
    if (core) {
        core->setMessageHandler([this](const string& message) {
            this->onMessageReceived(message);
        });
    }
    
    log("Core实例已设置，消息处理器已配置");
}

void WebServer::setupRoutes() {
    // 设置静态文件服务
    server->set_mount_point("/", "./frontend/public");
    
    // API路由
    server->Get("/api/status", [this](const httplib::Request& req, httplib::Response& res) {
        handleStatus(req, res);
    });
    
    server->Post("/api/setting", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetting(req, res);
    });
    
    server->Post("/api/send", [this](const httplib::Request& req, httplib::Response& res) {
        handleSendMessage(req, res);
    });
    
    server->Get("/api/receive", [this](const httplib::Request& req, httplib::Response& res) {
        handleReceiveMessage(req, res);
    });
    
    server->Get("/api/messages", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetMessages(req, res);
    });
    
    log("路由设置完成");
}

void WebServer::handleStatus(const httplib::Request& req, httplib::Response& res) {
    json response;
    response["status"] = "running";
    response["port"] = port;
    response["timestamp"] = getCurrentTime();
    
    if (core) {
        response["core_status"] = "connected";
    } else {
        response["core_status"] = "disconnected";
    }
    
    sendJsonResponse(res, response);
}

void WebServer::handleSetting(const httplib::Request& req, httplib::Response& res) {
    try {
        json requestData = json::parse(req.body);
        
        if (!core) {
            json errorResponse;
            errorResponse["error"] = "Core实例未设置";
            sendJsonResponse(res, errorResponse, 500);
            return;
        }
        
        string mode = requestData.value("mode", "");
        string ip = requestData.value("ip", "localhost");
        int port = requestData.value("port", 8848);
        
        bool success = false;
        string errorMsg = "";
        
        if (mode == "server") {
            success = core->startServer(ip, port);
            if (!success) {
                errorMsg = "启动服务器模式失败";
            }
        } else if (mode == "client") {
            success = core->startClient(ip, port);
            if (!success) {
                errorMsg = "启动客户端模式失败";
            }
        } else {
            json errorResponse;
            errorResponse["error"] = "无效的模式，请选择 'server' 或 'client'";
            sendJsonResponse(res, errorResponse, 400);
            return;
        }
        
        json response;
        if (success) {
            response["success"] = true;
            response["message"] = "设置已更新，" + mode + "模式已启动";
            response["mode"] = mode;
            response["ip"] = ip;
            response["port"] = port;
            sendJsonResponse(res, response);
            log("设置更新成功: 模式=" + mode + ", IP=" + ip + ", 端口=" + to_string(port));
        } else {
            response["success"] = false;
            response["error"] = errorMsg;
            sendJsonResponse(res, response, 500);
            log("设置更新失败: " + errorMsg);
        }
        
    } catch (const exception& e) {
        json errorResponse;
        errorResponse["error"] = "设置请求解析失败: " + string(e.what());
        sendJsonResponse(res, errorResponse, 400);
    }
}

void WebServer::handleSendMessage(const httplib::Request& req, httplib::Response& res) {
    try {
        json requestData = json::parse(req.body);
        string message = requestData.value("message", "");
        
        if (message.empty()) {
            json errorResponse;
            errorResponse["error"] = "消息不能为空";
            sendJsonResponse(res, errorResponse, 400);
            return;
        }
        
        if (!core) {
            json errorResponse;
            errorResponse["error"] = "Core实例未设置";
            sendJsonResponse(res, errorResponse, 500);
            return;
        }
        
        bool success = core->sendMessage(message);
        
        json response;
        if (success) {
            response["success"] = true;
            response["message"] = "消息已发送";
            response["sent_message"] = message;
            response["timestamp"] = getCurrentTime();
            sendJsonResponse(res, response);
            log("发送消息成功: " + message);
        } else {
            response["success"] = false;
            response["error"] = "消息发送失败";
            sendJsonResponse(res, response, 500);
            log("发送消息失败: " + message);
        }
        
    } catch (const exception& e) {
        json errorResponse;
        errorResponse["error"] = "发送消息失败: " + string(e.what());
        sendJsonResponse(res, errorResponse, 400);
    }
}

void WebServer::handleReceiveMessage(const httplib::Request& req, httplib::Response& res) {
    if (!core) {
        json errorResponse;
        errorResponse["error"] = "Core实例未设置";
        sendJsonResponse(res, errorResponse, 500);
        return;
    }
    
    json response;
    response["success"] = true;
    response["message"] = "暂无新消息";
    response["timestamp"] = getCurrentTime();
    
    sendJsonResponse(res, response);
}

void WebServer::handleGetMessages(const httplib::Request& req, httplib::Response& res) {
    lock_guard<mutex> lock(messagesMutex);
    
    json response;
    response["messages"] = json::array();
    
    queue<string> tempQueue = receivedMessages;
    while (!tempQueue.empty()) {
        response["messages"].push_back(tempQueue.front());
        tempQueue.pop();
    }
    
    response["count"] = response["messages"].size();
    response["timestamp"] = getCurrentTime();
    
    sendJsonResponse(res, response);
}

void WebServer::sendJsonResponse(httplib::Response& res, const json& data, int status) {
    res.status = status;
    res.set_header("Content-Type", "application/json");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res.body = data.dump();
}

void WebServer::log(const string& message) const {
    cout << BG_PURPL <<"[WebServer] " << getCurrentTime() << " " 
    << RESET << message << endl;
}

string WebServer::getCurrentTime() const {
    auto now = chrono::system_clock::now();
    auto time_t = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void WebServer::setState(ServerState newState) {
    state = newState;
}

bool WebServer::isPortAvailable(int port) const
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

int WebServer::findAvailablePort(int port, int Maxoffset)
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
    log("No available port found within the specified range.");
    return 0;
}

void WebServer::onMessageReceived(const string& message) {
    lock_guard<mutex> lock(messagesMutex);
    receivedMessages.push(message);
    log("接收到新消息: " + message);
}