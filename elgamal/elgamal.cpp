#include "elgamal.hpp"
#include <ctime>

ElGamal::ElGamal(int bits) : bits(bits), is_cleaned(false)
{
    mpz_inits(p, g, y, x, q, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
}

ElGamal::~ElGamal()
{
    if (!is_cleaned)
    {
        clean();
    }
}

// gen p q g h x y
void ElGamal::keygen()
{
    // 1. 生成安全素数 p = 2q + 1
    genSafePrime(p, q, bits);
    
    // 2. 选取生成元 g
    mpz_t h, exp;
    mpz_inits(h, exp, NULL);
    mpz_set_ui(exp, 2); // exp = 2
    
    while (true) {
        // 生成随机数 h ∈ [2, p-1]
        mpz_urandomm(h, state, p);
        if (mpz_cmp_ui(h, 2) < 0) continue;
        
        // g = h^2 mod p
        mpz_powm(g, h, exp, p);
        if (mpz_cmp_ui(g, 1) > 0) break;
    }
    mpz_clears(h, exp, NULL);
    
    // 3. 生成私钥 x ∈ [1, q-1]
    do {
        mpz_urandomm(x, state, q);
    } while (mpz_cmp_ui(x, 1) < 0);
    
    // 4. 计算公钥 y = g^x mod p
    mpz_powm(y, g, x, p);
}

void ElGamal::generatePrivateKey()
{
    mpz_sub_ui(q, p, 1);
    mpz_divexact_ui(q, q, 2);
    
    // 生成私钥 x ∈ [1, q-1]
    do {
        mpz_urandomm(x, state, q);
    } while (mpz_cmp_ui(x, 1) < 0);
}

void ElGamal::setPKG(mpz_t p_in, mpz_t g_in, mpz_t y_in)
{
    mpz_set(p, p_in);
    mpz_set(g, g_in);
    mpz_set(y, y_in);
}

void ElGamal::initX()
{
    mpz_urandomm(x, state, p);
}

void ElGamal::getPKG(mpz_t p_out, mpz_t g_out, mpz_t y_out)
{
    mpz_set(p_out, p);
    mpz_set(g_out, g);
    mpz_set(y_out, y);
}

void ElGamal::encrypt(mpz_t m, mpz_t c1, mpz_t c2)
{
    try {
        checkM(m);
    } catch (const std::invalid_argument& e) {
        throw;
    }
    
    mpz_t k;
    mpz_init(k);
    
    // 生成随机数 k ∈ [1, q-1]
    do {
        mpz_urandomm(k, state, q);
    } while (mpz_cmp_ui(k, 1) < 0);
    
    // 2. c1 = g^k mod p
    mpz_powm(c1, g, k, p);
    
    // 3. c2 = m * y^k mod p
    mpz_powm(c2, y, k, p);
    mpz_mul(c2, c2, m);
    mpz_mod(c2, c2, p);
    
    // 清理临时变量
    mpz_clear(k);
}

void ElGamal::decrypt(mpz_t c1, mpz_t c2, mpz_t m)
{
    // 创建临时变量存储中间结果
    mpz_t s;
    mpz_init(s);
    
    // 1. s = c1^x mod p
    mpz_powm(s, c1, x, p);
    
    // 2. m = c2 * s^(-1) mod p
    mpz_invert(s, s, p);
    mpz_mul(m, c2, s);
    mpz_mod(m, m, p);
    
    // 清理临时变量
    mpz_clear(s);
}

void ElGamal::clean()
{
    mpz_clears(p, g, y, x, q, NULL);
    gmp_randclear(state);
    is_cleaned = true;
}

void ElGamal::getM(mpz_t m)
{
    // m ∈ [2,q-1]
    do {
        mpz_urandomm(m, state, q);
    } while (mpz_cmp_ui(m, 2) < 0); 
}

void ElGamal::checkM(mpz_t m)
{
    if (mpz_cmp(m, p) >= 0 || mpz_cmp_ui(m, 2) < 0)
    {
        throw std::invalid_argument("Invalid message: m must be in [2, p-1]");
    }
}
