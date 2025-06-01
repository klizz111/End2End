#ifndef CORE_HPP
#define CORE_HPP

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <memory>
#include <map>
#include <chrono>
#include "httplib.h"
#include "json.hpp"
#include "../encrypter/encrypter.hpp"

using namespace std;
using json = nlohmann::json;

class Core {
public:
    enum Mode {
        SERVER,
        CLIENT
    };

    enum ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        KEY_EXCHANGING,
        READY
    };

    enum LogLevel {
        IMPORTANT,
        INFO,
        WARNING,
        ERROR
    };

    Core(int bits = 256);
    ~Core();

    bool startServer(const string& host = "localhost", int port = 8848);
    bool startClient(const string& host = "localhost", int port = 8848);
    
    bool sendMessage(const string& message);
    void setMessageHandler(function<void(const string&)> handler);
    
    ConnectionState getState() const;
    bool isConnected() const;
    
    void stop();
    void waitForConnection();

private:
    // 状态
    Mode mode;
    ConnectionState state;
    int bits;
    unique_ptr<MessageEncryptor> encryptor;
    
    // 通信
    unique_ptr<httplib::Server> server;
    unique_ptr<httplib::Client> client;
    atomic<bool> running;
    string serverHost;
    int serverPort;
    
    // 线程管理
    unique_ptr<thread> serverThread;
    unique_ptr<thread> messageThread;
    unique_ptr<thread> pollingThread;
    mutable mutex stateMutex;
    condition_variable stateCondition;
    
    // 消息传输
    function<void(const string&)> messageHandler;
    queue<string> outgoingMessageQueue;
    queue<string> incomingMessageQueue;
    mutex messageMutex;
    condition_variable messageCondition;
    
    queue<string> serverToClientMessages;
    mutex serverMessageMutex;
    
    bool keyExchangeComplete;
    mutex keyExchangeMutex;
    
    string sessionId;
    chrono::steady_clock::time_point lastActivity;
    
    // 内部方法
    void setState(ConnectionState newState);
    void setupServerRoutes();
    // void startPolling();
    void processMessageQueue();
    
    // 路由处理
    void handleKeyExchange(const httplib::Request& req, httplib::Response& res);
    void handleSendMessage(const httplib::Request& req, httplib::Response& res);
    void handleReceiveMessages(const httplib::Request& req, httplib::Response& res);
    void handleStatus(const httplib::Request& req, httplib::Response& res);
    
    // 密钥交换
    bool performKeyExchangeAsClient();
    bool sendPublicKey();
    bool receivePublicKey(const json& data);
    bool sendSecret();
    bool receiveSecret(const json& data);
    void completeKeyExchange();
    
    // JSON处理
    json createMessage(const string& type, const json& data);
    void sendJsonResponse(httplib::Response& res, const json& data, int status = 200);
    
    // 客户端轮询
    void pollMessages();
    
    // 日志
    void log(const string& message, LogLevel type = INFO) const;
    
    // 工具方法
    string generateSessionId();
    void updateLastActivity();
    bool isSessionValid() const;
    
    bool isPortAvailable(int port) const;
    int findAvailablePort(int port, int Maxoffset = 10);
};

#endif // CORE_HPP