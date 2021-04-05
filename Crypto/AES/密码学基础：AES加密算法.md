# 密码学基础：AES加密算法

# 基础部分概述：

> - **本节目的：**这一章作为AES算法的基础部分，目的主要是整理下密码学中AES加密与解密的相关知识点，并把它们分享出来。
> - **阅读方法：**希望大家在浏览完本章文章后可以自己去实现一下，相信一定会对你的编程技术有所提高。(附件中提供参考代码)
> - **具备基础：**
>   (1)熟练掌握C语言
>   (2)相关数学知识
> - **学习环境：**任意C语言开发环境，一个正确的AES算法程序(方便调试，验证程序结果)

------

# 第一节：AES算法简介

　　**AES的全称是Advanced Encryption Standard，意思是高级加密标准。它的出现主要是为了取代DES加密算法的，因为我们都知道DES算法的密钥长度是56Bit，因此算法的理论安全强度是2的56次方。但二十世纪中后期正是计算机飞速发展的阶段，元器件制造工艺的进步使得计算机的处理能力越来越强，虽然出现了3DES的加密方法，但由于它的加密时间是DES算法的3倍多，64Bit的分组大小相对较小，所以还是不能满足人们对安全性的要求。于是1997年1月2号，美国国家标准技术研究所宣布希望征集高级加密标准，用以取代DES。AES也得到了全世界很多密码工作者的响应，先后有很多人提交了自己设计的算法。最终有5个候选算法进入最后一轮：Rijndael，Serpent，Twofish，RC6和MARS。最终经过安全性分析、软硬件性能评估等严格的步骤，Rijndael算法获胜。**

 

　　**在密码标准征集中，所有AES候选提交方案都必须满足以下标准：**

- **分组大小为128位的分组密码。**
- **必须支持三种密码标准：128位、192位和256位。**
- **比提交的其他算法更安全。**
- **在软件和硬件实现上都很高效。**

　　**AES密码与分组密码Rijndael基本上完全一致，Rijndael分组大小和密钥大小都可以为128位、192位和256位。然而AES只要求分组大小为128位，因此只有分组长度为128Bit的Rijndael才称为AES算法。本文只对分组大小128位，密钥长度也为128位的Rijndael算法进行分析。密钥长度为192位和256位的处理方式和128位的处理方式类似，只不过密钥长度每增加64位，算法的循环次数就增加2轮，128位循环10轮、192位循环12轮、256位循环14轮。**
![AES输入输出](images/813468_VDYYGCCJ2BCCVVT.png)

------

# 第二节：AES算法相关数学知识

　　**在AES算法中的MixColumn层中会用到伽罗瓦域中的乘法运算，而伽罗瓦域的运算涉及一些数学知识，所以本节将会介绍其相关的知识帮助大家了解，知识不难看过就清楚了。**

## 素域简介

　　**有限域有时也称伽罗瓦域，它指的是由有限个元素组成的集合，在这个集合内可以执行加、减、乘和逆运算。而在密码编码学中，我们只研究拥有有限个元素的域，也就是有限域。域中包含元素的个数称为域的阶。只有当m是一个素数幂时，即m=p^n(其中n为正整数是p的次数，p为素数)，阶为m的域才存在。p称为这个有限域的特征。**

 

　　**也就是说，有限域中元素的个数可以是11(p=11是一个素数,n=1)、可以是81(p=3是一个素数，n=4)、也可以是256(p=2是一个素数，n=8).....但有限域的中不可能拥有12个元素，因为12=2·2·3，因此12也不是一个素数幂。**

 

　　**有限域中最直观的例子就是阶为素数的域，即n=1的域。域GF(p)的元素可以用整数0、1、...、p-1l来表示。域的两种操作就是模整数加法和整数乘法模p。加上p是一个素数，整数环Z表示为GF(p)，也成为拥有素数个元素的素数域或者伽罗瓦域。GF(p)中所有的非零元素都存在逆元，GF(p)内所有的运算都是模p实现的。**

 

　　**素域内的算数运算规则如下：(1)加法和乘法都是通过模p实现的；(2)任何一个元素a的加法逆元都是由a+(a的逆元)=0 mod p得到的；(3)任何一个非零元素a的乘法逆元定义为a·a的逆元=1。举个例子，在素域GF(5)={0、1、2、3、4}中，2的加法逆元为3，这是因为2+(3)=5，5mod5=0,所以2+3=5mod5=0。2的乘法逆元为3，这是因为2·3=6，6mod5=1，所以2·3=6mod5=1。(在很多地方a的加法逆元用-a表示，a的乘法逆元用1/a表示)**

 

**注：GF(2)是一个非常重要的素域，也是存在的最小的有限域，由于GF(2)的加法，即模2加法与异或(XOR)门等价，GF(2)的乘法与逻辑与(AND)门等价，所以GF(2)对AES非常重要。**

## 扩展域简介

　　**如果有限域的阶不是素数，则这样的有限域内的加法和乘法运算就不能用模整数加法和整数乘法模p表示。而且m>1的域被称为扩展域，为了处理扩展域，我们就要使用不同的符号表示扩展域内的元素，使用不同的规则执行扩展域内元素的算术运算。**
　　**在扩展域GF(2^m)中，元素并不是用整数表示的，而是用系数为域GF(2)中元素的多项式表示。这个多项式最大的度(幂)为m-1，所以每个元素共有m个系数，在AES算法使用的域GF(2^8)中，每个元素A∈GF(2^8)都可以表示为：**
![公式1](images/813468_Z8DYKF8937ZWEMM.png)
**注意：在域GF(2^8)中这样的多项式共有256个，这256个多项式组成的集合就是扩展域GF(2^8)。每个多项式都可以按一个8位项链的数值形式存储：**
![公式2](images/813468_KBYA8KM843C3H3E.png)
**像x^7、x^6等因子都无需存储，因为从位的位置就可以清楚地判断出每个系数对应的幂。**

## 扩展域GF(2^m)内的加减法

　　**在AES算法中的密钥加法层中就使用了这部分的知识，但是不是很明显，因为我们通常把扩展域中的加法当作异或运算进行处理了，因为在扩展域中的加减法处理都是在底层域GF(2)内完成的，与按位异或运算等价。假设A(x)、B(x)∈GF(2^m)，计算两个元素之和的方法就是：**
![公式3](images/813468_PW72XTRWEQXQHTW.png)
**而两个元素之差的计算公式就是：**
![公式4](images/813468_PBED8TS3HPE4PHY.png)
**注：在减法运算中减号之所以变成加号，这就和二进制减法的性质有关了，大家可以试着验算下。从上述两个公式中我们发现在扩展域中加法和减法等价，并且与XOR等价(异或运算也被称作二进制加法)。**

## 扩展域GF(2^m)内的乘法

　　**扩展域的乘法主要运用在AES算法的列混淆层(Mix Column)中，也是列混淆层中最重要的操作。我们项要将扩展域中的两个元素用多项式形式展开，然后使用标准的多项式乘法规则将两个多项式相乘：
![公式5](images/813468_UFDWBAVGBPU3EQQ.png)

 

**注意：通常在多项式乘法中C(x)的度会大于m-1，因此需要对此进行化简，而化简的基本思想与素域内乘法情况相似：在素域GF(p)中，将两个整数相乘得到的结果除以一个素数，化简后的结果就是最后的余数。而在扩展域中进行的操作就是：将两个多项式相乘的结果除以一个不可约多项式，最后的结果就是最后的余数。(这里的不可约多项式大致可以看作一个素数)**
![公式6](images/813468_HADYDCJW9QH79R8.png)

 

**举例：** ![例题](images/813468_3RNGQGNWRNX5DAA.png)

------

# 第三节：AES算法原理

　　**AES算法主要有四种操作处理，分别是密钥加法层(也叫轮密钥加，英文Add Round Key)、字节代换层(SubByte)、行位移层(Shift Rows)、列混淆层(Mix Column)。而明文x和密钥k都是由16个字节组成的数据(当然密钥还支持192位和256位的长度，暂时不考虑)，它是按照字节的先后顺序从上到下、从左到右进行排列的。而加密出的密文读取顺序也是按照这个顺序读取的，相当于将数组还原成字符串的模样了，然后再解密的时候又是按照4·4数组处理的。AES算法在处理的轮数上只有最后一轮操作与前面的轮处理上有些许不同(最后一轮只是少了列混淆处理)，在轮处理开始前还单独进行了一次轮密钥加的处理。在处理轮数上，我们只考虑128位密钥的10轮处理。接下来，就开始一步步的介绍AES算法的处理流程了。**
![字节排列](images/813468_2NTG4S4C797TXPY.png)
**AES算法流程图**
![AES流程图](images/813468_XJ9BX7PYB2XD7VJ.png)

## 密钥加法层

　　**在密钥加法层中有两个输入的参数，分别是明文和子密钥k[0]，而且这两个输入都是128位的。k[0]实际上就等同于密钥k，具体原因在密钥生成中进行介绍。我们前面在介绍扩展域加减法中提到过，在扩展域中加减法操作和异或运算等价，所以这里的处理也就异常的简单了，只需要将两个输入的数据进行按字节异或操作就会得到运算的结果。**

 

**图示：**
![密钥加法层](images/813468_W6HMQKZYX3JNJCR.png)

 

**相关代码：**

```
int AddRoundKey(unsigned char(*PlainArray)[4], unsigned char(*ExtendKeyArray)[44], unsigned int MinCol)
{
    int ret = 0;
 
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            PlainArray[i][j] ^= ExtendKeyArray[i][MinCol + j];
        }
    }
 
    return ret;
}
```

## 字节代换层

　　**字节代换层的主要功能就是让输入的数据通过S_box表完成从一个字节到另一个字节的映射，这里的S_box表是通过某种方法计算出来的，具体的计算方法将在进阶部分进行介绍，我们基础部分就只给出计算好的S_box结果。S_box表是一个拥有256个字节元素的数组，可以将其定义为一维数组，也可以将其定义为16·16的二维数组，如果将其定义为二维数组，读取S_box数据的方法就是要将输入数据的每个字节的高四位作为第一个下标，第四位作为第二个下标，略有点麻烦。这里建议将其视作一维数组即可。逆S盒与S盒对应，用于解密时对数据处理，我们对解密时的程序处理称作逆字节代换，只是使用的代换表盒加密时不同而已。**

 

**S盒**
![S_box](images/813468_VPB3XZYHRZCUTSD.png)

 

**逆S盒**
![reS_box](images/813468_45TJDUZNEGVNGB4.png)

 

**加密图示：**
![字节代换层](images/813468_K2PPNQM38C52FHQ.png)

 

**相关代码：**

```
//S盒
const unsigned char S_Table[16][16] =
{
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};
 
//字节代换
int Plain_S_Substitution(unsigned char *PlainArray)
{
    int ret = 0;
 
    for (int i = 0; i < 16; i++)
    {
        PlainArray[i] = S_Table[PlainArray[i] >> 4][PlainArray[i] & 0x0F];
    }
 
    return ret;
}
 
 
//逆S盒
const unsigned char ReS_Table[16][16] =
{
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};
//逆字节代换
int Cipher_S_Substitution(unsigned char *CipherArray)
{
    int ret = 0;
 
    for (int i = 0; i < 16; i++)
    {
        CipherArray[i] = ReS_Table[CipherArray[i] >> 4][CipherArray[i] & 0x0F];
    }
 
    return ret;
}
```

## 行位移——ShiftRows

　　**行位移操作最为简单，它是用来将输入数据作为一个4·4的字节矩阵进行处理的，然后将这个矩阵的字节进行位置上的置换。ShiftRows子层属于AES手动的扩散层，目的是将单个位上的变换扩散到影响整个状态当，从而达到雪崩效应。在加密时行位移处理与解密时的处理相反，我们这里将解密时的处理称作逆行位移。它之所以称作行位移，是因为它只在4·4矩阵的行间进行操作，每行4字节的数据。在加密时，保持矩阵的第一行不变，第二行向左移动8Bit(一个字节)、第三行向左移动2个字节、第四行向左移动3个字节。而在解密时恰恰相反，依然保持第一行不变，将第二行向右移动一个字节、第三行右移2个字节、第四行右移3个字节。操作结束！**

 

**正向行位移图解:**
![正向行位移](images/813468_QC94QRQKGMUEYPR.png)

 

**对应代码(这里将char二维数组强制转换成int一维数组处理)：**

```
int ShiftRows(unsigned int *PlainArray)
{
    int ret = 0;
 
    //第一行 不移位
    //PlainArray[0] = PlainArray[0];
 
    //第二行 左移8Bit
    PlainArray[1] = (PlainArray[1] >> 8) | (PlainArray[1] << 24);
 
    //第三行 左移16Bit
    PlainArray[2] = (PlainArray[2] >> 16) | (PlainArray[2] << 16);
 
    //第四行 左移24Bit
    PlainArray[3] = (PlainArray[3] >> 24) | (PlainArray[3] << 8);
 
    return ret;
}
```

**逆向行位移图解:**
![逆向行位移](images/813468_NN4U4URSVQY4W69.png)

 

**对应代码(这里将char二维数组强制转换成int一维数组处理)：**

```
int ReShiftRows(unsigned int *CipherArray)
{
    int ret = 0;
 
    //第一行 不移位
    //CipherArray[0] = CipherArray[0];
 
    //第二行 右移8Bit
    CipherArray[1] = (CipherArray[1] << 8) | (CipherArray[1] >> 24);
 
    //第三行 右移16Bit
    CipherArray[2] = (CipherArray[2] << 16) | (CipherArray[2] >> 16);
 
    //第四行 右移24Bit
    CipherArray[3] = (CipherArray[3] << 24) | (CipherArray[3] >> 8);
 
    return ret;
}
```

## 列混淆——MixColumn

　　**列混淆子层是AES算法中最为复杂的部分，属于扩散层，列混淆操作是AES算法中主要的扩散元素，它混淆了输入矩阵的每一列，使输入的每个字节都会影响到4个输出字节。行位移子层和列混淆子层的组合使得经过三轮处理以后，矩阵的每个字节都依赖于16个明文字节成可能。其中包含了矩阵乘法、伽罗瓦域内加法和乘法的相关知识。**

 

　　**在加密的正向列混淆中，我们要将输入的4·4矩阵左乘一个给定的4·4矩阵。而它们之间的加法、乘法都在扩展域GF(2^8)中进行，所以也就可以将这一个步骤分成两个部分进行讲解：**

 

**先上一个矩阵乘法的代码：**

```
//列混淆左乘矩阵
const unsigned char MixArray[4][4] =
{
    0x02, 0x03, 0x01, 0x01,
    0x01, 0x02, 0x03, 0x01,
    0x01, 0x01, 0x02, 0x03,
    0x03, 0x01, 0x01, 0x02
};
 
int MixColum(unsigned char(*PlainArray)[4])
{
    int ret = 0;
    //定义变量
    unsigned char ArrayTemp[4][4];
 
    //初始化变量
    memcpy(ArrayTemp, PlainArray, 16);
 
    //矩阵乘法 4*4
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            PlainArray[i][j] =
                MixArray[i][0] * ArrayTemp[0][j] +
                MixArray[i][1] * ArrayTemp[1][j] +
                MixArray[i][2] * ArrayTemp[2][j] +
                MixArray[i][3] * ArrayTemp[3][j];
        }
    }
 
    return ret;
}
```

　　**我们发现在矩阵乘法中，出现了加法和乘法运算，我们前面也提到过在扩展域中加法操作等同于异或运算，而乘法操作需要一个特殊的方式进行处理，于是我们就先把代码中的加号换成异或符号，然后将伽罗瓦域的乘法定义成一个有两个参数的函数，并让他返回最后计算结果。于是列混淆的代码就会变成下面的样子：**

```
const unsigned char MixArray[4][4] =
{
    0x02, 0x03, 0x01, 0x01,
    0x01, 0x02, 0x03, 0x01,
    0x01, 0x01, 0x02, 0x03,
    0x03, 0x01, 0x01, 0x02
};
 
int MixColum(unsigned char(*PlainArray)[4])
{
    int ret = 0;
    //定义变量
    unsigned char ArrayTemp[4][4];
 
    //初始化变量
    memcpy(ArrayTemp, PlainArray, 16);
 
    //矩阵乘法 4*4
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            PlainArray[i][j] =
                GaloisMultiplication(MixArray[i][0], ArrayTemp[0][j]) ^
                GaloisMultiplication(MixArray[i][1], ArrayTemp[1][j]) ^
                GaloisMultiplication(MixArray[i][2], ArrayTemp[2][j]) ^
                GaloisMultiplication(MixArray[i][3], ArrayTemp[3][j]);
        }
    }
    return ret;
}
```

　　**接下来我们就只用处理伽罗瓦域乘法相关处理了，由于前面介绍过相关概念，所以代码就不在此进行讲解了，大家可以参考下方的代码注释进行理解：**

```
///////////////////////////////////////////////////////////////
//功能:   伽罗瓦域内的乘法运算  GF(128)
//参数:   Num_L           输入的左参数
//      Num_R           输入的右参数
//返回值:计算结果
char GaloisMultiplication(unsigned char Num_L, unsigned char Num_R)
{
    //定义变量
    unsigned char Result = 0;       //伽罗瓦域内乘法计算的结果
 
    while (Num_L)
    {
        //如果Num_L最低位是1就异或Num_R，相当于加上Num_R * 1
        if (Num_L & 0x01)
        {
            Result ^= Num_R;
        }
 
        //Num_L右移一位，相当于除以2
        Num_L = Num_L >> 1;
 
        //如果Num_R最高位为1
        if (Num_R & 0x80)
        {
            //左移一位相当于乘二
            Num_R = Num_R << 1;     //注：这里会丢失最高位，但是不用担心
 
            Num_R ^= 0x1B;  //计算伽罗瓦域内除法Num_R = Num_R / (x^8(刚好丢失最高位) + x^4 + x^3 + x^1 + 1)
        }
        else
        {
            //左移一位相当于乘二
            Num_R = Num_R << 1;
        }
    }
    return Result;
}
```

　　**在解密的逆向列混淆中与正向列混淆的不同之处在于使用的左乘矩阵不同，它与正向列混淆的左乘矩阵互为逆矩阵，也就是说，数据矩阵同时左乘这两个矩阵后，数据矩阵不会发生任何变化。**

 

**正向列混淆处理**
![正向列混淆](images/813468_UES487VAAZPKDFS.png)

 

**逆向列混淆**
![逆向列混淆](images/813468_ZCDXZKFMZA7K9SD.png)

 

**加解密验证**
![验证](images/813468_CY9JWPVZNVZD3ET.png)

 

**加密部分讲解完毕，最后应该注意要将密文结果从矩阵形式还原成字符串形式输出！**

------

# 第四节：AES密钥生成

![密钥生成流程](images/813468_CHWREHFQKJRYJR8.png)

 

　　**子密钥的生成是以列为单位进行的，一列是32Bit，四列组成子密钥共128Bit。生成子密钥的数量比AES算法的轮数多一个，因为第一个密钥加法层进行密钥漂白时也需要子密钥。密钥漂白是指在AES的输入盒输出中都使用的子密钥的XOR加法。子密钥在图中都存储在W[0]、W[1]、...、W[43]的扩展密钥数组之中。k1-k16表示原始密钥对应的字节，而图中子密钥k0与原始子密钥相同。在生成的扩展密钥中W的下标如果是4的倍数时(从零开始)需要对异或的参数进行G函数处理。扩展密钥生成有关公式如下：**

```
1<= i <= 10
1<= j <= 3
w[4i]     = W[4(i-1)] + G(W[4i-1]);
w[4i+j]   = W[4(i-1)+j] + W[4i-1+j];
```

![扩展密钥组](images/813468_ESXF84G6FPB4AP6.png)

 

　　**函数G()首先将4个输入字节进行翻转，并执行一个按字节的S盒代换，最后用第一个字节与轮系数Rcon进行异或运算。轮系数是一个有10个元素的一维数组，一个元素1个字节。G()函数存在的目的有两个，一是增加密钥编排中的非线性；二是消除AES中的对称性。这两种属性都是抵抗某些分组密码攻击必要的。**
![轮系数](images/813468_2TE266M7PETUFED.png)

 

**生成密钥代码:**

```
//用于密钥扩展    Rcon[0]作为填充，没有实际用途
const unsigned int Rcon[11] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36 };
 
 
int Key_S_Substitution(unsigned char(*ExtendKeyArray)[44], unsigned int nCol)
{
    int ret = 0;
 
    for (int i = 0; i < 4; i++)
    {
        ExtendKeyArray[i][nCol] = S_Table[(ExtendKeyArray[i][nCol]) >> 4][(ExtendKeyArray[i][nCol]) & 0x0F];
    }
 
    return ret;
}
 
 
int G_Function(unsigned char(*ExtendKeyArray)[44], unsigned int nCol)
{
    int ret = 0;
 
    //1、将扩展密钥矩阵的nCol-1列复制到nCol列上，并将nCol列第一行的元素移动到最后一行，其他行数上移一行
    for (int i = 0; i < 4; i++)
    {
        ExtendKeyArray[i][nCol] = ExtendKeyArray[(i + 1) % 4][nCol - 1];
    }
 
    //2、将nCol列进行S盒替换
    Key_S_Substitution(ExtendKeyArray, nCol);
 
    //3、将该列第一行元素与Rcon进行异或运算
    ExtendKeyArray[0][nCol] ^= Rcon[nCol / 4];
 
    return ret;
}
 
 
int CalculateExtendKeyArray(const unsigned char(*PasswordArray)[4], unsigned char(*ExtendKeyArray)[44])
{
    int ret = 0;
 
    //1、将密钥数组放入前四列扩展密钥组
    for (int i = 0; i < 16; i++)
    {
        ExtendKeyArray[i & 0x03][i >> 2] = PasswordArray[i & 0x03][i >> 2];
    }
 
    //2、计算扩展矩阵的后四十列
    for (int i = 1; i < 11; i++)    //进行十轮循环
    {
        //(1)如果列号是4的倍数，这执行G函数  否则将nCol-1列复制到nCol列上
        G_Function(ExtendKeyArray, 4*i);
 
        //(2)每一轮中，各列进行异或运算
        //      列号是4的倍数
        for (int k = 0; k < 4; k++)//行号
        {
            ExtendKeyArray[k][4 * i] = ExtendKeyArray[k][4 * i] ^ ExtendKeyArray[k][4 * (i - 1)];
        }
 
        //      其他三列
        for (int j = 1; j < 4; j++)//每一轮的列号
        {
            for (int k = 0; k < 4; k++)//行号
            {
                ExtendKeyArray[k][4 * i + j] = ExtendKeyArray[k][4 * i + j - 1] ^ ExtendKeyArray[k][4 * (i - 1) + j];
            }
        }
    }
 
    return ret;
}

```

------

# 第五节：AES解密流程图

![解密流程图](images/813468_JQP55QA4VDRPSYN.png)

 

**至此，AES算法基础部分介绍完毕！**

------

# 进阶部分概述：

> - **本节目的：**这一章作为AES算法的进阶部分，目的主要是对AES算法中的S盒的建立做一些介绍。
> - **阅读方法：**希望大家在浏览完本章文章后可以自己去实现一下，相信一定会对你的编程技术有所提高。(附件中提供参考代码)
> - **具备基础：**
>   (1)熟练掌握C语言
>   (2)相关数学知识
> - **学习环境：**任意C语言开发环境

------

# 第六节：相关的数学知识

　　**在接触密码学之前我认为数学的主要用途就是考试。。。但是接触密码学后我才发现数学的魅力，虽然数学只是一个工具，但这个工具却异常强大，我也因此吃了以前不认真学习相关知识的亏。好了，不多说了现在学习还不算太迟。**

## 欧几里得算法：

　　**两个正整数r0和r1的gcd表示为gcd(r0, r1)，它指的是可以被r0和r1同时整除的最大正整数，例如gcd(21, 6)=3。对与较小的整数而言gcd就是将两个整数进行因式分解，并找出最大的公因子。
例：r0=84,r1=30，因式分解：r0=2·2·3·7；r1=2·3·5；gcd的结果就是：gcd(30,84)=2·3=6。**
　　**gcd(r0,r1)=gcd(r0-r1,r1)其中假设r0>r1，并且两个数均是正整数。证明：gcd(r0,r1)=g，由于g可以同时被r0、r1整除，则可以记作r0=g·x、r1=g·y，其中x>y，并且x和y互为素数。所以得到：gcd(r0,r1)=gcd(r0-r1,r1)=gcd(g·(x-y),g·y)=g=gcd(ri,0)。**
**例：r0=973,r1=301,gcd的计算方式为：**
![例题1](images/813468_58VGT48ENBUDVK3.png)

 

**欧几里得算法代码**

```
//输入两个正整数r0>r1，输出计算结果
int gcd(int r0, int r1)
{
    int r=0;
    while(r1 != 0)
    {
        r = r0 % r1;
        r0 = r1;
        r1 = r;
    }
    return r0;
}
```

## 扩展欧几里得算法：

　　**扩展欧几里得算法主要的应用不是为例计算最大公因子，它的在密码学中主要的作用是为了计算乘法逆元，乘法逆元在公钥密码学中占有着举足轻重的地位。当然，除了扩展欧几里得算法(EEA)除了可以计算gcd，它还可以计算以下形式的线性组合：**
![公式7](images/813468_7HZKHAE3B768VZH.png)
　　**其中s和t都表示整型系数。关于如何计算这两个系数的推到过程这里就不介绍了，我们只给出最后的公式结论:**
![公式8](images/813468_Z6VD8PAP2EJBJ3H.png)
　　**介绍完这些公式，我们来看看乘法的逆元是怎么计算的吧。假设我们要计算r1 mod r0的逆元，其中r1 < r0。我们前面提到过乘法的逆元计算公式为a\*b=1 mod p，b就是a mod p的乘法逆元，也就是gcd(p, a)=1的情况。下才存在乘法逆元。则s·r0+t·r1=1=gcd(r0,r1)，将这个等式执行模r0计算可得：**
![公式9](images/813468_RD73HP2FEVA8N5A.png)
**例题：计算12 mod 67，12的逆元，即gcd(67,12)=1**
![例题2](images/813468_4GWA95MP2KQEQR9.png)
**注：通常情况下不需要计算系数S,而且实际中一般也用不上它，另外结果t可能是一个负数，这种情况下就必须把t加是r0让人的结果为正，因为t=(t+r0) mod r0。**

 

**扩展欧几里得算法代码：**

```
int EEA(int r0, int r1)
{
    int mod = r0;
    int r = 0;
    int t0 = 0;
    int t1 = 1;
    int t = t1;
    int q = 0;
 
    //0不存在乘法逆元
    if (r1 == 0)
    {
        return 0;
    }
 
    while (r1 != 1)
    {
        q = r0 / r1;
 
        r = r0 - q * r1;
 
        t = t0 - q * t1;
 
        r0 = r1;
        r1 = r;
        t0 = t1;
        t1 = t;
    }
 
    //结果为负数
    if (t < 0)
    {
        t = t + mod;
    }
 
    return t;
}
```

　　**现在相关知识已经学完了，开始进入重点。如果要想计算伽罗瓦域内乘法的逆元，函数的输入r0就是GF(2^8)的不可约多项式p(x)，r1就是域元素a(x)，然后通过EEA计算多项式t(x)得到a(x)的乘法逆元。只不过在上方给出的EEA代码略有不同，因为在伽罗瓦域中多项式都是在GF(2)上进行加减运算的，也就是说上面的加号和减号都要换成异或运算符，同时乘法和除法也有要进行适当的调整，转变成多项式乘法和除法。否则结果会出现偏差。**

 

**伽罗瓦域的扩展欧几里得算法：**

```
//获取最高位
int GetHighestPosition(unsigned short Number)
{
    int i = 0;
    while (Number)
    {
        i++;
        Number = Number >> 1;
    }
    return i;
}
 
 
//GF(2^8)的多项式除法
unsigned char Division(unsigned short Num_L, unsigned short Num_R, unsigned short *Remainder)
{
    unsigned short r0 = 0;
    unsigned char  q = 0;
    int bitCount = 0;
 
    r0 = Num_L;
 
    bitCount = GetHighestPosition(r0) - GetHighestPosition(Num_R);
    while (bitCount >= 0)
    {
        q = q | (1 << bitCount);
        r0 = r0 ^ (Num_R << bitCount);
        bitCount = GetHighestPosition(r0) - GetHighestPosition(Num_R);
    }
    *Remainder = r0;
    return q;
}
 
 
 
//GF(2^8)多项式乘法
short Multiplication(unsigned char Num_L, unsigned char Num_R)
{
    //定义变量
    unsigned short Result = 0;      //伽罗瓦域内乘法计算的结果
 
    for (int i = 0; i < 8; i++)
    {
        Result ^= ((Num_L >> i) & 0x01) * (Num_R << i);
    }
 
    return Result;
}
 
 
int EEA_V2(int r0, int r1)
{
    int mod = r0;
    int r = 0;
    int t0 = 0;
    int t1 = 1;
    int t = t1;
    int q = 0;
 
    if (r1 == 0)
    {
        return 0;
    }
 
    while (r1 != 1)
    {
        //q = r0 / r1;
        q = Division(r0, r1, &r);
 
        r = r0 ^ Multiplication(q, r1);
 
        t = t0 ^ Multiplication(q, t1);
 
        r0 = r1;
        r1 = r;
        t0 = t1;
        t1 = t;
    }
 
    if (t < 0)
    {
        t = t ^ mod;
    }
 
    return t;
}
```

# 第七节：生成S盒的过程：

　　**我们以前介绍过DES算法，那里面也有一个S盒，我们没有介绍过它是怎么形成的是因为DES的S盒是一种特殊的随即表。而AES中的S盒则不同，这个S盒具有非常强的代数结构，它是经过两个步骤计算而来的：**
![S盒流程](images/813468_ZXAJF5TSEGRV3HY.png)

 

　　**我们已经了解的逆元的计算过程，接下来只剩下了仿射映射过程了。**

## S盒的仿射映射

　　**仿射映射这个名词听起来有点高深莫测的感觉，不过在我的理解上，它就是一个计算过程。S盒的仿射映射也比较简单，主要就是运用到了矩阵乘法，不过这个矩阵是Bit矩阵。先上一下计算方法：**
![S盒仿射映射](images/813468_H657C48FAJXKJC5.png)

 

　　**注意仿射映射所有的计算都是基于GF(2)上的。我们从计算过程上发现，输入数据A的逆元B在仿射映射中被展开成了一个8·1的矩阵，最上方是LSB，然后左乘了一个8·8的Bit矩阵，后加上了0x63展开的8·1矩阵，由于是基于GF(2)的，所以需要进行mod 2操作，最终的结果才是输出数据C。可能有些同学还是看不懂这张图，那我们就以输入数据为A=0x7为例，手动计算一下这个结果C:**

 

**1、计算乘法逆元：**
将以下两个参数带入EEA_V2()函数可以得到A的逆元:

 

![公式10](images/813468_BU4GYQXNYJ82CNY.png)

 

**最终结果为：B=EEA(A)=0xD1**

 

**2、仿射映射(重点):**

 

我们将上述结果B拆成Bit带入第二个矩阵得：
![公式11](images/813468_SNHBUX4CN8FUASP.png)

 

计算输出值C:

```
c[0] = (1*1) ^ (0*0) ^ (0*0) ^ (0*0) ^ (1*1) ^ (1*0) ^ (1*1) ^ (1*1) ^ 1 = 1
c[1] = (1*1) ^ (1*0) ^ (0*0) ^ (0*0) ^ (0*1) ^ (1*0) ^ (1*1) ^ (1*1) ^ 1 = 0
c[2] = (1*1) ^ (1*0) ^ (1*0) ^ (0*0) ^ (0*1) ^ (0*0) ^ (1*1) ^ (1*1) ^ 0 = 1
c[3] = (1*1) ^ (1*0) ^ (1*0) ^ (1*0) ^ (0*1) ^ (0*0) ^ (0*1) ^ (1*1) ^ 0 = 0
c[4] = (1*1) ^ (1*0) ^ (1*0) ^ (1*0) ^ (1*1) ^ (0*0) ^ (0*1) ^ (0*1) ^ 0 = 0
c[5] = (0*1) ^ (1*0) ^ (1*0) ^ (1*0) ^ (1*1) ^ (1*0) ^ (0*1) ^ (0*1) ^ 1 = 0
c[6] = (0*1) ^ (0*0) ^ (1*0) ^ (1*0) ^ (1*1) ^ (1*0) ^ (1*1) ^ (0*1) ^ 1 = 1
c[7] = (0*1) ^ (0*0) ^ (0*0) ^ (1*0) ^ (1*1) ^ (1*0) ^ (1*1) ^ (1*1) ^ 0 = 1
```

**最后得到的仿射映射的结果为：C=0xC5**
查看S盒，验证结果正确！

 

**仿射映射代码：**

```
unsigned char ByteImage(int imput)
{
    unsigned char Result = 0;
 
    for (int i = 0; i < 8; i++)
    {
        Result ^= (((imput >> i) & 1) ^ ((imput >> ((i + 4) % 8)) & 1) ^ ((imput >> ((i + 5) % 8)) & 1) ^ ((imput >> ((i + 6) % 8)) & 1) ^ ((imput >> ((i + 7) % 8)) & 1)) << i;
    }
 
    Result = Result ^ 0x63;
 
    return Result;
}
```

# 第八节：生成逆S盒的过程：

![逆S盒流程](images/813468_CPZKVANJS58HPTD.png)
　　**可以发现逆S盒是先进行逆仿射映射，然后才计算乘法逆元的，这也是与S盒生成的不同之处，而逆仿射映射与仿射映射的结构是相同的，只不过8·8Bit矩阵的数值不同，最后异或的那个数字不是0x63而是0xA0。**
![逆仿射映射](images/813468_TFAHZJ4Q2EBW9AR.png)

 

笔者在这里偷个懒，就不提供逆仿射映射的代码盒过程了，大家有兴趣可以自己实现下^_^!





files：

![image-20210222141242419](images/image-20210222141242419.png)





![image-20210222141602078](images/image-20210222141602078.png)

![image-20210222141625017](images/image-20210222141625017.png)

![image-20210222141829963](images/image-20210222141829963.png)