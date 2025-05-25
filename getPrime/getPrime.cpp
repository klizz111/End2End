#include "getPrime.hpp"
#include <cstdlib>
#include <ctime>

// 全局随机数状态
static gmp_randstate_t global_rand_state;
static bool rand_state_initialized = false;

// 初始化全局随机数状态
static void init_global_rand_state() {
    if (!rand_state_initialized) {
        gmp_randinit_default(global_rand_state);
        gmp_randseed_ui(global_rand_state, time(NULL));
        rand_state_initialized = true;
        
        // 注册清理函数，程序退出时自动清理
        atexit([]() {
            if (rand_state_initialized) {
                gmp_randclear(global_rand_state);
                rand_state_initialized = false;
            }
        });
    }
}

// 获取全局随机数状态
static gmp_randstate_t& get_global_rand_state() {
    init_global_rand_state();
    return global_rand_state;
}

// 运算符重载实现
std::ostream &operator<<(std::ostream &os, const mpz_t &mpz)
{
    char *str = mpz_get_str(nullptr, 10, mpz);
    os << str;
    free(str);
    return os;
}

std::istream &operator>>(std::istream &is, mpz_t &mpz)
{
    std::string str;
    is >> str;
    mpz_set_str(mpz, str.c_str(), 10);
    return is;
}

bool MillerRabin(mpz_t n, int k)
{
    // 处理小于 2 的数
    if (mpz_cmp_ui(n, 2) < 0) {
        return false;
    }
    
    // 处理 2 和 3
    if (mpz_cmp_ui(n, 2) == 0 || mpz_cmp_ui(n, 3) == 0) {
        return true;
    }
    
    // 处理偶数
    if (mpz_even_p(n)) {
        return false;
    }
    
    // 将 n-1 写成 2^r * d 的形式
    mpz_t n_minus_1, d, temp, a, x;
    mpz_init(n_minus_1);
    mpz_init(d);
    mpz_init(temp);
    mpz_init(a);
    mpz_init(x);
    
    mpz_sub_ui(n_minus_1, n, 1);
    mpz_set(d, n_minus_1);
    
    int r = 0;
    while (mpz_even_p(d)) {
        mpz_fdiv_q_ui(d, d, 2);
        r++;
    }
    
    // 使用全局随机数状态
    gmp_randstate_t& state = get_global_rand_state();
    
    // 进行 k 轮测试
    for (int i = 0; i < k; i++) {
        // 生成随机数 a，范围 [2, n-2]
        mpz_sub_ui(temp, n, 3);  // n-3
        mpz_urandomm(a, state, temp);  // [0, n-4]
        mpz_add_ui(a, a, 2);  // [2, n-2]
        
        // 计算 x = a^d mod n
        mpz_powm(x, a, d, n);
        
        // 如果 x = 1 或 x = n-1，则继续下一轮
        if (mpz_cmp_ui(x, 1) == 0 || mpz_cmp(x, n_minus_1) == 0) {
            continue;
        }
        
        // 进行 r-1 次平方
        bool composite = true;
        for (int j = 0; j < r - 1; j++) {
            mpz_powm_ui(x, x, 2, n);
            if (mpz_cmp(x, n_minus_1) == 0) {
                composite = false;
                break;
            }
        }
        
        if (composite) {
            // 清理内存
            mpz_clear(n_minus_1);
            mpz_clear(d);
            mpz_clear(temp);
            mpz_clear(a);
            mpz_clear(x);
            return false;
        }
    }
    
    // 清理内存
    mpz_clear(n_minus_1);
    mpz_clear(d);
    mpz_clear(temp);
    mpz_clear(a);
    mpz_clear(x);
    
    return true;
}

void getPrime(mpz_t p, int bits)
{
    // 使用全局随机数状态
    gmp_randstate_t& state = get_global_rand_state();
    
    mpz_t candidate;
    mpz_init(candidate);
    
    do {
        // 生成指定位数的随机数
        mpz_urandomb(candidate, state, bits);
        
        // 确保最高位为 1（保证位数）
        mpz_setbit(candidate, bits - 1);
        
        // 确保最低位为 1（保证为奇数）
        mpz_setbit(candidate, 0);
        
    } while (!MillerRabin(candidate, 40));  // 使用 40 轮测试，错误概率约为 2^-80
    
    mpz_set(p, candidate);
    
    mpz_clear(candidate);
}

void genSafePrime(mpz_t p, mpz_t q, int bits)
{
    // 使用全局随机数状态
    gmp_randstate_t& state = get_global_rand_state();
    
    mpz_t candidate_q, candidate_p;
    mpz_init(candidate_q);
    mpz_init(candidate_p);
    
    do {
        // 生成 (bits-1) 位的素数 q
        do {
            mpz_urandomb(candidate_q, state, bits - 1);
            mpz_setbit(candidate_q, bits - 2);  // 确保最高位为 1
            mpz_setbit(candidate_q, 0);         // 确保为奇数
        } while (!MillerRabin(candidate_q, 40));
        
        // 计算 p = 2q + 1
        mpz_mul_ui(candidate_p, candidate_q, 2);
        mpz_add_ui(candidate_p, candidate_p, 1);
        
    } while (!MillerRabin(candidate_p, 40));  // 检查 p 是否也是素数
    
    mpz_set(q, candidate_q);
    mpz_set(p, candidate_p);
    
    mpz_clear(candidate_q);
    mpz_clear(candidate_p);
}