#include "encrypter.hpp"
#include <algorithm>

MessageEncryptor::MessageEncryptor(int bits) : bits(bits), server(bits), client(bits)
{
}

MessageEncryptor::~MessageEncryptor()
{
}

void MessageEncryptor::SendPKG(mpz_t p, mpz_t g, mpz_t y)
{
    server.keygen();
    server.getPKG(p, g, y);
}

void MessageEncryptor::GetPKG(mpz_t p, mpz_t g, mpz_t y)
{
    server.getPKG(p, g, y);
}

void MessageEncryptor::ReceivePKG(mpz_t p, mpz_t g, mpz_t y)
{
    client.setPKG(p, g, y);
    client.keygen();
}

void MessageEncryptor::SendSecret(mpz_t c1, mpz_t c2)
{
    mpz_t m, c1_1, c2_1;
    mpz_inits(m, c1_1, c2_1, NULL);

    client.getM(m);
    SetSM4Key(m, 1); // client

    client.encrypt(m, c1_1, c2_1);

    mpz_set(c1, c1_1);
    mpz_set(c2, c2_1);

    // 释放内存
    mpz_clears(m, c1_1, c2_1, NULL);
    client.clean(); // 无需再使用client
}

void MessageEncryptor::SetSM4Key(mpz_t m, int mode)
{
    // mpz_t m直接转化为16进制字符串
    char* hex_char = mpz_get_str(NULL, 16, m);
    string hex_key(hex_char);
    free(hex_char); // 释放mpz_get_str分配的内存
    
    // 转为大写
    transform(hex_key.begin(), hex_key.end(), hex_key.begin(), ::toupper);
    hex_key = hex_key.substr(0, 32); 

    // IV 直接为key inverse
    string iv_key = hex_key;
    reverse(iv_key.begin(), iv_key.end());

    if (mode == 0) {
        sm4_key_server = hex_key;
        sm4_IV_server = iv_key;
    } else {
        sm4_key_client = hex_key;
        sm4_IV_client = iv_key;
    }
}

void MessageEncryptor::ReceiveSecret(mpz_t c1, mpz_t c2)
{
    mpz_t m;
    mpz_init(m);

    server.decrypt(c1, c2, m);
    SetSM4Key(m, 0); // server

    mpz_clear(m);
    server.clean(); // 无需再使用server
}

// 使用server的密钥加密消息
void MessageEncryptor::EncryptMessage(const string &message, string &encrypted_message)
{
    string plain = stringToHex(message);
    encrypted_message = sm4_encode_CBC(plain, sm4_key_server, sm4_IV_server);   
}

// 使用client的密钥解密消息
void MessageEncryptor::DecryptMessage(const string &encrypted_message, string &message)
{
    string plain = sm4_decode_CBC(encrypted_message, sm4_key_client, sm4_IV_client);
    message = hexToString(plain);
}

string MessageEncryptor::stringToHex(const string &input) 
{
    string result;
    result.reserve(input.length() * 2);
    
    for (unsigned char c : input) {
        result += "0123456789ABCDEF"[c >> 4];
        result += "0123456789ABCDEF"[c & 0x0F];
    }
    
    return result;
}

string MessageEncryptor::hexToString(const string &hex) 
{
    if (hex.length() % 2 != 0) {
        throw std::invalid_argument("十六进制字符串长度必须为偶数");
    }
    
    string result;
    result.reserve(hex.length() / 2);
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        char high = hex[i];
        char low = hex[i + 1];
        
        // 转换为数值
        int highVal = (high >= '0' && high <= '9') ? (high - '0') :
                     (high >= 'A' && high <= 'F') ? (high - 'A' + 10) :
                     (high >= 'a' && high <= 'f') ? (high - 'a' + 10) : -1;
        
        int lowVal = (low >= '0' && low <= '9') ? (low - '0') :
                    (low >= 'A' && low <= 'F') ? (low - 'A' + 10) :
                    (low >= 'a' && low <= 'f') ? (low - 'a' + 10) : -1;
        
        if (highVal == -1 || lowVal == -1) {
            throw std::invalid_argument("无效的十六进制字符");
        }
        
        result += static_cast<char>((highVal << 4) | lowVal);
    }
    
    return result;
}