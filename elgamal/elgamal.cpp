#include "elgamal.hpp"

ElGamal::ElGamal(int bits) : bits(bits)
{
    mpz_inits(p, g, y, x, NULL);
}
