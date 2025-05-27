#include "core.hpp"
#include <sys/socket.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

Core::Core(ServerMode mode, int bits)
{
    mpz_inits(p, g, y, remote_p, remote_g, remote_y, c1, c2, NULL);
    status = ConnectionStatus::DISCONNECTED;
    key_exchange_completed = false;
    destination_host = "localhost";
    destination_port = 8848; 
    // cout << "debug" << endl;
    this->mode = mode;
    encryptor = make_unique<MessageEncryptor>(bits);
    encryptor->SendPKG(p, g, y);
}

bool Core::isPortAvailable(int port)
{
    // 使用socket API检查端口是否可用
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return false;
    }
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    int result = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    
    if (result == 0) {
        cout << BLUE << "端口 " << port << " 可用" << RESET << endl;
        return true;
    } else {
        cout << RED << "端口 " << port << " 不可用: "  << strerror(errno) << RESET << endl;
        return false;
    }
}

int Core::findAvailablePort(int start_port, int max_attempts) {
    for (int i = 0; i < max_attempts; ++i) {
        if (isPortAvailable(start_port + i)) {
            return start_port + i;
        }
    }
    return -1; // 未找到可用端口
}

void Core::setupRoutes()
{
    if (!server) {
        cerr << "服务器未初始化！请先调用 start() 方法。" << endl;
        return;
    }
    server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    // 接受client发送的hello请求
    server->Get("/hello", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content("Hello from server!", "text/plain");
        cout << GREEN << "Received hello request" << RESET << endl;
    });

    // 发送我的PKG
    server->Get("/send_pkg", [this](const httplib::Request&, httplib::Response& res) {
        // 直接转为string
        json j;
        j["p"] = mpz_get_str(NULL, 16, p);
        j["g"] = mpz_get_str(NULL, 16, g);
        j["y"] = mpz_get_str(NULL, 16, y);
        res.set_content(j.dump(), "application/json");
        cout << GREEN << "Sent public key parameters to client" << RESET << endl;
    });

    // 接受client告知的server地址
    server->Post("/where_are_you", [this](const httplib::Request& req, httplib::Response& res) {
        json j = json::parse(req.body);
        string host = j["host"];
        int port = j["port"];
        

        SetDestination(host, port);
        string message = "Server address set to: " + host + ":" + to_string(port);
        res.set_content(message,"text/plain");

        cout << GREEN << "Received server address from client: " << host << ":" << port << RESET << endl;

        SetDestination(host, port);
        client ;
    });
}

void Core::SetDestination(const string &host, int port)
{
    destination_host = host;
    destination_port = port;
    cout << GREEN << "Destination set to " << destination_host << ":" << destination_port << RESET << endl;
}

void Core::clientHello()
{
}

void Core::maintainingServer()
{
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void Core::GetServerPublicKey()
{
    if (!client) {
        cerr << "Client未初始化！请先调用 start() 方法 与 SetDestination() 方法。" << endl;
        return;
    }

    cout << GREEN << "Requesting public key parameters from server..." << RESET << endl;
    auto res = client->Get("/send_pkg");

    if (res && res->status == 200) {
        json j = json::parse(res->body);
        mpz_set_str(p, j["p"].get<string>().c_str(), 16);
        mpz_set_str(g, j["g"].get<string>().c_str(), 16);
        mpz_set_str(y, j["y"].get<string>().c_str(), 16);
        cout << GREEN << "Received public key parameters from server" << RESET << endl;
        cout << "p: " << mpz_get_str(NULL, 16, p) << endl;
        cout << "g: " << mpz_get_str(NULL, 16, g) << endl;
        cout << "y: " << mpz_get_str(NULL, 16, y) << endl;
    } else {
        cerr << RED << "Failed to get public key parameters: HTTP status code " 
             << (res ? to_string(res->status) : "N/A") << RESET << endl;
    }
}

void Core::ConnectionTest()
{
    if (!client) {
        cerr << "Client未初始化！请先调用 start() 方法 与 SetDestination() 方法。" << endl;
        return;
    }

    cout << GREEN << "Testing connection to " << destination_host << ":" << destination_port << RESET << endl;
    auto res = client->Get("/hello");

    if (res && res->status == 200) {
        cout << GREEN << "Connection successful: " << res->body << RESET << endl;
    } else {
        cerr << RED << "Connection failed: HTTP status code " << (res ? to_string(res->status) : "N/A") << RESET << endl;
    }
    cout << GREEN << "Connection test completed." << RESET << endl;
}

void Core::WhereIsMyServer()
{
    if (!client) {
        cerr << "Client未初始化！请先调用 start() 方法 与 SetDestination() 方法。" << endl;
        return;
    }

    json j;
    j["host"] = destination_host;
    j["port"] = destination_port;

    cout << GREEN << "Sending server address to client..." << RESET << endl;
    auto res = client->Post("/where_are_you", j.dump(), "application/json");

    if (res && res->status == 200) {
        cout << GREEN << "Server address sent successfully: " << res->body << RESET << endl;
    } else {
        cerr << RED << "Failed to send server address: HTTP status code " 
             << (res ? to_string(res->status) : "N/A") << RESET << endl;
    }
}

void Core::start()
{
    server = make_unique<httplib::Server>();
    setupRoutes();
    cout << GREEN << "Starting server..." << RESET << endl;
    listen_port = findAvailablePort(8848);
    if (listen_port == -1) {
        cerr << "无法找到可用的监听端口！" << endl;
        return;
    }

    server_thread = thread([this]() {
        cout << GREEN << "Server thread starting..." << RESET << endl;
        server->listen("localhost", listen_port);
    });
    

    this_thread::sleep_for(chrono::milliseconds(100));
    
    cout << GREEN << "Listening on port " << listen_port << RESET << endl;
    if (mode)
    client = make_unique<httplib::Client>(destination_host, destination_port);
}


Core::~Core() {
    if (server) {
        server->stop();
    }
    if (server_thread.joinable()) {
        server_thread.join();
    }
    mpz_clears(p, g, y, remote_p, remote_g, remote_y, c1, c2, NULL);
}
