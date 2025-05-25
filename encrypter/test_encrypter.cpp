#include <iostream>
#include <string>
#include "encrypter.hpp"

using namespace std;

int main() {
    auto encrypter = MessageEncryptor(256);

    mpz_t p, g, y;
    mpz_inits(p, g, y, NULL);

    encrypter.SendPKG(p, g, y);
    cout << "p,g,y=" << endl;
    cout << p << ' ' << g << ' ' << y << endl;

    mpz_t pp, gg, yy;
    mpz_inits(pp, gg, yy, NULL);

    cout << "pp,gg,yy=" << endl;
    cin >> pp >> gg >> yy;

    encrypter.ReceivePKG(pp, gg, yy);
    
    mpz_t c1, c2;
    mpz_inits(c1, c2, NULL);

    encrypter.SendSecret(c1, c2);
    cout << "c1,c2=" << endl;
    cout << c1 << ' ' << c2 << endl;

    mpz_t cc1, cc2;
    mpz_inits(cc1, cc2, NULL);
    cout << "cc1,cc2=" << endl;
    cin >> cc1 >> cc2;

    encrypter.ReceiveSecret(cc1, cc2);

    string key1, key2;
    encrypter.GetSM4Key(key1, key2);

    cout << "key1= " << key1 << endl;
    cout << "key2= " << key2 << endl;
    
    string message = "你好你好你好你好你好你好你好你好你好你好你好你好";
    string encrypted_message;
    encrypter.EncryptMessage(message, encrypted_message);
    cout << "Encrypted message: " << encrypted_message << endl;

    string secret;
    cout << "Input secret message: " << endl;
    cin >> secret;

    string decrypted_message;
    encrypter.DecryptMessage(secret, decrypted_message);
    cout << "Decrypted message: " << decrypted_message << endl;
}