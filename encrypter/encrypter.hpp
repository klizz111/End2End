#include <iostream>
#include <string>
using namespace std;

#include "../sm4/sm34.h"
#include "../elgamal/elgamal.hpp"

class MessageEncryptor{
public:
    MessageEncryptor(int bits);
    ~MessageEncryptor();

    void SendPKG(mpz_t p, mpz_t g, mpz_t y); 
    void GetPKG(mpz_t p, mpz_t g, mpz_t y);
    void ReceivePKG(mpz_t p, mpz_t g, mpz_t y); 
    void SendSecret(mpz_t c1, mpz_t c2); 
    void SetSM4Key(mpz_t m, int mode); // mode = 0, server; mode = 1, client
    void ReceiveSecret(mpz_t c1, mpz_t c2); 
    void EncryptMessage(const string& message, string& encrypted_message);
    void DecryptMessage(const string& encrypted_message, string& message);
private:
    int bits;
    ElGamal server; // server, 指'我'作为服务端接受请求
    ElGamal client; // client, 指'我'作为客户端发送请求
    string sm4_key_server;
    string sm4_IV_server;
    string sm4_key_client;
    string sm4_IV_client;
    string stringToHex(const string& input);   // 字符串转十六进制
    string hexToString(const string& hex);     // 十六进制转字符串
};
