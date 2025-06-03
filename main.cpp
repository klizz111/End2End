#include "./frontend/web.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <signal.h>
#include <atomic>

using namespace std;

atomic<bool> shutdown_requested(false);
WebServer* global_webServer = nullptr;
// 退出信号处理
void signalHandler(int signum) {
    if (signum == SIGINT) {
        cout << "\n\n收到退出信号 (Ctrl+C)，正在关闭服务器..." << endl;
        shutdown_requested = true;
        
        // 如果webServer存在，立即停止
        if (global_webServer) {
            global_webServer->stop();
        }
    }
}

void printUsage(const string& programName) {
    cout << "使用方法: " << programName << " [-p port] [-b bits]" << endl;
    cout << "参数:" << endl;
    cout << "  -p port    指定前端服务器端口 (默认: 3000)" << endl;
    cout << "  -b bits    指定加密位数 (默认: 256)" << endl;
    cout << endl;
    cout << "示例:" << endl;
    cout << "  " << programName << "              # 使用默认端口3000，256位加密" << endl;
    cout << "  " << programName << " -p 8080      # 使用端口8080" << endl;
    cout << "  " << programName << " -b 512       # 使用512位加密" << endl;
    cout << "  " << programName << " -p 8080 -b 1024  # 使用端口8080和1024位加密" << endl;
}

int main(int argc, char* argv[]) {
    // 注册信号处理器
    signal(SIGINT, signalHandler);
    
    int port = 3000;  // 默认端口
    int bits = 256;   // 默认加密位数
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                try {
                    port = stoi(argv[i + 1]);
                    if (port <= 0 || port > 65535) {
                        cerr << "错误: 端口号必须在1-65535之间" << endl;
                        return 1;
                    }
                    i++;  
                } catch (const exception& e) {
                    cerr << "错误: 无效的端口号 '" << argv[i + 1] << "'" << endl;
                    return 1;
                }
            } else {
                cerr << "错误: -p 参数需要指定端口号" << endl;
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "-b" || arg == "--bits") {
            if (i + 1 < argc) {
                try {
                    bits = stoi(argv[i + 1]);
                    if (bits < 128 || bits > 4096) {
                        cerr << "错误: 加密位数必须在128-4096之间" << endl;
                        return 1;
                    }
                    i++;  
                } catch (const exception& e) {
                    cerr << "错误: 无效的加密位数 '" << argv[i + 1] << "'" << endl;
                    return 1;
                }
            } else {
                cerr << "错误: -b 参数需要指定加密位数" << endl;
                printUsage(argv[0]);
                return 1;
            }
        } else {
            cerr << "错误: 未知参数 '" << arg << "'" << endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    cout << "=== End2End WebServer===" << endl;
    cout << "加密位数: " << bits << endl;
    cout << "按 Ctrl+C 退出" << endl;
    cout << "=========================" << endl;
    
    auto webServer = new WebServer(port);
    global_webServer = webServer; // 保存全局引用用于信号处理
    
    auto core = make_shared<Core>(bits);
    webServer->setCoreInstance(core);
    
    if (!webServer->start()) {
        cerr << "错误: 无法启动Web服务器" << endl;
        return 1;
    }
        
    try {
        while (webServer->isRunning() && !shutdown_requested) {
            this_thread::sleep_for(chrono::milliseconds(100)); 
        }
    } catch (const exception& e) {
        cout << "\n服务器异常: " << e.what() << endl;
    }
    
    // 清理全局引用
    global_webServer = nullptr;
    
    cout << "\n正在关闭服务器..." << endl;
    webServer->stop();
    cout << "服务器已关闭" << endl;
    
    delete webServer;
    
    return 0;
}