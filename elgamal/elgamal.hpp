#include "../getPrime/getPrime.hpp"


class ElGamal{
public:
    ElGamal(int bits = 1024);
    ~ElGamal();

    void keygen();
    void setPKG(mpz_t p, mpz_t g, mpz_t y);
    void getM(mpz_t m);
private:
    mpz_t p, g, y, x; 
    int bits;
};