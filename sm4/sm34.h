#ifndef SM4_H_
#define SM4_H_
#include <string>
#include <cmath>
#include <iostream>

using namespace std;

/// @brief SM4加密算法
/// @param plain (明文，32位十六进制字符串)
/// @param key  (密钥，32位十六进制字符串)
/// @return 加密后的密文 (32位十六进制字符串)
string sm4_encode(string plain, string key);

/// @brief SM4_ECB加密算法
/// @param plain HEX字符串
/// @param key Hex字符串
/// @return HEX字符串
string sm4_encode_ECB(string plain, string key); //ECB模式加密算法
string sm4_encode_CBC_1(string plain, string key, string IV);//CBC模式加密算法(无填充)

/// @brief SM4_CBC加密算法  
/// @param plain 
/// @param key 
/// @param IV 
/// @return 
string sm4_encode_CBC(string plain, string key, string IV);//CBC模式加密算法
string sm4_decode_CBC_1(string cipher, string key, string IV);//CBC模式解密算法(无填充)

/// @brief SM4解密算法
/// @param cipher (密文，32位十六进制字符串)
/// @param key  (32位十六进制字符串)
/// @return 明文 (32位十六进制字符串)
string sm4_decode(string cipher, string key); //解密算法

/// @brief SM4_ECB解密算法
/// @param cipher 
/// @param key 
/// @return string
string sm4_decode_ECB(string cipher, string key); //ECB模式解密算法

/// @brief SM4_CBC解密算法
/// @param cipher 
/// @param key 
/// @param IV 
/// @return string
string sm4_decode_CBC(string cipher, string key, string IV);//CBC模式解密算法

/// @brief 字符串转十六进制(最多15字符，不足15个字符补0)
/// @param str 
/// @return 32位十六进制字符串
string String2Hex(string str);//字符串转十六进制,补0

/// @brief SM3算法
/// @param str 
/// @param mode，0为字符串，1为HEX
/// @return 256位杂凑值(64位十六进制数)
string sm3(string str, int mode);//sm3算法

string Gen_IV();//生成随机32位HEX 

//辅助函数

string NOT(string str);//实现非操作
string Hex2string(string str);//十六进制转字符串 /
string DecToHex(int num);//十进制转十六进制/
string BinToHex(string str);//二进制转十六进制/
string HexToBin(string str);//十六进制转二进制/
int BinToDec(string str);//二进制转十进制/
string DecToBin(int str);//十进制转二进制   /
int HexToDec(string str);//十六进制转十进制 /
int HexToDec(char str);//十六进制转十进制 /
string LeftShift(string str, int len);//循环左移len位函数/
string XOR(string str1, string str2);//异或函数 /
string NLTransform(string str);//非线性变换函数t /
string LTransform(string str);//线性变换函数L /
string L2Transform(string str);//线性变换函数L' /
string T(string str);//用于加解密的合成置换函数T /
string T2(string str);//用于密钥扩展算法的合成置换函数T'
string KeyExtension(string MK);//密钥扩展算法
string padding(string str, int mode);//填充函数
char binXor(char str1, char str2);//单比特异或
char binAnd(char str1, char str2);//单比特与
string ModAdd(string str1, string str2);//:mod 2^32比特算术加运算
string P1(string str);//P1置换 
string P0(string str);//P0置换
string T(int j);//返回Tj常量
string FF(string x, string y, string z, int j);//FF函数
string GG(string x, string y, string z, int j);//GG函数
string extension(string str);//扩展函数
string compress(string str1, string str2);//压缩函数
string iteration(string str);//迭代函数
string padding(string str, int mode);//填充函数
string AND(string str1, string str2);//与
string OR(string str1, string str2);//或
string StringToHex(string str);//字符串转十六进制

/// @brief 将密钥标准化为15位十六进制字符串，不足15位补0    
/// @param key 
/// @return 16位十六进制字符串
string Key_standardization(string key);//密钥标准化函数


/// @brief 将明文标准化为15位十六进制字符串，不足15位补0
/// @param plain 
/// @return 15位十六进制字符串
string Plain_standardization(string plain);//明文标准化函数


/// @brief 十六进制字符串填充函数
/// @param str (十六进制字符串)
/// @return 
string PKCS7_padding(string str);//PKCS7填充函数

/// @brief 十六进制字符串去填充函数
/// @param str (十六进制字符串)
/// @return
string PKCS7_unpadding(string str);//PKCS7去填充函数


#endif