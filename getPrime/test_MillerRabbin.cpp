#include <iostream>
#include <chrono>
#include <string>
#include "getPrime.hpp"
using namespace std;

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BOLD "\033[1m"
#define RESET "\033[0m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"

void printInstructions() {
    cout << YELLOW << "Miller-Rabin Primality Test" << RESET << endl;
    cout << "----------------------------------------" << endl;
    cout << BOLD << "Instructions: " << endl;
    cout << "1. Enter a number to test for primality." << endl;
    cout << "2. Enter 'k <int>' to set the number of iterations for the test (default: 20)." << endl;
    cout << "3. Enter 'n <int>' to generate a random prime number with n bits." << endl;
    cout << "4. Enter 't' to toggle time display." << endl;
    cout << "5. Enter 'h' to show this help message." << endl;
    cout << "6. Enter 'q' or '0' to exit." << RESET << endl;
    cout << "----------------------------------------" << endl;
}

int main() {
    mpz_t n, p, q;
    mpz_init(n);
    mpz_init(p);
    mpz_init(q);
    
    int k = 40;  // 默认迭代次数
    bool showTime = false;
    string input;
    
    printInstructions();
    
    while (true) {
        cout << BLUE << "\n> " << RESET;
        cin >> input;
        
        if (input == "q" || input == "0") {
            cout << GREEN << "Goodbye!" << RESET << endl;
            break;
        }
        else if (input == "h") {
            printInstructions();
        }
        else if (input == "t") {
            showTime = !showTime;
            cout << GREEN << "Time display " << (showTime ? "enabled" : "disabled") << RESET << endl;
        }
        else if (input == "k") {
            int newK;
            cin >> newK;
            if (newK > 0) {
                k = newK;
                cout << GREEN << "Iterations set to " << k << RESET << endl;
            } else {
                cout << RED << "Invalid number of iterations!" << RESET << endl;
            }
        }
        else if (input == "n") {
            int bits;
            cin >> bits;
            if (bits <= 0 || bits > 10000) {
                cout << RED << "Invalid bit size! Please enter a value between 1 and 10000." << RESET << endl;
                continue;
            }
            
            cout << YELLOW << "Generating " << bits << "-bit prime number..." << RESET << endl;
            
            auto start = chrono::high_resolution_clock::now();
            getPrime(p, bits);
            auto end = chrono::high_resolution_clock::now();
            
            cout << GREEN << "Generated prime: " << p << RESET << endl;
            
            if (showTime) {
                auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
                cout << BLUE << "Time taken: " << duration.count() << " ms" << RESET << endl;
            }
        }
        else {
            // 尝试将输入作为数字进行素性测试
            if (mpz_set_str(n, input.c_str(), 10) == 0) {
                if (mpz_cmp_ui(n, 0) <= 0) {
                    cout << RED << "Please enter a positive integer!" << RESET << endl;
                    continue;
                }
                
                cout << YELLOW << "Testing primality of " << n << " with " << k << " iterations..." << RESET << endl;
                
                auto start = chrono::high_resolution_clock::now();
                bool isPrime = MillerRabin(n, k);
                auto end = chrono::high_resolution_clock::now();
                
                if (isPrime) {
                    cout << GREEN << n << " is probably prime!" << RESET << endl;
                } else {
                    cout << RED << n << " is composite!" << RESET << endl;
                }
                
                if (showTime) {
                    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
                    if (duration.count() >= 1000) {
                        cout << BLUE << "Time taken: " << duration.count() / 1000.0 << " ms" << RESET << endl;
                    } else {
                        cout << BLUE << "Time taken: " << duration.count() << " μs" << RESET << endl;
                    }
                }
            } else {
                cout << RED << "Invalid input! Please enter a valid command or number." << RESET << endl;
                cout << "Type 'h' for help." << endl;
            }
        }
    }
    
    mpz_clear(n);
    mpz_clear(p);
    mpz_clear(q);
    
    return 0;
}