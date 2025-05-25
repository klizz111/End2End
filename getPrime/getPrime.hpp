#pragma once
#include <gmp.h>
#include <string>
#include <iostream>

/// @brief Miller-Rabin primality test
/// @param n 
/// @param k times to test
/// @return 
bool MillerRabin(mpz_t n, int k = 20);

/// @brief Gen a random prime number with given bits
/// @param p 
/// @param bits 
/// @return 
void getPrime(mpz_t p, int bits);

/// @brief 生成安全素数p = 2q + 1
/// @param p 
/// @param q 
/// @param bits 
void genSafePrime(mpz_t p, mpz_t q, int bits);

// 运算符重载声明
std::ostream &operator<<(std::ostream &os, const mpz_t &mpz);
// std::istream &operator>>(std::istream &is, mpz_t &mpz);