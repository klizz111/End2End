#include "core.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    cout << "=== Client测试 ===" << endl;
    
    // 获取服务器地址
    string host = "localhost";
    int port = 8080;
    
    cout << "请输入服务器地址 (默认: localhost): ";
    string input;
    getline(cin, input);
    if (!input.empty()) {
        host = input;
    }
    
    cout << "请输入服务器端口 (默认: 8848): ";
    getline(cin, input);
    if (!input.empty()) {
        port = stoi(input);
    }
    
    // 创建客户端实例
    Core client(256);
    
    // 设置消息接收处理器
    client.setMessageHandler([](const string& message) {
        cout << "\n[收到消息] " << message << endl;
        cout << "> ";
        cout.flush();
    });
    
    // 连接到服务器
    cout << "正在连接到服务器 " << host << ":" << port << "..." << endl;
    if (!client.startClient(host, port)) {
        cout << "连接服务器失败!" << endl;
        return 1;
    }
    
    cout << "正在建立连接和进行密钥交换..." << endl;
    
    // 等待连接建立和密钥交换完成
    client.waitForConnection();
    
    if (client.isConnected()) {
        cout << "\n连接成功，密钥交换完成！" << endl;
        cout << "现在可以开始通信了。" << endl;
        cout << "请输入消息发送给服务器 (输入 'quit' 退出):" << endl;
        
        string input;
        while (true) {
            cout << "> ";
            getline(cin, input);
            
            if (input == "quit") {
                break;
            }
            
            if (!input.empty()) {
                if (client.sendMessage(input)) {
                    cout << "[已发送] " << input << endl;
                } else {
                    cout << "[发送失败] 连接可能已断开" << endl;
                }
            }
        }
    } else {
        cout << "连接失败或已断开" << endl;
    }
    
    cout << "正在断开连接..." << endl;
    client.stop();
    cout << "客户端已关闭" << endl;
    
    return 0;
}