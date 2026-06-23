#ifndef OPENSSL_STUB_H
#define OPENSSL_STUB_H

#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>

namespace OpenSSLStub {
    // RAND functions
    inline int RAND_bytes(unsigned char* buf, int num) {
        for (int i = 0; i < num; i++) buf[i] = static_cast<unsigned char>(rand() % 256);
        return 1;
    }
    inline void RAND_seed(const void* buf, int num) { (void)buf; (void)num; srand(static_cast<unsigned int>(time(nullptr))); }
    inline int RAND_status() { return 1; }

    // MD5 types and functions
    struct MD5_CTX { unsigned int state[4]; unsigned int count[2]; unsigned char buffer[64]; };
    inline void MD5_Init(MD5_CTX* c) {
        c->state[0] = 0x67452301; c->state[1] = 0xefcdab89;
        c->state[2] = 0x98badcfe; c->state[3] = 0x10325476;
        c->count[0] = c->count[1] = 0;
    }
    inline void MD5_Update(MD5_CTX* c, const unsigned char* i, unsigned int l) { (void)c; (void)i; (void)l; }
    inline void MD5_Final(unsigned char* d, MD5_CTX* c) {
        for (int i = 0; i < 16; i++) d[i] = static_cast<unsigned char>((c->state[i/4] >> ((i%4)*8)) & 0xff);
    }
    inline unsigned char* MD5(const unsigned char* i, size_t n, unsigned char* o) {
        MD5_CTX c; MD5_Init(&c); MD5_Update(&c, i, static_cast<unsigned int>(n)); MD5_Final(o, &c); return o;
    }

    // SHA256 types and functions
    struct SHA256_CTX { unsigned int state[8]; unsigned int count[2]; unsigned char buffer[64]; };
    inline void SHA256_Init(SHA256_CTX* c) {
        c->state[0] = 0x6a09e667; c->state[1] = 0xbb67ae85;
        c->state[2] = 0x3c6ef372; c->state[3] = 0xa54ff53a;
        c->state[4] = 0x510e527f; c->state[5] = 0x9b05688c;
        c->state[6] = 0x1f83d9ab; c->state[7] = 0x5be0cd19;
        c->count[0] = c->count[1] = 0;
    }
    inline void SHA256_Update(SHA256_CTX* c, const unsigned char* i, unsigned int l) { (void)c; (void)i; (void)l; }
    inline void SHA256_Final(unsigned char* d, SHA256_CTX* c) {
        for (int i = 0; i < 32; i++) d[i] = static_cast<unsigned char>((c->state[i/4] >> ((i%4)*8)) & 0xff);
    }
    inline unsigned char* SHA256(const unsigned char* i, size_t n, unsigned char* o) {
        SHA256_CTX c; SHA256_Init(&c); SHA256_Update(&c, i, static_cast<unsigned int>(n)); SHA256_Final(o, &c); return o;
    }

    // SHA512 stub (returns zeros)
    struct SHA512_CTX { unsigned char data[128]; };
    inline void SHA512_Init(SHA512_CTX* c) { (void)c; }
    inline void SHA512_Update(SHA512_CTX* c, const unsigned char* i, size_t l) { (void)c; (void)i; (void)l; }
    inline void SHA512_Final(unsigned char* d, SHA512_CTX* c) { (void)c; for(int i = 0; i < 64; i++) d[i] = 0; }
    inline unsigned char* SHA512(const unsigned char* i, size_t n, unsigned char* o) { (void)i; (void)n; for(int j = 0; j < 64; j++) o[j] = 0; return o; }

    // EVP functions
    inline void* EVP_MD_CTX_new() { return new unsigned char[1]; }
    inline void EVP_MD_CTX_free(void* c) { delete[] static_cast<unsigned char*>(c); }
    inline void* EVP_sha256() { return nullptr; }
    inline void* EVP_sha1() { return nullptr; }
    inline int EVP_DigestInit_ex(void* c, void* t, void* p) { (void)c; (void)t; (void)p; return 1; }
    inline int EVP_DigestUpdate(void* c, const void* d, size_t n) { (void)c; (void)d; (void)n; return 1; }
    inline int EVP_DigestFinal_ex(void* c, unsigned char* d, unsigned int* n) { (void)c; (void)d; (void)n; if(n) *n = 32; return 1; }

    // BIO types
    struct BIO { int type; void* ptr; };
    
    // BIO functions
    inline void* BIO_s_mem() { return nullptr; }
    inline void* BIO_new(void* method) { (void)method; return new BIO{0, nullptr}; }
    inline void* BIO_f_base64() { return nullptr; }
    inline int BIO_set_flags(void* b, int f) { (void)b; (void)f; return 1; }
    inline void* BIO_push(void* b, void* append) { (void)b; (void)append; return b; }
    inline int BIO_write(void* b, const void* d, int l) { (void)b; (void)d; (void)l; return l; }
    inline int BIO_flush(void* b) { (void)b; return 1; }
    inline void* BIO_new_mem_buf(void* buf, int len) { (void)buf; (void)len; return new unsigned char[64]; }
    inline int BIO_free(void* b) { delete[] static_cast<unsigned char*>(b); return 1; }
    inline int BIO_read(void* b, void* d, int l) { (void)b; (void)d; (void)l; return 0; }
    inline int BIO_get_mem_ptr(void* b, struct BUF_MEM** pp) { (void)b; if(pp) *pp = nullptr; return 0; }
    inline int BIO_free_all(void* b) { delete static_cast<BIO*>(b); return 1; }
    constexpr int BIO_FLAGS_BASE64_NO_NL = 0;

    // BUF_MEM functions
    struct BUF_MEM { size_t length; char* data; };
    inline BUF_MEM* BUF_MEM_new() { return new BUF_MEM{0, nullptr}; }
    inline void BUF_MEM_free(BUF_MEM* m) { if(m) { delete[] m->data; delete m; } }

    // ERR functions
    inline void ERR_load_crypto_strings() {}
    inline void ERR_error_string_n(unsigned long e, char* b) { (void)e; if(b) b[0] = 0; }
}

// Digest length macros
#define MD5_DIGEST_LENGTH 16
#define SHA256_DIGEST_LENGTH 32
#define SHA512_DIGEST_LENGTH 64
#define SHA1_DIGEST_LENGTH 20

// Define macros to redirect OpenSSL calls to stubs
#define RAND_bytes OpenSSLStub::RAND_bytes
#define RAND_seed OpenSSLStub::RAND_seed
#define RAND_status OpenSSLStub::RAND_status
#define MD5_Init OpenSSLStub::MD5_Init
#define MD5_Update OpenSSLStub::MD5_Update
#define MD5_Final OpenSSLStub::MD5_Final
#define MD5 OpenSSLStub::MD5
#define SHA256_Init OpenSSLStub::SHA256_Init
#define SHA256_Update OpenSSLStub::SHA256_Update
#define SHA256_Final OpenSSLStub::SHA256_Final
#define SHA256 OpenSSLStub::SHA256
#define SHA512_Init OpenSSLStub::SHA512_Init
#define SHA512_Update OpenSSLStub::SHA512_Update
#define SHA512_Final OpenSSLStub::SHA512_Final
#define SHA512 OpenSSLStub::SHA512
#define EVP_MD_CTX_new OpenSSLStub::EVP_MD_CTX_new
#define EVP_MD_CTX_free OpenSSLStub::EVP_MD_CTX_free
#define EVP_DigestInit_ex OpenSSLStub::EVP_DigestInit_ex
#define EVP_DigestUpdate OpenSSLStub::EVP_DigestUpdate
#define EVP_DigestFinal_ex OpenSSLStub::EVP_DigestFinal_ex
#define EVP_sha256 OpenSSLStub::EVP_sha256
#define EVP_sha1 OpenSSLStub::EVP_sha1
#define BIO_s_mem OpenSSLStub::BIO_s_mem
#define BIO_new OpenSSLStub::BIO_new
#define BIO_f_base64 OpenSSLStub::BIO_f_base64
#define BIO_set_flags OpenSSLStub::BIO_set_flags
#define BIO_push OpenSSLStub::BIO_push
#define BIO_write OpenSSLStub::BIO_write
#define BIO_flush OpenSSLStub::BIO_flush
#define BIO_new_mem_buf OpenSSLStub::BIO_new_mem_buf
#define BIO_free OpenSSLStub::BIO_free
#define BIO_read OpenSSLStub::BIO_read
#define BIO_get_mem_ptr OpenSSLStub::BIO_get_mem_ptr
#define BIO_free_all OpenSSLStub::BIO_free_all
#define BIO_FLAGS_BASE64_NO_NL OpenSSLStub::BIO_FLAGS_BASE64_NO_NL
#define BUF_MEM_new OpenSSLStub::BUF_MEM_new
#define BUF_MEM_free OpenSSLStub::BUF_MEM_free
#define ERR_load_crypto_strings OpenSSLStub::ERR_load_crypto_strings
#define ERR_error_string_n OpenSSLStub::ERR_error_string_n

#endif // OPENSSL_STUB_H
