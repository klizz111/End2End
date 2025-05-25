#include <iostream>
#include <chrono>
using namespace std;

#include "elgamal.hpp"

int main() {
    // 测试 ElGamal 加密算法
    auto start = std::chrono::high_resolution_clock::now();
    ElGamal elgamal(512);
    elgamal.keygen();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cout << "Key generation time: " << duration.count() << " ms" << endl;

    cout << "Test ElGamal Encryption and Decryption" << endl;
    mpz_t m, c1, c2;
    mpz_inits(m, c1, c2, NULL);

    // 明文消息
    elgamal.getM(m);
    cout << "Plaintext message: " << m << endl;
    // 加密
    start = std::chrono::high_resolution_clock::now();
    elgamal.encrypt(m, c1, c2);
    end = std::chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cout << "Encryption time: " << duration.count() << " ms" << endl;
    // 解密
    mpz_t decrypted_m;
    mpz_init(decrypted_m);

    start = std::chrono::high_resolution_clock::now();
    elgamal.decrypt(c1, c2, decrypted_m);
    end = std::chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cout << "Decryption time: " << duration.count() << " ms" << endl;
    cout << "Decrypted message: " << decrypted_m << endl;

    return 0;
}