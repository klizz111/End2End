#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <queue>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include "httplib.h"
#include "../encrypter/encrypter.hpp"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

enum class ServerMode {
    NONE,
    SERVER,
    CLIENT
};

enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    KEY_EXCHANGE,
    CONNECTED
};

class SecureServer {
private:
    ServerMode mode;
    ConnectionStatus status;
    string destination_host;
    int destination_port;
    int listen_port;
    int api_port;
    unique_ptr<httplib::Server> http_server;
    unique_ptr<httplib::Client> http_client;
    unique_ptr<MessageEncryptor> encryptor;
    
    // 密钥交换相关
    mpz_t p, g, y;  // 本地公钥参数
    mpz_t remote_p, remote_g, remote_y;  // 远程公钥参数
    mpz_t c1, c2;  // 加密的密钥
    bool key_exchange_completed;
    
    // 消息队列
    queue<string> message_queue;
    mutex queue_mutex;
    
public:
    SecureServer() : mode(ServerMode::NONE), status(ConnectionStatus::DISCONNECTED), 
                     listen_port(8848), api_port(1145), key_exchange_completed(false) {
        mpz_inits(p, g, y, remote_p, remote_g, remote_y, c1, c2, NULL);
        encryptor = make_unique<MessageEncryptor>(256);
    }
    
    ~SecureServer() {
        mpz_clears(p, g, y, remote_p, remote_g, remote_y, c1, c2, NULL);
    }
    
    bool isPortAvailable(int port) {
        // 使用socket API检查端口是否可用
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return false;
        }
        
        // 设置SO_REUSEADDR选项
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
            cout << "端口 " << port << " 可用" << endl;
            return true;
        } else {
            cout << "端口 " << port << " 不可用: " << strerror(errno) << endl;
            return false;
        }
    }
    
    int findAvailablePort(int start_port, int max_attempts = 10) {
        for (int i = 0; i < max_attempts; i++) {
            int test_port = start_port + i;
            if (isPortAvailable(test_port)) {
                return test_port;
            }
        }
        return -1;
    }
    
    void setupRoutes() {
        if (!http_server) return;
        
        // CORS设置
        http_server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return httplib::Server::HandlerResponse::Unhandled;
        });
        
        // OPTIONS请求处理
        http_server->Options(".*", [](const httplib::Request&, httplib::Response& res) {
            return;
        });
        
        // setting路由
        http_server->Post("/setting", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                auto j = json::parse(req.body);
                string mode_str = j["mode"].get<string>();
                
                if (mode_str == "server") {
                    mode = ServerMode::SERVER;
                    status = ConnectionStatus::DISCONNECTED;
                    
                    // 为服务器模式找一个可用的监听端口
                    listen_port = findAvailablePort(8848);
                    if (listen_port == -1) {
                        res.status = 500;
                        res.set_content("{\"error\": \"无法找到可用的监听端口\"}", "application/json");
                        return;
                    }
                    
                    cout << "设置为服务器模式，监听端口: " << listen_port << endl;
                    
                } else if (mode_str == "client") {
                    mode = ServerMode::CLIENT;
                    if (j.contains("destination")) {
                        destination_host = j["destination"]["host"].get<string>();
                        destination_port = j["destination"]["port"].get<int>();
                        cout << "设置为客户端模式，目标服务器: " << destination_host << ":" << destination_port << endl;
                        
                        // 开始连接到服务器
                        thread([this]() { connectToServer(); }).detach();
                    }
                }
                
                res.set_content("{\"success\": true}", "application/json");
            } catch (const exception& e) {
                res.status = 400;
                res.set_content("{\"error\": \"" + string(e.what()) + "\"}", "application/json");
            }
        });
        
        // status路由
        http_server->Get("/status", [this](const httplib::Request&, httplib::Response& res) {
            string status_str;
            switch (status) {
                case ConnectionStatus::DISCONNECTED: status_str = "disconnected"; break;
                case ConnectionStatus::CONNECTING: status_str = "connecting"; break;
                case ConnectionStatus::KEY_EXCHANGE: status_str = "key_exchange"; break;
                case ConnectionStatus::CONNECTED: status_str = "connected"; break;
            }
            
            json response;
            response["status"] = status_str;
            response["mode"] = mode == ServerMode::SERVER ? "server" : 
                             mode == ServerMode::CLIENT ? "client" : "none";
            response["api_port"] = api_port;
            response["listen_port"] = listen_port;
            res.set_content(response.dump(), "application/json");
        });
        
        // 密钥交换路由 - 发送公钥参数
        http_server->Post("/key_exchange/public_key", [this](const httplib::Request& req, httplib::Response& res) {
            if (mode == ServerMode::SERVER) {
                // 服务器模式：生成并发送公钥参数
                encryptor->SendPKG(p, g, y);
                
                char* p_str = mpz_get_str(NULL, 10, p);
                char* g_str = mpz_get_str(NULL, 10, g);
                char* y_str = mpz_get_str(NULL, 10, y);
                
                json response;
                response["p"] = string(p_str);
                response["g"] = string(g_str);
                response["y"] = string(y_str);
                
                free(p_str);
                free(g_str);
                free(y_str);
                
                cout << "发送公钥参数给客户端" << endl;
                res.set_content(response.dump(), "application/json");
                status = ConnectionStatus::KEY_EXCHANGE;
            } else {
                res.status = 400;
                res.set_content("{\"error\": \"Only server can send public key\"}", "application/json");
            }
        });
        
        // 密钥交换路由 - 接收加密的密钥
        http_server->Post("/key_exchange/secret", [this](const httplib::Request& req, httplib::Response& res) {
            if (mode == ServerMode::SERVER) {
                try {
                    auto j = json::parse(req.body);
                    mpz_set_str(c1, j["c1"].get<string>().c_str(), 10);
                    mpz_set_str(c2, j["c2"].get<string>().c_str(), 10);
                    
                    encryptor->ReceiveSecret(c1, c2);
                    key_exchange_completed = true;
                    status = ConnectionStatus::CONNECTED;
                    
                    cout << "密钥交换完成！" << endl;
                    res.set_content("{\"success\": true}", "application/json");
                } catch (const exception& e) {
                    res.status = 400;
                    res.set_content("{\"error\": \"" + string(e.what()) + "\"}", "application/json");
                }
            } else {
                res.status = 400;
                res.set_content("{\"error\": \"Only server can receive secret\"}", "application/json");
            }
        });
        
        // 聊天路由 - 发送消息
        http_server->Post("/chat/send", [this](const httplib::Request& req, httplib::Response& res) {
            if (!key_exchange_completed) {
                res.status = 400;
                res.set_content("{\"error\": \"Key exchange not completed\"}", "application/json");
                return;
            }
            
            try {
                auto j = json::parse(req.body);
                string message = j["message"].get<string>();
                
                string encrypted_message;
                encryptor->EncryptMessage(message, encrypted_message);
                
                // 发送到对方
                if (mode == ServerMode::CLIENT && http_client) {
                    json msg;
                    msg["encrypted_message"] = encrypted_message;
                    auto client_res = http_client->Post("/chat/receive", msg.dump(), "application/json");
                    if (client_res && client_res->status == 200) {
                        res.set_content("{\"success\": true}", "application/json");
                    } else {
                        res.status = 500;
                        res.set_content("{\"error\": \"Failed to send message\"}", "application/json");
                    }
                } else {
                    // 服务器模式，消息会被客户端主动获取
                    lock_guard<mutex> lock(queue_mutex);
                    message_queue.push(encrypted_message);
                    res.set_content("{\"success\": true}", "application/json");
                }
                
                cout << "发送消息: " << message << endl;
            } catch (const exception& e) {
                res.status = 400;
                res.set_content("{\"error\": \"" + string(e.what()) + "\"}", "application/json");
            }
        });
        
        // 聊天路由 - 接收消息
        http_server->Post("/chat/receive", [this](const httplib::Request& req, httplib::Response& res) {
            if (!key_exchange_completed) {
                res.status = 400;
                res.set_content("{\"error\": \"Key exchange not completed\"}", "application/json");
                return;
            }
            
            try {
                auto j = json::parse(req.body);
                string encrypted_message = j["encrypted_message"].get<string>();
                
                string decrypted_message;
                encryptor->DecryptMessage(encrypted_message, decrypted_message);
                
                cout << "收到消息: " << decrypted_message << endl;
                
                json response;
                response["message"] = decrypted_message;
                res.set_content(response.dump(), "application/json");
            } catch (const exception& e) {
                res.status = 400;
                res.set_content("{\"error\": \"" + string(e.what()) + "\"}", "application/json");
            }
        });
        
        // 聊天路由 - 获取消息（用于服务器模式）
        http_server->Get("/chat/messages", [this](const httplib::Request&, httplib::Response& res) {
            lock_guard<mutex> lock(queue_mutex);
            json messages = json::array();
            
            while (!message_queue.empty()) {
                messages.push_back(message_queue.front());
                message_queue.pop();
            }
            
            res.set_content(messages.dump(), "application/json");
        });
    }
    
    void connectToServer() {
        status = ConnectionStatus::CONNECTING;
        cout << "正在连接到服务器 " << destination_host << ":" << destination_port << "..." << endl;
        
        // 使用目标服务器的API端口而不是监听端口
        // 我们需要找到目标服务器的API端口
        string server_url = destination_host + ":" + to_string(destination_port);
        http_client = make_unique<httplib::Client>(server_url);
        
        // 设置连接超时
        http_client->set_connection_timeout(5, 0); // 5秒超时
        
        // 1. 获取服务器公钥参数
        auto res = http_client->Post("/key_exchange/public_key", "", "application/json");
        if (!res) {
            cout << "连接服务器失败：无法建立连接" << endl;
            status = ConnectionStatus::DISCONNECTED;
            return;
        }
        
        if (res->status != 200) {
            cout << "连接服务器失败：HTTP状态码 " << res->status << endl;
            status = ConnectionStatus::DISCONNECTED;
            return;
        }
        
        auto j = json::parse(res->body);
        mpz_set_str(remote_p, j["p"].get<string>().c_str(), 10);
        mpz_set_str(remote_g, j["g"].get<string>().c_str(), 10);
        mpz_set_str(remote_y, j["y"].get<string>().c_str(), 10);
        
        cout << "获取到服务器公钥参数" << endl;
        
        // 2. 设置公钥参数并生成密钥
        encryptor->ReceivePKG(remote_p, remote_g, remote_y);
        encryptor->SendSecret(c1, c2);
        
        // 3. 发送加密的密钥给服务器
        char* c1_str = mpz_get_str(NULL, 10, c1);
        char* c2_str = mpz_get_str(NULL, 10, c2);
        
        json secret_msg;
        secret_msg["c1"] = string(c1_str);
        secret_msg["c2"] = string(c2_str);
        
        free(c1_str);
        free(c2_str);
        
        res = http_client->Post("/key_exchange/secret", secret_msg.dump(), "application/json");
        if (res && res->status == 200) {
            key_exchange_completed = true;
            status = ConnectionStatus::CONNECTED;
            cout << "密钥交换完成！客户端已连接到服务器" << endl;
        } else {
            cout << "密钥交换失败！" << endl;
            status = ConnectionStatus::DISCONNECTED;
        }
    }
    
    void start() {
        // 为HTTP API服务器寻找可用端口
        api_port = findAvailablePort(1145);
        if (api_port == -1) {
            cout << "无法找到可用端口！" << endl;
            return;
        }
        
        http_server = make_unique<httplib::Server>();
        setupRoutes();
        
        // 静态文件服务
        http_server->set_mount_point("/", "./server/public");
        
        cout << "请访问 http://localhost:" << api_port << " 进行配置和测试" << endl;
        cout << "HTTP API服务器启动在端口: " << api_port << endl;
        
        http_server->listen("0.0.0.0", api_port);
    }
};

int main() {
    SecureServer server;
    
    cout << "=== 端到端加密通信系统 ===" << endl;
    
    server.start();
    
    return 0;
}