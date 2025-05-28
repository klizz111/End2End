#include "core.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    cout << "=== 安全通信系统 - 服务器端测试 ===" << endl;
    
    // 创建服务器实例
    Core server(256);
    
    // 设置消息接收处理器
    server.setMessageHandler([](const string& message) {
        cout << "\n[收到消息] " << message << endl;
        cout << "请输入回复消息 (输入 'quit' 退出): ";
        cout.flush();
    });
    
    // 启动服务器
    cout << "正在启动服务器..." << endl;
    if (!server.startServer("0.0.0.0", 8080)) {
        cout << "服务器启动失败!" << endl;
        return 1;
    }
    
    cout << "服务器已启动，等待客户端连接..." << endl;
    cout << "服务器地址: 0.0.0.0:8080" << endl;
    
    // 等待连接建立
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