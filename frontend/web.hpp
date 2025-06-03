#ifndef WEB_HPP
#define WEB_HPP

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "../core/httplib.h"
#include "../core/json.hpp"
#include "../core/core.hpp"

using namespace std;
using json = nlohmann::json;

class WebServer {
public:
    enum ServerState {
        STOPPED,
        STARTING,
        RUNNING,
        STOPPING
    };

    WebServer(int port = 3000);
    ~WebServer();

    bool start();
    void stop();
    bool isRunning() const;
    ServerState getState() const;

    // 设置核心通信对象
    void setCoreInstance(shared_ptr<Core> coreInstance);

private:
    int port;
    ServerState state;
    mutable mutex stateMutex;
    
    unique_ptr<httplib::Server> server;
    unique_ptr<thread> serverThread;
    shared_ptr<Core> core;
    
    // 设置路由
    void setupRoutes();
    
    // API处理
    void handleStatus(const httplib::Request& req, httplib::Response& res);
    void handleSetting(const httplib::Request& req, httplib::Response& res);
    void handleSendMessage(const httplib::Request& req, httplib::Response& res);
    void handleReceiveMessage(const httplib::Request& req, httplib::Response& res);
    void handleGetMessages(const httplib::Request& req, httplib::Response& res);
    
    // 工具函数
    void sendJsonResponse(httplib::Response& res, const json& data, int status = 200);
    void log(const string& message) const;
    string getCurrentTime() const;
    void setState(ServerState newState);
    bool isPortAvailable(int port) const;
    int findAvailablePort(int port, int Maxoffset = 10);
    
    // 消息回调处理
    void onMessageReceived(const string& message);

    // 存储接收到的消息
    mutable mutex messagesMutex;
    queue<string> receivedMessages;
};

#endif // WEB_HPP