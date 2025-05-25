#include "sm34.h"

string Hex2string(string str)
{
	string res = "";
	for (int i = 0; i < str.size(); i += 2)
	{
		res += char(HexToDec(str[i]) * 16 + HexToDec(str[i + 1]));
	}
	return res;
}

string DecToHex(int num)
{
	string hex = "";
	int temp = 0;
	if (num < 16)
	{
		hex = '0' + string(1, "0123456789ABCDEF"[num]);
		return hex;
	}
	while (num >= 1)
	{
		temp = num % 16;
		if (temp < 10 && temp >= 0)
		{
			hex = to_string(temp) + hex;
		}
		else
		{
			hex = string(1, 'A' + (temp - 10)) + hex;
		}
		num = num / 16;
	}
	return hex;
}

string BinToHex(string str)
{
	string hex = "";//用来存储最后生成的十六进制数
	int temp = 0;//用来存储每次四位二进制数的十进制值
	while (str.size() % 4 != 0)
	{//因为每四位二进制数就能够成为一个十六进制数，所以将二进制数长度转换为4的倍数
		str = "0" + str;//最高位添0直到长度为4的倍数即可
	}
	for (int i = 0; i < str.size(); i += 4)
	{
		temp = (str[i] - '0') * 8 + (str[i + 1] - '0') * 4 + (str[i + 2] - '0') * 2 + (str[i + 3] - '0') * 1;//判断出4位二进制数的十进制大小为多少
		if (temp < 10)
		{//当得到的值小于10时，可以直接用0-9来代替
			hex += to_string(temp);
		}
		else
		{//当得到的值大于10时，需要进行A-F的转换
			hex += 'A' + (temp - 10);
		}
	}
	return hex;
}

string HexToBin(string str)
{
	string bin = "";
	string table[16] =
	{ "0000","0001","0010","0011","0100","0101","0110","0111","1000","1001","1010","1011","1100","1101","1110","1111" };
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] >= 'A' && str[i] <= 'F')
		{
			bin += table[str[i] - 'A' + 10];
		}
		else
		{
			bin += table[str[i] - '0'];
		}
	}
	return bin;
}

//二进制转换为十进制的函数实现
int BinToDec(string str)
{
	int dec = 0;
	for (int i = 0; i < str.size(); i++)
	{
		dec += (str[i] - '0') * pow(2, str.size() - i - 1);
	}
	return dec;
}

string DecToBin(int str)
{
	string bin = "";
	while (str >= 1)
	{
		bin = to_string(str % 2) + bin;
		str = str / 2;
	}
	return bin;
}

int HexToDec(string str)
{
	int dec = 0;
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] >= 'A' && str[i] <= 'F')
		{
			dec += (str[i] - 'A' + 10) * pow(16, str.size() - i - 1);
		}
		else
		{
			dec += (str[i] - '0') * pow(16, str.size() - i - 1);
		}
	}
	return dec;
}

int HexToDec(char str)
{
	int dec = 0;
	if (str >= 'A' && str <= 'F')
	{
		dec += (str - 'A' + 10);
	}
	else
	{
		dec += (str - '0');
	}
	return dec;
}

string LeftShift(string str, int len)
{
	string res = HexToBin(str);
	res = res.substr(len) + res.substr(0, len);
	return BinToHex(res);
}

string XOR(string str1, string str2)
{
	string res1 = HexToBin(str1);
	string res2 = HexToBin(str2);
	string res = "";
	for (int i = 0; i < res1.size(); i++) {
		if (res1[i] == res2[i]) {
			res += "0";
		}
		else {
			res += "1";
		}
	}
	return BinToHex(res);
}

//非线性变换t函数实现
string NLTransform(string str)
{
	string Sbox[16][16] = { {"D6","90","E9","FE","CC","E1","3D","B7","16","B6","14","C2","28","FB","2C","05"},
							{"2B","67","9A","76","2A","BE","04","C3","AA","44","13","26","49","86","06","99"},
							{"9C","42","50","F4","91","EF","98","7A","33","54","0B","43","ED","CF","AC","62"},
							{"E4","B3","1C","A9","C9","08","E8","95","80","DF","94","FA","75","8F","3F","A6"},
							{"47","07","A7","FC","F3","73","17","BA","83","59","3C","19","E6","85","4F","A8"},
							{"68","6B","81","B2","71","64","DA","8B","F8","EB","0F","4B","70","56","9D","35"},
							{"1E","24","0E","5E","63","58","D1","A2","25","22","7C","3B","01","21","78","87"},
							{"D4","00","46","57","9F","D3","27","52","4C","36","02","E7","A0","C4","C8","9E"},
							{"EA","BF","8A","D2","40","C7","38","B5","A3","F7","F2","CE","F9","61","15","A1"},
							{"E0","AE","5D","A4","9B","34","1A","55","AD","93","32","30","F5","8C","B1","E3"},
							{"1D","F6","E2","2E","82","66","CA","60","C0","29","23","AB","0D","53","4E","6F"},
							{"D5","DB","37","45","DE","FD","8E","2F","03","FF","6A","72","6D","6C","5B","51"},
							{"8D","1B","AF","92","BB","DD","BC","7F","11","D9","5C","41","1F","10","5A","D8"},
							{"0A","C1","31","88","A5","CD","7B","BD","2D","74","D0","12","B8","E5","B4","B0"},
							{"89","69","97","4A","0C","96","77","7E","65","B9","F1","09","C5","6E","C6","84"},
							{"18","F0","7D","EC","3A","DC","4D","20","79","EE","5F","3E","D7","CB","39","48"} };
	string res = "";
	for (int i = 0; i < 4; i++)
	{
		res = res + Sbox[HexToDec(str[2 * i])][HexToDec(str[2 * i + 1])];
	}
	return res;
}


string LTransform(string str)
{
	return XOR(XOR(XOR(XOR(str, LeftShift(str, 2)), LeftShift(str, 10)), LeftShift(str, 18)), LeftShift(str, 24));
}

string L2Transform(string str)
{
	return XOR(XOR(str, LeftShift(str, 13)), LeftShift(str, 23));
}

string T(string str)
{
	return LTransform(NLTransform(str));
}

string T2(string str)
{
	return L2Transform(NLTransform(str));
}


string KeyExtension(string MK)
{
	string FK[4] = { "A3B1BAC6", "56AA3350", "677D9197", "B27022DC" };
	string CK[32] = { "00070E15", "1C232A31", "383F464D", "545B6269",
						"70777E85", "8C939AA1", "A8AFB6BD", "C4CBD2D9",
						"E0E7EEF5", "FC030A11", "181F262D", "343B4249",
						"50575E65", "6C737A81", "888F969D", "A4ABB2B9",
						"C0C7CED5", "DCE3EAF1", "F8FF060D", "141B2229",
						"30373E45", "4C535A61", "686F767D", "848B9299",
						"A0A7AEB5", "BCC3CAD1", "D8DFE6ED", "F4FB0209",
						"10171E25", "2C333A41", "484F565D", "646B7279" };
	string K[36] = { XOR(MK.substr(0,8),FK[0]),XOR(MK.substr(8,8),FK[1]),XOR(MK.substr(16,8),FK[2]),XOR(MK.substr(24),FK[3]) };
	string rks = "";
	for (int i = 0; i < 32; i++) {
		K[i + 4] = XOR(K[i], T2(XOR(XOR(XOR(K[i + 1], K[i + 2]), K[i + 3]), CK[i])));
		rks += K[i + 4];
	}
	return rks;
}

string sm4_decode_ECB(string cipher, string key)
{
	string res = "";
	auto key__ = PKCS7_padding(key);
	if (key__.size() > 32)
	{
		key__ = key__.substr(0, 32);
	}
	for (int i = 0; i < cipher.size(); i += 32)
	{
		res += sm4_decode(cipher.substr(i, 32), key__);
	}
	return PKCS7_unpadding(res);
}

string sm4_decode_CBC(string cipher, string key, string IV)
{
	string res = "";
	auto key__ = PKCS7_padding(key);
	string temp = IV;
	if (key__.size() > 32)
	{
		key__ = key__.substr(0, 32);
	}
	if (temp.size() > 32)
	{
		temp = temp.substr(0, 32);
	}
	for (int i = 0; i < cipher.size(); i += 32)
	{
		res += XOR(sm4_decode(cipher.substr(i, 32), key__), temp);
		temp = cipher.substr(i, 32);
	}
	return PKCS7_unpadding(res);
}

string String2Hex(string str)
{
	string res = "";
	for (int i = 0; i < str.size(); i++)
	{
		res += DecToHex(int(str[i]));
	}
	return res;
}


string Key_standardization(string key)
{
	return String2Hex(key);
}

string Plain_standardization(string plain)
{
	return String2Hex(plain);
}

string sm4_encode(string plain, string key)
{
	string cipher[36] = { plain.substr(0,8),plain.substr(8,8),plain.substr(16,8),plain.substr(24) };
	string rks = KeyExtension(key);
	for (int i = 0; i < 32; i++)
	{
		cipher[i + 4] = XOR(cipher[i], T(XOR(XOR(XOR(cipher[i + 1], cipher[i + 2]), cipher[i + 3]), rks.substr(8 * i, 8))));
	}
	return cipher[35] + cipher[34] + cipher[33] + cipher[32];
}

string sm4_encode_ECB(string plain, string key)
{
	string res = "";
	auto plain__ = PKCS7_padding(plain);
	auto key__ = PKCS7_padding(key);
	if (key__.size() > 32)
	{
		key__ = key__.substr(0, 32);
	}
	for (int i = 0; i < plain__.size(); i += 32)
	{
		res += sm4_encode(plain__.substr(i, 32), key__);
	}
	return res;
}

string sm4_encode_CBC_1(string plain, string key, string IV)
{
	string res = "";
	for (int i = 0; i < plain.size(); i += 32)
	{
		res += sm4_encode(XOR(plain.substr(i, 32), IV), key);
		IV = res.substr(i, 32);
	}
	return res;
}

string sm4_encode_CBC(string plain, string key, string IV)
{
	string res = "";
	auto plain__ = PKCS7_padding(plain);
	auto key__ = PKCS7_padding(key);
	string temp = IV;
	if (temp.size() > 32)
	{
		temp = temp.substr(0, 32);
	}
	if (key__.size() > 32)
	{
		key__ = key__.substr(0, 32);
	}
	for (int i = 0; i < plain__.size(); i += 32)
	{
		temp = sm4_encode(XOR(plain__.substr(i, 32), temp), key__);
		res += temp;
	}
	return res;
}

//CBC模式解密算法(无填充)
string sm4_decode_CBC_1(string cipher, string key, string IV)
{
	string res = "";
	for (int i = 0; i < cipher.size(); i += 32)
	{
		res += XOR(sm4_decode(cipher.substr(i, 32), key), IV);
		IV = cipher.substr(i, 32);
	}
	return res;
}


string sm4_decode(string cipher, string key)
{
	string plain[36] = { cipher.substr(0,8),cipher.substr(8,8), cipher.substr(16,8), cipher.substr(24,8) };
	string rks = KeyExtension(key);
	for (int i = 0; i < 32; i++)
	{
		plain[i + 4] = XOR(plain[i], T(XOR(XOR(XOR(plain[i + 1], plain[i + 2]), plain[i + 3]), rks.substr(8 * (31 - i), 8))));
	}
	return plain[35] + plain[34] + plain[33] + plain[32];
}

//0为字符串，1为HEX
string padding(string str, int mode)
{
	string res = "";

	if (mode == 0)//字符串
	{
		for (int i = 0; i < str.size(); i++)
		{//首先将输入值转换为16进制字符串
			res += DecToHex((int)str[i]);
		}
	}

	else if (mode == 1)//HEX
	{
		res = str;
	}

	int res_length = res.size() * 4;//记录的长度为2进制下的长度
	res += "8";//在获得的数据后面添1，在16进制下相当于是添加8
	while (res.size() % 128 != 112)
	{
		res += "0";//“0”数据填充
	}
	string res_len = DecToHex(res_length);//用于记录数据长度的字符串
	while (res_len.size() != 16)
	{
		res_len = "0" + res_len;
	}
	res += res_len;
	return res;
}

string AND(string str1, string str2)
{
	string res1 = HexToBin(str1);
	string res2 = HexToBin(str2);
	string res = "";
	for (int i = 0; i < res1.size(); i++)
	{
		if (res1[i] == '1' && res2[i] == '1')
		{
			res += "1";
		}
		else
		{
			res += "0";
		}
	}
	return BinToHex(res);
}

string OR(string str1, string str2)
{
	string res1 = HexToBin(str1);
	string res2 = HexToBin(str2);
	string res = "";
	for (int i = 0; i < res1.size(); i++)
	{
		if (res1[i] == '0' && res2[i] == '0')
		{
			res += "0";
		}
		else
		{
			res += "1";
		}
	}
	return BinToHex(res);
}

string StringToHex(string str)
{
	string res = "";
	for (int i = 0; i < str.size(); i++)
	{
		res += DecToHex((int)str[i]);
	}
	return res;
}

char binXor(char str1, char str2)
{
	return str1 == str2 ? '0' : '1';
}

char binAnd(char str1, char str2)
{
	return (str1 == '1' && str2 == '1') ? '1' : '0';
}

string ModAdd(string str1, string str2)
{
	string res1 = HexToBin(str1);
	string res2 = HexToBin(str2);
	char temp = '0';
	string res = "";
	for (int i = res1.size() - 1; i >= 0; i--)
	{
		res = binXor(binXor(res1[i], res2[i]), temp) + res;
		if (binAnd(res1[i], res2[i]) == '1')
		{
			temp = '1';
		}
		else
		{
			if (binXor(res1[i], res2[i]) == '1')
			{
				temp = binAnd('1', temp);
			}
			else
			{
				temp = '0';
			}
		}
	}
	return BinToHex(res);
}

//实现置换功能P1（X）
string P1(string str)
{
	return XOR(XOR(str, LeftShift(str, 15)), LeftShift(str, 23));
}

//实现置换功能P0（X）
string P0(string str)
{
	return XOR(XOR(str, LeftShift(str, 9)), LeftShift(str, 17));
}

//返回Tj常量值的函数实现
string T(int j)
{
	if (0 <= j && j <= 15)
	{
		return "79CC4519";
	}
	else
	{
		return "7A879D8A";
	}
}

//实现布尔函数FF功能
string FF(string str1, string str2, string str3, int j)
{
	if (0 <= j && j <= 15)
	{
		return XOR(XOR(str1, str2), str3);
	}
	else
	{
		return OR(OR(AND(str1, str2), AND(str1, str3)), AND(str2, str3));
	}
}

//实现非操作
string NOT(string str)
{
	string res1 = HexToBin(str);
	string res = "";
	for (int i = 0; i < res1.size(); i++)
	{
		if (res1[i] == '0')
		{
			res += "1";
		}
		else
		{
			res += "0";
		}
	}
	return BinToHex(res);
}

//实现布尔函数GG功能
string GG(string str1, string str2, string str3, int j)
{
	if (0 <= j && j <= 15)
	{
		return XOR(XOR(str1, str2), str3);
	}
	else
	{
		return OR(AND(str1, str2), AND(NOT(str1), str3));
	}
}

//消息扩展函数
string extension(string str)
{
	string res = str;//字符串类型存储前68位存储扩展字W值
	for (int i = 16; i < 68; i++)
	{//根据公式生成第17位到第68位的W值
		res += XOR(XOR(P1(XOR(XOR(res.substr((i - 16) * 8, 8), res.substr((i - 9) * 8, 8)), LeftShift(res.substr((i - 3) * 8, 8), 15))), LeftShift(res.substr((i - 13) * 8, 8), 7)), res.substr((i - 6) * 8, 8));
	}
	for (int i = 0; i < 64; i++)//根据公式生成64位W'值
	{
		res += XOR(res.substr(i * 8, 8), res.substr((i + 4) * 8, 8));
	}
	return res;
}

//消息压缩函数
string compress(string str1, string str2)
{
	string IV = str2;
	string A = IV.substr(0, 8), B = IV.substr(8, 8), C = IV.substr(16, 8), D = IV.substr(24, 8), E = IV.substr(32, 8), F = IV.substr(40, 8), G = IV.substr(48, 8), H = IV.substr(56, 8);
	string SS1 = "", SS2 = "", TT1 = "", TT2 = "";
	for (int j = 0; j < 64; j++)
	{
		SS1 = LeftShift(ModAdd(ModAdd(LeftShift(A, 12), E), LeftShift(T(j), (j % 32))), 7);
		SS2 = XOR(SS1, LeftShift(A, 12));
		TT1 = ModAdd(ModAdd(ModAdd(FF(A, B, C, j), D), SS2), str1.substr((j + 68) * 8, 8));
		TT2 = ModAdd(ModAdd(ModAdd(GG(E, F, G, j), H), SS1), str1.substr(j * 8, 8));
		D = C;
		C = LeftShift(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = LeftShift(F, 19);
		F = E;
		E = P0(TT2);
	}
	string res = (A + B + C + D + E + F + G + H);
	return res;
}

//迭代压缩函数实现
string iteration(string str)
{
	int num = str.size() / 128;
	string V = "7380166F4914B2B9172442D7DA8A0600A96F30BC163138AAE38DEE4DB0FB0E4E";
	string B = "", extensionB = "", compressB = "";
	for (int i = 0; i < num; i++)
	{
		B = str.substr(i * 128, 128);
		extensionB = extension(B);
		compressB = compress(extensionB, V);
		V = XOR(V, compressB);
	}
	return V;
}

string sm3(string str, int mode)
{
	string res = padding(str, mode);
	return iteration(res);
}

string Gen_IV()
{
	string BOX = "0123456789ABCDEF";
	string res = "";
	for (int i = 0; i < 32; i++)
	{
		res += BOX[rand() % 16];
	}
	return res;
}

string PKCS7_padding(string hex_str)
{
	string res = "";
	res = hex_str;
	string str = Hex2string(hex_str);
	int len = str.size();
	int pad = 16 - (len % 16);
	string pad_str = DecToHex(pad);
	for (int i = 0; i < pad; i++)
	{
		res += pad_str;
	}
	return res;
}

string PKCS7_unpadding(string str)
{
	string len = str.substr(str.size() - 2);
	if (len == "80")
	{
		return str.substr(0, str.size() - 16);
	}
	else
	{
		int pad = HexToDec(len);
		return str.substr(0, str.size() - 2 * pad);
	}

}
