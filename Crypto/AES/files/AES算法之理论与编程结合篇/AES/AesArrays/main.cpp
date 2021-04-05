/**
 * main.cpp
 * @Author  Tu Yongce <tuyongce (at) 163 (dot) com>
 * @Date    Aug. 4 2007 
 * @Last    Aug. 20 2007  
 * @version 0.2
 */

#include <iostream>
#include <fstream>
#include <cassert>
#include "AesArrays.h"

using namespace std;

int main()
{
    ofstream of("tables.txt");
    if (of.fail())
        return -1;

	unsigned char rc[52];
    generateRC(rc, 52);
    dumpData(of, "rc", rc, 52, 10);

    unsigned char inv[256];
    getMulInvTable(inv, 256);
    dumpData(of, "inv", inv, 256, 16);    

    unsigned char sbox[256];
    unsigned char isbox[256];
    getSbox(sbox, isbox, 256);
    dumpData(of, "sbox", sbox, 256, 16);
    dumpData(of, "isbox", isbox, 256, 16);

    u32_t mc1[256], mc2[256], mc3[256], mc4[256];
    getMC(mc1, mc2, mc3, mc4, 256);
    dumpData(of, "mc1", mc1, 256, 8);
    dumpData(of, "mc2", mc2, 256, 8);
    dumpData(of, "mc3", mc3, 256, 8);
    dumpData(of, "mc4", mc4, 256, 8);

    u32_t imc1[256], imc2[256], imc3[256], imc4[256];
    getIMC(imc1, imc2, imc3, imc4, 256);
    dumpData(of, "imc1", imc1, 256, 8);
    dumpData(of, "imc2", imc2, 256, 8);
    dumpData(of, "imc3", imc3, 256, 8);
    dumpData(of, "imc4", imc4, 256, 8);

    // test & usage
    assert(invMixColumn(mixColumn(0x876e46a6)) == 0x876e46a6);
    assert((mc1[0x87] ^ mc2[0x6e] ^ mc3[0x46] ^ mc4[0xa6]) == mixColumn(0x876e46a6));
    assert((imc1[0x87] ^ imc2[0x6e] ^ imc3[0x46] ^ imc4[0xa6]) == invMixColumn(0x876e46a6));

    u32_t te1[256], te2[256], te3[256], te4[256];
    getTE(te1, te2, te3, te4, sbox, 256);
    dumpData(of, "te1", te1, 256, 8);
    dumpData(of, "te2", te2, 256, 8);
    dumpData(of, "te3", te3, 256, 8);
    dumpData(of, "te4", te4, 256, 8);

    u32_t td1[256], td2[256], td3[256], td4[256];
    getTD(td1, td2, td3, td4, isbox, 256);
    dumpData(of, "td1", td1, 256, 8);
    dumpData(of, "td2", td2, 256, 8);
    dumpData(of, "td3", td3, 256, 8);
    dumpData(of, "td4", td4, 256, 8);

    return 0;
}
