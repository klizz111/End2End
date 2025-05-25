---
outline: deep
head:
  - - link
    - rel: stylesheet
      href: /katex.min.css
---
# 基于Elgamal的端到端加密通信
:::danger
写于2025-5-25 因为一段傻逼cmake把代码删得啥都不剩 已红温
:::
```sh
# make clean-all
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/test
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "清理所有构建文件和test文件夹"
)
```
## 项目编译
::: tip
使用gmp作为大数运算库,在Ubuntu环境下进行编译，需要运行`apt install libgmp-dev`安装
:::
## 素性检验

## Elgamal加密算法
::: tip 密钥生成算法如下 
$$\begin{aligned}
& 1. \; \text{Gen a } (n-1) \text{ bits prime } q, \text{ let } p = 2q+1 .\\
& 2. \; \text{Choose a random int } h \in [2, p-1] .\\
& 3. \; \text{Calculate } g = h^{2} \; mod \; p , \text{ if } g>1, \text{ then accept as generator} .\\
& \quad \text{Proof: Since } p = 2q+1 \text{ and } q \text{ is prime, ord} (g) \text{ divides } \phi(p) = 2q .\\
& \quad \; \text{If } g^2 \neq 1 \; mod \; p, \text{ then } \text{ord}(g) \in \{q, 2q\} . \text{ Since } g = h^2, \text{ we have } \text{ord}(g) = q .\\
& 4. \; \text{Choose a random int } x \in [1, q-1] .\\
& 5. \; \text{Calculate } y = g^{x} \; mod \; p .\\
& 6. \; \text{The public key is } (p, g, y), \text{ private key is } x .
\end{aligned}
$$
:::

```cpp [elgamal/elgamal.cpp]
void ElGamal::generatePQG() {
    // 生成 p 和 q
    getSafePrime(p, q, bits);
    
    // 选取生成元 g
    mpz_t h, exp;
    mpz_inits(h, exp, NULL);
    mpz_set_ui(exp, 2); // exp = (p-1)/q = 2q/q = 2
    
    while (true) {
        // 生成随机数 h ∈ [2, p-1]
        mpz_urandomm(h, state, p);
        if (mpz_cmp_ui(h, 2) < 0) continue;
        
        // g = h^2 mod p
        mpz_powm(g, h, exp, p);
        if (mpz_cmp_ui(g, 1) > 0) break;
    }
    mpz_clears(h, exp, NULL);
}

void ElGamal::keygen() {
    // x ∈ [1, q-1]
    do {
        mpz_urandomm(x, state, q);
    } while (mpz_cmp_ui(x, 1) < 0);
    // y = g^x mod p
    mpz_powm(y, g, x, p);
}
```

## 程序优化

### 随机数生成优化

- 原先在每次调用`getPrime`和`miller_rabin`时都会重新初始化随机数生成器，后经过修改使用全局随机数状态（`getPrime/getPrime.cpp 4 ~ 22`），避免重复初始化开销。

### 快速幂算法选择
::: details
- 🕱🕱🕱这部分被删完了
- ~~原本使用的快速幂算法是自己编写的基本二进制幂算法（`getPrime/getPrime.cpp 123 ~ 167`），和*gmp*库提供的`mpz_powm`相比，在底数为$2^{41 \sim 43} \, bits$的情况下是自己编写的算法更优，但是偶然发现如果把`miller_rabin`函数的快速幂算法替换为`mpz_powm`（`getPrime/getPrime.cpp 72 ~ 74`）后，虽然`getPrime`的运算时间整体不变，但是如果后续继续调用快速幂算法时，运行时间会有较大提升，推测原因为CPU缓存预热以及`mpz_powm`是宏展开而非函数调用的原因。~~
> ~~此处的代码在`getPrime/test/test_mod.cpp`~~


<Terminal>
Time taken (getPrime) : 170658061 nanoseconds<br/>
Time taken (using mod_exp) : 1802 nanoseconds<br/>
Result: 2667336<br/>
Time taken (using mpz_powm) : 1965 nanoseconds<br/>
Result: 2667336<br/>
</Terminal>

<p style="text-align: center; font-size: 15px;">
    使用自己实现的快速幂算法
</p>

<Terminal>
Time taken (getPrime) : 115933891 nanoseconds<br/>
Time taken (using mod_exp) : 1901 nanoseconds<br/>
Result: 3261320<br/>
Time taken (using mpz_powm) : 587 nanoseconds<br/>
Result: 3261320
</Terminal>

<p style="text-align: center; font-size: 15px;">
    使用gmp库的快速幂算法
</p>
:::

