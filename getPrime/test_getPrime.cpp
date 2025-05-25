#include "getPrime.hpp"
#include <chrono>
#include <iostream>
#include <vector>

int main() {
    // 初始化 GMP 变量
    mpz_t prime;
    mpz_init(prime);
    
    // 测试不同位数的素数生成
    std::vector<int> test_bits = {32, 64, 128, 256, 512, 1024};
    
    std::cout << "=== getPrime 函数性能测试 ===" << std::endl;
    std::cout << "位数\t时间(毫秒)\t生成的素数" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    
    for (int bits : test_bits) {
        // 开始计时
        auto start = std::chrono::high_resolution_clock::now();
        
        // 调用 getPrime 函数
        getPrime(prime, bits);
        
        // 结束计时
        auto end = std::chrono::high_resolution_clock::now();
        
        // 计算毫秒数
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // 输出结果
        std::cout << bits << "\t" << duration.count() << "ms\t\t" << prime << std::endl;
        
        // 验证生成的数确实是素数（使用 Miller-Rabin 测试）
        if (MillerRabin(prime, 20)) {
            std::cout << "✓ 验证通过：生成的数是素数" << std::endl;
        } else {
            std::cout << "✗ 验证失败：生成的数不是素数" << std::endl;
        }
        std::cout << std::endl;
    }
    
    // 多次测试同一位数，计算平均时间
    std::cout << "=== 256位素数生成时间统计（10次测试）===" << std::endl;
    int test_count = 10;
    int test_bits_avg = 256;
    long total_time = 0;
    
    for (int i = 0; i < test_count; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        getPrime(prime, test_bits_avg);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        long time_ms = duration.count();
        total_time += time_ms;
        
        std::cout << "第" << (i+1) << "次: " << time_ms << "ms" << std::endl;
    }
    
    double avg_time = (double)total_time / test_count;
    std::cout << "平均时间: " << avg_time << "ms" << std::endl;
    
    // 清理 GMP 变量
    mpz_clear(prime);
    
    return 0;
}