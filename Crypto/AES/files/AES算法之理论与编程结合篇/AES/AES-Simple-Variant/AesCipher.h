/**
 * AesCipher.h
 * @Author  Tu Yongce <tuyongce (at) 163 (dot) com>
 * @Date    Aug. 4 2007 
 * @Last    Aug. 20 2007  
 * @version 0.2
 */

#ifndef __AES_CIPHER_H__
#define __AES_CIPHER_H__

#include <cstring>
#include <climits>
#include <stdexcept>

/**
 * The AesCipher class is a utility class for encryption/decryption of data.
 * It's an encapsulation for the AES (Advanced Encryption Standard) cipher 
 * (also called Rijndael cipher).
 */

/**
 * You maybe will define the macro __AES_IMC_SPEEDUP__, this will speed up
 * the process making round key (AesCipher::makeRoundKey), but 4 KB memory
 * more will be used. By default, we disable this speedup.
 * If you indeed want to enable this speedup, just uncomment the following
 * macro definition.
 */

//#define __AES_IMC_SPEEDUP__

class AesCipher
{
// public types and data
public: 
    /**
     * AES's working-mode (also all other block ciphers' working-mode):
     *   ECB: Electronic Codebook mode
     *   CBC: Cipher Block Chaining mode
     *   CFB: Cipher Feedback mode
     *   OFB: Output Feedback mode
     *   CTR: Counter mode
     */
    enum { ECB = 0, CBC = 1, CFB = 2, OFB = 3, CTR = 4 };
    enum { DEFAULT_KEY_LENGTH = 16 };      // default key length
    enum { BLOCK_SIZE = 16 };              // block size

// public member functions
public:
    AesCipher() : m_bKey(false), m_bIv(false), m_bCounter(false) { }

    /**
     * Expand a user key into the round key.
     * @param key: The 128/192/256-bit user-key to use. It cannot be null pointer.
     * @param keylength: The key length of this AES (16, 24 or 32 bytes, 16 bytes by default).
     */
    void makeRoundKey(const char *key, int keyLength = DEFAULT_KEY_LENGTH);

    /**
     * Set the initial vector (IV) for chain working mode (CBC, CFB and OFB).
     * @param iv: Initial vector (IV) for CBC, CFB and OFB.
     */
    void setIV(const char *iv) {
        if (iv != 0) {
            m_bIv = true;
            memcpy(m_iv, iv, BLOCK_SIZE);
        } else
            m_bIv = false;
    }

    /**
     * Set the initial counter for CTR working mode.
     * @counter: Initial counter for CTR (128 bits or 16 bytes).
     * @note: The low address bytes corresponds to the counter's low bits.
     */
    void setCounter(const char *counter) {
        if (counter != 0) {
            m_bCounter = true;
            memcpy(m_counter, counter, BLOCK_SIZE);
        } else
            m_bCounter = false;
    }

    /**
     * Encrypt multiple blocks of plaintext.
     * @param in: The plaintext to encrypt.
     * @param result: The output ciphertext.
     * @param num: Number of bytes to encrypt, must be a multiple of the blocksize.
     * @param mode: Mode to use, CBC by default.
     */
    void encrypt(const char *in, char *result, size_t num, int mode = CBC);
    
    /**
     * Decrypt multiple blocks of ciphertext.
     * @param in: The ciphertext to decrypt.
     * @param result: The output plaintext.
     * @param num: Number of bytes to decrypt, must be a multiple of the blocksize.
     * @param mode: Mode to use, CBC by default.
     */
    void decrypt(const char *in, char *result, size_t num, int mode = CBC);

    /**
     * Get number of bytes of ciphertext corresponding to plaintext with length @param plainLen.
     * @param plaineLen: Number of bytes of plaintext.
     * @return: Number of bytes of ciphertext.
     */
    static int calculateCipherLen(size_t len) {
        return (len / BLOCK_SIZE + (len % BLOCK_SIZE ? 1 : 0) + 1) * 16;
    }

    /**
     * Encrypt a plaintext string with arbitrary length.
     * @param in: The plaintext to encrypt.
     * @param result: The output ciphertext.
     * @param num: Number of bytes to encrypt.
     * @param mode: Mode to use, CBC by default.
     * @note: If needed, zero bytes will be appended to the plaintext to make that
     *        the plaintext length will be a multiple of blocksize.
     *        You should first call calculateCipherLen() to get ciphertext length.
     *        An exception runtime_error may be thrown.
     */
    void encryptString(const char *in, char *result, size_t num, int mode = CBC) {
        size_t len = calculateCipherLen(num);
        char *buf = new char[len];
        if (buf == 0) 
            throw std::runtime_error("Allocating memory failed.");       
        // filling with zero
        memset(buf, 0, len);
        memcpy(buf, in, num);
        // The last block holds valid number of bytes in last plaintext block.
        buf[len - BLOCK_SIZE] = num % BLOCK_SIZE ? num % BLOCK_SIZE : BLOCK_SIZE;        
        encrypt(buf, result, len, mode);
        
        delete [] buf;
    }
    
    /**
     * Decrypt a ciphertext string.
     * @param in: The ciphertext to decrypt.
     * @param result: The output plaintext.
     * @param num: Number of bytes to decrypt, must be a multiple of the blocksize.
     * @param mode: Mode to use, CBC by default.
     * @return: Number of bytes of plaintext restored.
     */
    int decryptString(const char *in, char *result, size_t num, int mode = CBC) {
        decrypt(in, result, num, mode);
        return num - 2 * BLOCK_SIZE + result[num - BLOCK_SIZE];
    }

    /**
     * Get Key Length.
     */
    int getKeyLength() {
        if (!m_bKey)
            return 0;
        return m_keyLength;
    }

    /**
     * Get Number of Rounds.
     */
    int getRounds() {
        if (!m_bKey)
            return 0;
        return m_rounds;
    }

// private types and functions
private:
    typedef unsigned __int32 u32_t;
    typedef unsigned char    u8_t;
    
    /**
     * Encrypt exactly one block of plaintext, assuming AES' block size (128-bit).
     * @param in: The plaintext.
     * @param result: The ciphertext generated from a plaintext using the key.
     */
    void encryptBlock(const char *in, char *result);

    /**
     * Decrypt exactly one block of plaintext, assuming AES' block size (128-bit).
     * @param in: The ciphertext.
     * @param result: The plaintext generated from a ciphertext using the key.
     */
    void decryptBlock(const char *in, char *result);

    /**
     * XOR operation.
     */
    static void xorBlock(char *block1, const char *block2) {
        for (int i = 0; i < BLOCK_SIZE; ++i)
            block1[i] ^= block2[i]; 
    }

    /**
     * Increase counter by one.
     * @param counter: The counter for CTR, assuming AES' block size (128 bits).
     * @return: If overflow, then return false; or return true.
     * @note: The low address bytes corresponds to the counter's low bits.
     */
    bool incrCounter(char *counter) {
        unsigned char *bytes = reinterpret_cast<unsigned char*>(counter);
        int i;
        for (i = 0; i < BLOCK_SIZE && bytes[i] == UCHAR_MAX; ++i)
            bytes[i] = 0;  // carry
        if (i < BLOCK_SIZE) {
            bytes[i]++;
            return true;
        } else // overflow
            return false;
    }

private:
    enum { MAX_ROUNDS = 14 };        // maximum number of round
    enum { MAX_KWC = 8 };            // key's maximum number of word
    enum { BWC = 4 };                // block's number of word (BLOCK_SIZE / 4)

    // Substitute Bytes Transformation Array, S-Box
    static const u8_t sm_sbox[256];
    // Inverse Substitute Bytes Transformation Array, Inverse S-Box
    static const u8_t sm_isbox[256];
    // Two-in-one Transform Array: substitute byte & MixColumns transformation
    static const u32_t  sm_te1[256];
    static const u32_t  sm_te2[256];
    static const u32_t  sm_te3[256];
    static const u32_t  sm_te4[256];
    // Two-in-one Transform Array: inverse substitute byte & inverse MixColumns transformation
    static const u32_t  sm_td1[256];
    static const u32_t  sm_td2[256];
    static const u32_t  sm_td3[256];
    static const u32_t  sm_td4[256];
    
#ifdef __AES_IMC_SPEEDUP__
    // Inverse MixColumns Transformation Array
    static const u32_t  sm_imc1[256];          
    static const u32_t  sm_imc2[256];
    static const u32_t  sm_imc3[256];
    static const u32_t  sm_imc4[256];
#else
    // Inverse MixColumns Transformation Function
    unsigned char mul(unsigned char a, unsigned char b);
    u32_t invMixColumn(u32_t w);   
#endif 

    // Round Constant Array
    static const u8_t sm_rcon[52];           

    bool m_bKey;                        // key initialization flag
    u32_t m_ke[MAX_ROUNDS + 1][BWC];    // encryption round key
    u32_t m_kd[MAX_ROUNDS + 1][BWC];    // decryption round key
    int m_keyLength;                    // length of key
    int m_rounds;                       // number of rounds
    // initial vector (IV) block, for chain mode
    bool m_bIv;                         // IV initialization flag
    char m_iv[BLOCK_SIZE];
    // initial counter for CTR mode.
    bool m_bCounter;                    // counter initialization flag
    char m_counter[BLOCK_SIZE];
};

#endif  //__AES_CIPHER_H__

