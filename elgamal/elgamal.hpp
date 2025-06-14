#include "../getPrime/getPrime.hpp"


class ElGamal{
public:
    ElGamal(int bits = 1024);
    ~ElGamal();

    void keygen();
    void generatePrivateKey(); 
    void initX();
    void getPKG(mpz_t p_out, mpz_t g_out, mpz_t y_out);  
    void setPKG(mpz_t p_in, mpz_t g_in, mpz_t y_in);
    void encrypt(mpz_t m, mpz_t c1, mpz_t c2);
    void decrypt(mpz_t c1, mpz_t c2, mpz_t m);
    void clean();
    void getM(mpz_t m); // 获取随机数，用于产生SM4密钥
    void checkM(mpz_t m); // 检查明文是否符合要求
private:
    mpz_t p, g, y, x, q;  
    gmp_randstate_t state;  
    int bits;
    bool is_cleaned;
};