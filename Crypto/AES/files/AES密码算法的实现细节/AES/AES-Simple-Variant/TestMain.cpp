/**
 * TestMain.cpp
 * @Author  Tu Yongce <tuyongce (at) 163 (dot) com>
 * @Date    Aug. 4 2007 
 * @Last    Aug. 20 2007  
 * @version 0.2
 */

#include <iostream>
#include <cassert>
#include "AesCipher.h"
#include "Base64.h"

using namespace std;

void test1();
void test2();
void test3();

int main()
{
    test1();
    test2();
    test3();

    return 0;
}

void test1()
{
    // Test1: encrypt/decrypt string with ECB, CBC, CFB, OFB working modes.
    cout << "=======================================" << endl;
    cout << ">> Test1: encrypt/decrypt string with ECB, CBC, CFB, OFB working modes." << endl;

    const char in[] = "abcdefghijklmnopqrstuvwxyz012345abcdefghijklmnop";
    cout << "Plaintext: \n" << in << endl;
    cout << "Length: " << strlen(in) << endl;

    char out[100];
    char out2[100];
    char out3[100];

    AesCipher crypto;
    crypto.makeRoundKey("this is a key.ok");

    // ECB mode
    cout << "\nECB Mode: " << endl;
    crypto.encrypt(in, out, 48, AesCipher::ECB);

    Base64::binToBase64((unsigned char*)out, out3, 48);
    cout << "base64: " << out3 << endl;
    Base64::base64ToBin(out3, (unsigned char*)out, Base64::calculateBase64Len(48));

    crypto.decrypt(out, out2, 48, AesCipher::ECB);
    out2[48] = '\0';
    cout << out2 << endl;
    assert(strcmp(in, out2) == 0);

    // CBC mode, default
    crypto.setIV("chain.0123456789");

    cout << "\nCBC Mode: " << endl;
    crypto.encrypt(in, out, 48);
    
    Base64::binToBase64((unsigned char*)out, out3, 48);
    cout << "base64: " << out3 << endl;
    Base64::base64ToBin(out3, (unsigned char*)out, Base64::calculateBase64Len(48));

    crypto.decrypt(out, out2, 48);
    out2[48] = '\0';
    cout << out2 << endl;
    assert(strcmp(in, out2) == 0);

    // CFB mode
    cout << "\nCFB Mode: " << endl;
    crypto.encrypt(in, out, 48, AesCipher::CFB);
    
    Base64::binToBase64((unsigned char*)out, out3, 48);
    cout << "base64: " << out3 << endl;
    Base64::base64ToBin(out3, (unsigned char*)out, Base64::calculateBase64Len(48));

    crypto.decrypt(out, out2, 48, AesCipher::CFB);
    out2[48] = '\0';
    cout << out2 << endl;
    assert(strcmp(in, out2) == 0);

    // OFB mode
    cout << "\nOFB Mode: " << endl;
    crypto.encrypt(in, out, 48, AesCipher::OFB);
    
    Base64::binToBase64((unsigned char*)out, out3, 48);
    cout << "base64: " << out3 << endl;
    Base64::base64ToBin(out3, (unsigned char*)out, Base64::calculateBase64Len(48));

    //crypto.decrypt(out, out2, 32, AesCipher::OFB);
    // at this mode, decryption is the same with encryption
    crypto.encrypt(out, out2, 48, AesCipher::OFB);
    out2[48] = '\0';
    cout << out2 << endl;
    assert(strcmp(in, out2) == 0);
}

void test2()
{
    // Test2: encrypt/decrypt string with arbitrary lenght and CTR working mode.
    cout << "\n=======================================" << endl;
    cout << ">> Test2: encrypt/decrypt string with arbitrary lenght and CTR working mode." << endl;

    const char in[] = "encrypt/decrypt string with arbitrary lenght and CTR working mode.";
    cout << "Plaintext: " << in << endl;
    cout << "Length: " << strlen(in) << endl;

    char counter[16];
    memset(counter, 0, 16);
    counter[5] = -113;

    AesCipher crypto;
    crypto.makeRoundKey("this is a key.ok");   
    crypto.setCounter(counter);

    int outLen = crypto.calculateCipherLen(strlen(in));
    char *out = new char[outLen];
    if (out == 0)
        return ;
    crypto.encryptString(in, out, strlen(in), AesCipher::CTR);

    char *buf = new char[Base64::calculateBase64Len(outLen)];
    Base64::binToBase64((unsigned char*)out, buf, outLen);
    cout << "Ciphertext with base64: " <<  buf << endl;
    Base64::base64ToBin(buf, (unsigned char*)out, outLen);
    delete [] buf;

    char *out2 = new char[outLen + 1];
    if (out2 == 0)
        return ;
    int len = crypto.decryptString(out, out2, outLen, AesCipher::CTR);
    out2[len] = '\0';
    cout << "Plaintext restored: " << out2 << endl;
    assert(strcmp(in, out2) == 0);

    delete [] out;
    delete [] out2;
}

void test3()
{
    // Test3: Thc case when in and out pointers point to the same address.
    cout << "\n=======================================" << endl;
    cout << ">> Test3: Thc case when in and out pointers point to the same address." << endl;

    char in[100] = "Thc case when in and out pointers point to the same address.";
    char in2[100] = "Thc case when in and out pointers point to the same address.";
    cout << "Plaintext: " << in << endl;
    cout << "Length: " << strlen(in) << endl;

    AesCipher crypto;
    crypto.makeRoundKey("this is a key.ok");   
    crypto.setIV("chain.0123456789");

    int outLen = crypto.calculateCipherLen(strlen(in));
    char *buf;
    int len;

    // input and output pointers are all "in"!

    // ECB mode
    cout << "\nECB Mode: " << endl;
    crypto.encryptString(in, in, strlen(in), AesCipher::ECB);

    buf = new char[Base64::calculateBase64Len(outLen)];
    Base64::binToBase64((unsigned char*)in, buf, outLen);
    cout << "Ciphertext with base64: " <<  buf << endl;
    Base64::base64ToBin(buf, (unsigned char*)in, outLen);
    delete [] buf;

    len = crypto.decryptString(in, in, outLen, AesCipher::ECB);
    in[len] = '\0';
    cout << "Plaintext restored: " << in << endl;
    assert(strcmp(in, in2) == 0);

    // CBC mode
    cout << "\nCBC Mode: " << endl;
    crypto.encryptString(in, in, strlen(in), AesCipher::CBC);

    buf = new char[Base64::calculateBase64Len(outLen)];
    Base64::binToBase64((unsigned char*)in, buf, outLen);
    cout << "Ciphertext with base64: " <<  buf << endl;
    Base64::base64ToBin(buf, (unsigned char*)in, outLen);
    delete [] buf;

    len = crypto.decryptString(in, in, outLen, AesCipher::CBC);
    in[len] = '\0';
    cout << "Plaintext restored: " << in << endl;
    assert(strcmp(in, in2) == 0);

    // CFB mode
    cout << "\nCFB Mode: " << endl;
    char counter[16];
    memset(counter, 0, 16);
    counter[13] = -25;
    counter[5] = 117;  
    crypto.setCounter(counter);
    crypto.encryptString(in, in, strlen(in), AesCipher::CFB);

    buf = new char[Base64::calculateBase64Len(outLen)];
    Base64::binToBase64((unsigned char*)in, buf, outLen);
    cout << "Ciphertext with base64: " <<  buf << endl;
    Base64::base64ToBin(buf, (unsigned char*)in, outLen);
    delete [] buf;

    len = crypto.decryptString(in, in, outLen, AesCipher::CFB);
    in[len] = '\0';
    cout << "Plaintext restored: " << in << endl;
    assert(strcmp(in, in2) == 0);

    // OFB mode
    cout << "\nOFB Mode: " << endl;
    crypto.encryptString(in, in, strlen(in), AesCipher::OFB);

    buf = new char[Base64::calculateBase64Len(outLen)];
    Base64::binToBase64((unsigned char*)in, buf, outLen);
    cout << "Ciphertext with base64: " <<  buf << endl;
    Base64::base64ToBin(buf, (unsigned char*)in, outLen);
    delete [] buf;

    len = crypto.decryptString(in, in, outLen, AesCipher::OFB);
    in[len] = '\0';
    cout << "Plaintext restored: " << in << endl;
    assert(strcmp(in, in2) == 0);

    // CTR mode
    cout << "\nCTR Mode: " << endl;
    crypto.encryptString(in, in, strlen(in), AesCipher::CTR);

    buf = new char[Base64::calculateBase64Len(outLen)];
    Base64::binToBase64((unsigned char*)in, buf, outLen);
    cout << "Ciphertext with base64: " <<  buf << endl;
    Base64::base64ToBin(buf, (unsigned char*)in, outLen);
    delete [] buf;

    len = crypto.decryptString(in, in, outLen, AesCipher::CTR);
    in[len] = '\0';
    cout << "Plaintext restored: " << in << endl;
    assert(strcmp(in, in2) == 0);
}