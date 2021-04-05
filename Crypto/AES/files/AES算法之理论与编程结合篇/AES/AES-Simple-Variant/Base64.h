/**
 * Base64.h
 * @Author  Tu Yongce <tuyongce (at) 163 (dot) com>
 * @Date    Aug. 4 2007 
 * @Last    Aug. 20 2007  
 * @version 0.2
 */

#ifndef __BASE64_H__
#define __BASE64_H__

/**
 * The Base64 class is a utility class for coding/decoding data with base64.
 * Its all members are static member.
 */

class Base64
{
public:
    /**
     * Get number of bytes of base64 string corresponding to 
     * binary string with length @param len.
     * @param len: Number of bytes of binary string.
     * @return: Number of bytes of base64 string.
     */
    static int calculateBase64Len(size_t len) {
        return (len / 3 + (len % 3 ? 1 : 0)) * 4 + 1; // one more byte for '\0'
    }

    /**
     * Binary bytes to base 64 string (NUL-terminated).
     * Every three bytes inputted needs four bytes to output. 
     * A byte more needs to hold NUL terminator.
     * @param in: Input buffer.
     * @param out: Output buffer.
     * @param inLength: Number of bytes in input buffer.
     */
    static void binToBase64(const unsigned char *in, char *out, size_t inLength);

    /**
     * Get number of bytes of binary string corresponding to 
     * base64 string with length @param len.
     * @param len: Number of bytes of base64 string.
     * @return: Number of bytes of binary string.
     */
    static int calculateBinLen(size_t len) {
        return len / 4 * 3; 
    }

    /**
     * base64 string to binary bytes.
     * @param in: Input buffer.
     * @param out: Output buffer.
     * @param maxLen: Size of output buffer. Set to zero to ignore.
     * @return: Number of bytes actually converted.
     */
    static int base64ToBin(const char *in, unsigned char *out, size_t maxLen);

private:
    static char decode64(unsigned char ch) {
        return ch < 128 ? sm_base64val[ch] : BAD;
    }

private:
    enum {BAD = -1};
    static const char sm_base64digits[65];
    static const char sm_base64val[128];
};

#endif // __BASE64_H__
