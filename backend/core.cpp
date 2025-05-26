#include "core.hpp"
#include <sys/socket.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

Core::Core() : status(ConnectionStatus::DISCONNECTED), 
    key_exchange_completed(false) {
    mpz_inits(p, g, y, remote_p, remote_g, remote_y, c1, c2, NULL);
}

bool Core::isPortAvailable(int port) {
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
}

void Core::start()
{
    server = make_unique<httplib::Server>();
    listen_port = findAvailablePort(8848);
    if (listen_port == -1) {
        cerr << "无法找到可用的监听端口！" << endl;
        return;
    }
    server->listen("localhost", listen_port);

    client = make_unique<httplib::Client>(destination_host, destination_port);
}
