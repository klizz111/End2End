#include "core.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

int main(int argc, char* argv[]) {
    cout << "=== Server测试 ===" << endl;

    int port= 8848;

    if (argc > 1) {
        if (argv[1] == string("-p")) {
            port = stoi(argv[2]);
        }
        else {
            cerr << "未知参数: " << argv[1] << endl;
            return 1;
        }
    }
    
    Core server(256);
    
    server.setMessageHandler([](const string& message) {
        cout << "\n[收到消息] " << message << endl;
        cout << "请输入回复消息 (输入 'quit' 退出): ";
        cout.flush();
    });
    
    cout << "正在启动服务器..." << endl;
    if (!server.startServer("0.0.0.0", port)) {
        cout << "服务器启动失败!" << endl;
        return 1;
    }
    
    cout << "服务器已启动，等待客户端连接..." << endl;
    cout << "服务器地址: localhost:" << port << endl;
    
    server.waitForConnection();
    
    if (server.isConnected()) {
        cout << "\n客户端已连接，密钥交换完成！" << endl;
        cout << "现在可以开始安全通信了。" << endl;
        cout << "请输入消息发送给客户端 (输入 'quit' 退出):" << endl;
        
        string input;
        while (true) {
            cout << "> ";
            getline(cin, input);
            
            if (input == "quit") {
                break;
            }
            
            if (!input.empty()) {
                if (server.sendMessage(input)) {
                    cout << "[已发送] " << input << endl;
                } else {
                    cout << "[发送失败] 连接可能已断开" << endl;
                }
            }
        }
    } else {
        cout << "连接失败或已断开" << endl;
    }
    
    cout << "正在关闭服务器..." << endl;
    server.stop();
    cout << "服务器已关闭" << endl;
    
    return 0;
}