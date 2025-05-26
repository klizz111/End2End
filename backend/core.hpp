#include <iostream>
#include <memory>
#include <string>
#include <gmp.h>
using namespace std;

#include "../encrypter/encrypter.hpp"
#include "httplib.h"
#include "json.hpp"
using json = nlohmann::json;

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

class Core {
private:
    unique_ptr<httplib::Server> server;
    unique_ptr<httplib::Client> client;
    unique_ptr<MessageEncryptor> encryptor;
    
    ConnectionStatus status;
    string destination_host;
    int destination_port;
    int listen_port;
    int api_port;
    
    // 密钥交换相关
    mpz_t p, g, y;  // 本地公钥参数
    mpz_t remote_p, remote_g, remote_y;  // 远程公钥参数
    mpz_t c1, c2;  // 加密的密钥
    bool key_exchange_completed;

public:
    Core();
    
    bool isPortAvailable(int port);
    int findAvailablePort(int start_port, int max_attempts = 10);
    void setupRoutes();

    void start();
};