/**
 * Base64.cpp
 * @Author  Tu Yongce <tuyongce (at) 163 (dot) com>
 * @Date    Aug. 4 2007 
 * @Last    Aug. 20 2007  
 * @version 0.2
 */

#include "Base64.h"

const char Base64::sm_base64digits[65] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char Base64::sm_base64val[128] = {
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
     52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
     41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};



/**
 * binary bytes to base 64 string (NUL-terminated).
 * Every three bytes inputted needs four bytes to output. 
 * A byte more needs to hold NUL terminator.
 * @param in: Input buffer.
 * @param out: Output buffer.
 * @param inLength: Number of bytes in input buffer.
 */
void Base64::binToBase64(const unsigned char *in, char *out, size_t inLength)
{
    for ( ; inLength >= 3; inLength -= 3, in += 3) {
        *out++ = sm_base64digits[in[0] >> 2];
        *out++ = sm_base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
        *out++ = sm_base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
        *out++ = sm_base64digits[in[2] & 0x3f];
    }
    if (inLength > 0) {
        unsigned char fragment;
        
        *out++ = sm_base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;
        if (inLength > 1)
            fragment |= in[1] >> 4;
        *out++ = sm_base64digits[fragment];
        *out++ = (inLength < 2) ? '=' : sm_base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }
    *out = '\0';
}

/**
 * base64 string to binary bytes.
 * @param in: Input buffer.
 * @param out: Output buffer.
 * @param maxLen: Size of output buffer. Set to zero to ignore.
 * @return: Number of bytes actually converted.
 */
int Base64::base64ToBin(const char *in, unsigned char *out, size_t maxLen)
{
    size_t len = 0;
    unsigned char digit1, digit2, digit3, digit4;
    
    if (in[0] == '+' && in[1] == ' ')
        in += 2;
    if (in[0] == '\r')
        return 0;
    
    while (*in && *in != '\r' && digit4 != '=') {
        digit1 = in[0];
        if (decode64(digit1) == BAD)
            return -1;
        digit2 = in[1];
        if (decode64(digit2) == BAD)
            return -1;
        digit3 = in[2];
        if (digit3 != '=' && decode64(digit3) == BAD)
            return -1; 
        digit4 = in[3];
        if (digit4 != '=' && decode64(digit4) == BAD)
            return -1;
        in += 4;
        
        ++len;
        if (maxLen && len > maxLen)
            return -1;
            
        *(out++) = (decode64(digit1) << 2) | (decode64(digit2) >> 4);
        if (digit3 != '=') {
            ++len;
            if (maxLen && len > maxLen)
                return -1;
            *(out++) = ((decode64(digit2) << 4) & 0xf0) | (decode64(digit3) >> 2);
            if (digit4 != '=') {
                ++len;
                if (maxLen && len > maxLen)
                    return -1;
                *(out++) = ((decode64(digit3) << 6) & 0xc0) | decode64(digit4);
            } // if
        } // if
    } // while
    
    return len;
}
