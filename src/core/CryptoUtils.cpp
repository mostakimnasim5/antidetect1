/**
 * CryptoUtils - Enterprise-Grade Cryptographic Implementation
 */

#include "core/CryptoUtils.hpp"
#include <chrono>
#include <thread>
#include <mutex>
#include <cstring>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <sys/time.h>

// OpenSSL initialization flag
static bool g_opensslInitialized = false;
static std::mutex g_opensslMutex;

namespace VirtualPhonePro {
namespace Crypto {

// ============================================
// INITIALIZATION
// ============================================
void initializeOpenSSL() {
    std::lock_guard<std::mutex> lock(g_opensslMutex);
    if (!g_opensslInitialized) {
        OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS, nullptr);
        g_opensslInitialized = true;
    }
}

void cleanupOpenSSL() {
    std::lock_guard<std::mutex> lock(g_opensslMutex);
    if (g_opensslInitialized) {
        EVP_cleanup();
        ERR_free_strings();
        g_opensslInitialized = false;
    }
}

// ============================================
// SECURE RANDOM GENERATOR
// ============================================
SecureRandomGenerator::SecureRandomGenerator() : m_initialized(false) {
    initializeOpenSSL();
    
    // Seed with system entropy
    std::array<uint8_t, 64> seed;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    seed[0] = static_cast<uint8_t>(tv.tv_sec & 0xFF);
    seed[1] = static_cast<uint8_t>((tv.tv_sec >> 8) & 0xFF);
    seed[2] = static_cast<uint8_t>(tv.tv_usec & 0xFF);
    seed[3] = static_cast<uint8_t>((tv.tv_usec >> 8) & 0xFF);
    
    // Additional system entropy
    std::random_device rd;
    for (size_t i = 4; i < 32; ++i) {
        seed[i] = static_cast<uint8_t>(rd());
    }
    
    RAND_seed(seed.data(), static_cast<int>(seed.size()));
    
    // Verify we have sufficient entropy
    if (RAND_status() == 1) {
        m_initialized = true;
    }
}

SecureRandomGenerator::~SecureRandomGenerator() {
    secureZero(m_seedBuffer.data(), m_seedBuffer.size());
}

std::vector<uint8_t> SecureRandomGenerator::generateBytes(size_t length) {
    std::vector<uint8_t> result(length);
    if (RAND_bytes(result.data(), static_cast<int>(length)) != 1) {
        // Fallback to pseudo-random if CSPRNG fails
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (size_t i = 0; i < length; ++i) {
            result[i] = static_cast<uint8_t>(dis(gen));
        }
    }
    return result;
}

std::string SecureRandomGenerator::generateHexString(size_t length) {
    auto bytes = generateBytes(length / 2 + 1);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length / 2; ++i) {
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str().substr(0, length);
}

std::string SecureRandomGenerator::generateBase64String(size_t length) {
    auto bytes = generateBytes(length);
    return bytesToBase64(bytes.data(), bytes.size());
}

uint32_t SecureRandomGenerator::generateUint32() {
    auto bytes = generateBytes(4);
    return static_cast<uint32_t>(bytes[0]) | 
           (static_cast<uint32_t>(bytes[1]) << 8) |
           (static_cast<uint32_t>(bytes[2]) << 16) |
           (static_cast<uint32_t>(bytes[3]) << 24);
}

uint64_t SecureRandomGenerator::generateUint64() {
    auto bytes = generateBytes(8);
    return static_cast<uint64_t>(bytes[0]) |
           (static_cast<uint64_t>(bytes[1]) << 8) |
           (static_cast<uint64_t>(bytes[2]) << 16) |
           (static_cast<uint64_t>(bytes[3]) << 24) |
           (static_cast<uint64_t>(bytes[4]) << 32) |
           (static_cast<uint64_t>(bytes[5]) << 40) |
           (static_cast<uint64_t>(bytes[6]) << 48) |
           (static_cast<uint64_t>(bytes[7]) << 56);
}

int32_t SecureRandomGenerator::generateInt32() {
    return static_cast<int32_t>(generateUint32());
}

int64_t SecureRandomGenerator::generateInt64() {
    return static_cast<int64_t>(generateUint64());
}

double SecureRandomGenerator::generateGaussian(double mean, double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(*this);
}

float SecureRandomGenerator::generateGaussianF(float mean, float stddev) {
    std::normal_distribution<float> dist(mean, stddev);
    return dist(*this);
}

void SecureRandomGenerator::fillBytes(uint8_t* buffer, size_t length) {
    auto bytes = generateBytes(length);
    std::memcpy(buffer, bytes.data(), length);
}

void SecureRandomGenerator::addEntropy(const uint8_t* data, size_t length) {
    RAND_add(data, static_cast<int>(length), static_cast<double>(length) / 2.0);
}

SecureRandomGenerator& SecureRandomGenerator::getInstance() {
    static SecureRandomGenerator instance;
    return instance;
}

// ============================================
// SHA256 HASHER
// ============================================
SHA256Hasher::SHA256Hasher() : m_context(nullptr), m_finalized(false) {
    initializeOpenSSL();
    m_context = EVP_MD_CTX_new();
    if (m_context) {
        EVP_DigestInit_ex(m_context, EVP_sha256(), nullptr);
    }
}

SHA256Hasher::~SHA256Hasher() {
    if (m_context) {
        EVP_MD_CTX_free(m_context);
    }
}

void SHA256Hasher::update(const uint8_t* data, size_t length) {
    if (m_context && !m_finalized) {
        EVP_DigestUpdate(m_context, data, length);
    }
}

void SHA256Hasher::update(const std::string& data) {
    update(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::array<uint8_t, SHA256_DIGEST_LENGTH> SHA256Hasher::finalize() {
    std::array<uint8_t, SHA256_DIGEST_LENGTH> result{};
    if (m_context && !m_finalized) {
        unsigned int len = SHA256_DIGEST_LENGTH;
        EVP_DigestFinal_ex(m_context, result.data(), &len);
        m_finalized = true;
        EVP_DigestInit_ex(m_context, EVP_sha256(), nullptr);
    }
    return result;
}

std::string SHA256Hasher::finalizeHex() {
    auto hash = finalize();
    return bytesToHex(hash.data(), hash.size());
}

std::string SHA256Hasher::finalizeBase64() {
    auto hash = finalize();
    return bytesToBase64(hash.data(), hash.size());
}

std::array<uint8_t, SHA256_DIGEST_LENGTH> SHA256Hasher::hash(const uint8_t* data, size_t length) {
    SHA256Hasher hasher;
    hasher.update(data, length);
    return hasher.finalize();
}

std::array<uint8_t, SHA256_DIGEST_LENGTH> SHA256Hasher::hash(const std::string& data) {
    return hash(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::string SHA256Hasher::hashHex(const uint8_t* data, size_t length) {
    return bytesToHex(hash(data, length).data(), SHA256_DIGEST_LENGTH);
}

std::string SHA256Hasher::hashHex(const std::string& data) {
    return hashHex(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::string SHA256Hasher::hashBase64(const uint8_t* data, size_t length) {
    return bytesToBase64(hash(data, length).data(), SHA256_DIGEST_LENGTH);
}

std::string SHA256Hasher::hashBase64(const std::string& data) {
    return hashBase64(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::array<uint8_t, SHA256_DIGEST_LENGTH> SHA256Hasher::hmac(
    const uint8_t* key, size_t keyLen,
    const uint8_t* data, size_t dataLen) {
    
    std::array<uint8_t, SHA256_DIGEST_LENGTH> result{};
    unsigned int len = SHA256_DIGEST_LENGTH;
    
    HMAC(EVP_sha256(), key, static_cast<int>(keyLen),
         data, dataLen, result.data(), &len);
    
    return result;
}

std::string SHA256Hasher::hmacHex(const std::string& key, const std::string& data) {
    auto h = hmac(
        reinterpret_cast<const uint8_t*>(key.data()), key.size(),
        reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return bytesToHex(h.data(), h.size());
}

// ============================================
// RSA KEY PAIR
// ============================================
RSAKeyPair::RSAKeyPair() : m_rsa(nullptr), m_hasPrivateKey(false) {
    initializeOpenSSL();
    m_rsa = RSA_new();
}

RSAKeyPair::~RSAKeyPair() {
    if (m_rsa) {
        if (m_hasPrivateKey) {
            RSA_clear_flags(m_rsa, RSA_F4);
        }
        RSA_free(m_rsa);
    }
}

RSAKeyPair::RSAKeyPair(RSAKeyPair&& other) noexcept
    : m_rsa(other.m_rsa), m_hasPrivateKey(other.m_hasPrivateKey) {
    other.m_rsa = nullptr;
    other.m_hasPrivateKey = false;
}

RSAKeyPair& RSAKeyPair::operator=(RSAKeyPair&& other) noexcept {
    if (this != &other) {
        if (m_rsa) RSA_free(m_rsa);
        m_rsa = other.m_rsa;
        m_hasPrivateKey = other.m_hasPrivateKey;
        other.m_rsa = nullptr;
        other.m_hasPrivateKey = false;
    }
    return *this;
}

bool RSAKeyPair::generate(size_t keyBits) {
    if (!m_rsa) return false;
    
    BIGNUM* e = BN_new();
    if (!e) return false;
    
    int ret = BN_set_word(e, RSA_F4);
    if (ret) {
        ret = RSA_generate_key_ex(m_rsa, static_cast<int>(keyBits), e, nullptr);
        if (ret) {
            m_hasPrivateKey = true;
        }
    }
    
    BN_free(e);
    return ret == 1;
}

bool RSAKeyPair::loadPrivateKey(const std::string& pem) {
    if (!m_rsa) return false;
    
    BIO* bio = BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size()));
    if (!bio) return false;
    
    RSA* temp = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (temp) {
        if (m_rsa != temp) {
            RSA_free(m_rsa);
            m_rsa = temp;
        }
        m_hasPrivateKey = true;
        return true;
    }
    
    return false;
}

bool RSAKeyPair::loadPublicKey(const std::string& pem) {
    if (!m_rsa) return false;
    
    BIO* bio = BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size()));
    if (!bio) return false;
    
    RSA* temp = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
    if (!temp) {
        // Try public key format
        BIO* bio2 = BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size()));
        temp = PEM_read_bio_RSA_PUBKEY(bio2, nullptr, nullptr, nullptr);
        BIO_free(bio2);
    }
    BIO_free(bio);
    
    if (temp) {
        RSA_free(m_rsa);
        m_rsa = temp;
        m_hasPrivateKey = false;
        return true;
    }
    
    return false;
}

std::optional<std::string> RSAKeyPair::exportPrivateKeyPem() {
    if (!m_rsa || !m_hasPrivateKey) return std::nullopt;
    
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) return std::nullopt;
    
    if (!PEM_write_bio_RSAPrivateKey(bio, m_rsa, nullptr, nullptr, 0, nullptr, nullptr)) {
        BIO_free(bio);
        return std::nullopt;
    }
    
    char* ptr;
    long len = BIO_get_mem_data(bio, &ptr);
    std::string result(ptr, len);
    BIO_free(bio);
    
    return result;
}

std::optional<std::string> RSAKeyPair::exportPublicKeyPem() {
    if (!m_rsa) return std::nullopt;
    
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) return std::nullopt;
    
    if (!PEM_write_bio_RSAPublicKey(bio, m_rsa)) {
        BIO_free(bio);
        return std::nullopt;
    }
    
    char* ptr;
    long len = BIO_get_mem_data(bio, &ptr);
    std::string result(ptr, len);
    BIO_free(bio);
    
    return result;
}

std::optional<std::vector<uint8_t>> RSAKeyPair::sign(
    const uint8_t* data, size_t length, int digestType) {
    
    if (!m_rsa || !m_hasPrivateKey) return std::nullopt;
    
    std::vector<uint8_t> signature(RSA_size(m_rsa));
    unsigned int sigLen = 0;
    
    if (!RSA_sign(digestType, data, static_cast<unsigned int>(length),
                  signature.data(), &sigLen, m_rsa)) {
        return std::nullopt;
    }
    
    signature.resize(sigLen);
    return signature;
}

std::optional<std::string> RSAKeyPair::signHex(const std::string& data, int digestType) {
    auto sig = sign(reinterpret_cast<const uint8_t*>(data.data()), data.size(), digestType);
    if (!sig) return std::nullopt;
    return bytesToHex(sig->data(), sig->size());
}

std::optional<std::vector<uint8_t>> RSAKeyPair::signPSS(
    const uint8_t* data, size_t length, int digestType, int saltLen) {
    
    if (!m_rsa || !m_hasPrivateKey) return std::nullopt;
    
    std::vector<uint8_t> signature(RSA_size(m_rsa));
    unsigned int sigLen = 0;
    
    if (!RSA_sign(digestType, data, static_cast<unsigned int>(length),
                  signature.data(), &sigLen, m_rsa)) {
        return std::nullopt;
    }
    
    signature.resize(sigLen);
    return signature;
}

std::optional<std::string> RSAKeyPair::signPSSHex(const std::string& data, int digestType) {
    auto sig = signPSS(reinterpret_cast<const uint8_t*>(data.data()), data.size(), digestType);
    if (!sig) return std::nullopt;
    return bytesToHex(sig->data(), sig->size());
}

bool RSAKeyPair::verify(
    const uint8_t* data, size_t length,
    const uint8_t* signature, size_t sigLength, int digestType) {
    
    if (!m_rsa) return false;
    
    return RSA_verify(digestType, data, static_cast<unsigned int>(length),
                      signature, static_cast<unsigned int>(sigLength), m_rsa) == 1;
}

bool RSAKeyPair::verifyHex(const std::string& data, const std::string& signatureHex, int digestType) {
    auto sig = hexToBytes(signatureHex);
    return verify(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                   sig.data(), sig.size(), digestType);
}

std::optional<std::vector<uint8_t>> RSAKeyPair::encrypt(
    const uint8_t* plaintext, size_t length, int padding) {
    
    if (!m_rsa) return std::nullopt;
    
    std::vector<uint8_t> ciphertext(RSA_size(m_rsa));
    int len = RSA_public_encrypt(static_cast<int>(length), plaintext,
                                  ciphertext.data(), m_rsa, padding);
    if (len < 0) return std::nullopt;
    
    ciphertext.resize(len);
    return ciphertext;
}

std::optional<std::vector<uint8_t>> RSAKeyPair::decrypt(
    const uint8_t* ciphertext, size_t length, int padding) {
    
    if (!m_rsa || !m_hasPrivateKey) return std::nullopt;
    
    std::vector<uint8_t> plaintext(RSA_size(m_rsa));
    int len = RSA_private_decrypt(static_cast<int>(length), ciphertext,
                                   plaintext.data(), m_rsa, padding);
    if (len < 0) return std::nullopt;
    
    plaintext.resize(len);
    return plaintext;
}

size_t RSAKeyPair::getKeySize() const {
    return m_rsa ? RSA_size(m_rsa) : 0;
}

std::string RSAKeyPair::getPublicKeyFingerprint() {
    if (!m_rsa) return "";
    
    const RSA* constRsa = m_rsa;
    std::vector<uint8_t> keyBytes(RSA_size(m_rsa));
    int len = i2d_RSAPublicKey(constRsa, nullptr);
    if (len > 0) {
        keyBytes.resize(len);
        unsigned char* p = keyBytes.data();
        i2d_RSAPublicKey(constRsa, &p);
        return SHA256Hasher::hashHex(keyBytes);
    }
    return "";
}

// ============================================
// EC KEY PAIR
// ============================================
ECKeyPair::ECKeyPair(int curve) : m_ecKey(nullptr), m_curve(curve) {
    initializeOpenSSL();
    m_ecKey = EC_KEY_new_by_curve_name(curve);
}

ECKeyPair::~ECKeyPair() {
    if (m_ecKey) {
        EC_KEY_free(m_ecKey);
    }
}

ECKeyPair::ECKeyPair(ECKeyPair&& other) noexcept
    : m_ecKey(other.m_ecKey), m_curve(other.m_curve) {
    other.m_ecKey = nullptr;
}

ECKeyPair& ECKeyPair::operator=(ECKeyPair&& other) noexcept {
    if (this != &other) {
        if (m_ecKey) EC_KEY_free(m_ecKey);
        m_ecKey = other.m_ecKey;
        m_curve = other.m_curve;
        other.m_ecKey = nullptr;
    }
    return *this;
}

bool ECKeyPair::generate() {
    if (!m_ecKey) return false;
    
    if (EC_KEY_generate_key(m_ecKey) != 1) return false;
    
    if (EC_KEY_check_key(m_ecKey) != 1) return false;
    
    return true;
}

std::optional<std::string> ECKeyPair::exportPrivateKeyPem() {
    if (!m_ecKey) return std::nullopt;
    
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) return std::nullopt;
    
    if (!PEM_write_bio_ECPrivateKey(bio, m_ecKey, nullptr, nullptr, 0, nullptr, nullptr)) {
        BIO_free(bio);
        return std::nullopt;
    }
    
    char* ptr;
    long len = BIO_get_mem_data(bio, &ptr);
    std::string result(ptr, len);
    BIO_free(bio);
    
    return result;
}

std::optional<std::string> ECKeyPair::exportPublicKeyPem() {
    if (!m_ecKey) return std::nullopt;
    
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) return std::nullopt;
    
    EC_KEY* pubKey = m_ecKey; // EC_KEY up_ref would be needed for const
    if (!PEM_write_bio_EC_PUBKEY(bio, m_ecKey)) {
        BIO_free(bio);
        return std::nullopt;
    }
    
    char* ptr;
    long len = BIO_get_mem_data(bio, &ptr);
    std::string result(ptr, len);
    BIO_free(bio);
    
    return result;
}

std::optional<std::vector<uint8_t>> ECKeyPair::sign(const uint8_t* data, size_t length) {
    if (!m_ecKey) return std::nullopt;
    
    ECDSA_SIG* sig = ECDSA_do_sign(data, static_cast<int>(length), m_ecKey);
    if (!sig) return std::nullopt;
    
    // Convert signature to DER format
    unsigned char* derBuf = nullptr;
    int derLen = i2d_ECDSA_SIG(sig, &derBuf);
    ECDSA_SIG_free(sig);
    
    if (derLen <= 0) return std::nullopt;
    
    std::vector<uint8_t> result(derBuf, derBuf + derLen);
    OPENSSL_free(derBuf);
    
    return result;
}

std::optional<std::string> ECKeyPair::signHex(const std::string& data) {
    auto sig = sign(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    if (!sig) return std::nullopt;
    return bytesToHex(sig->data(), sig->size());
}

bool ECKeyPair::verify(const uint8_t* data, size_t length,
                       const uint8_t* signature, size_t sigLen) {
    if (!m_ecKey) return false;
    
    const unsigned char* p = signature;
    ECDSA_SIG* sig = d2i_ECDSA_SIG(nullptr, &p, static_cast<long>(sigLen));
    if (!sig) return false;
    
    int result = ECDSA_do_verify(data, static_cast<int>(length), sig, m_ecKey);
    ECDSA_SIG_free(sig);
    
    return result == 1;
}

bool ECKeyPair::verifyHex(const std::string& data, const std::string& signatureHex) {
    auto sig = hexToBytes(signatureHex);
    return verify(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                   sig.data(), sig.size());
}

std::string ECKeyPair::getCurveName() const {
    if (!m_ecKey) return "";
    
    const EC_GROUP* group = EC_KEY_get0_group(m_ecKey);
    if (!group) return "";
    
    int nid = EC_group_get_curve_name(group);
    return OBJ_nid2sn(nid);
}

// ============================================
// ATTESTATION SIGNER
// ============================================
AttestationSigner::AttestationSigner() {
    initializeOpenSSL();
}

AttestationSigner::~AttestationSigner() {}

bool AttestationSigner::initialize() {
    // Generate RSA key for SafetyNet (RS256)
    if (!m_rsaKey.generate()) return false;
    
    // Generate EC key for Play Integrity (ES256)
    if (!m_ecKey.generate()) return false;
    
    // Generate attestation key ID
    auto fingerprint = m_rsaKey.getPublicKeyFingerprint();
    if (fingerprint.empty()) return false;
    
    m_attestationKeyId = fingerprint.substr(0, 16);
    
    // Generate certificate chain
    generateCertificateChain("CN=Android Keystore", "CN=Google Hardware Attestation Root");
    
    return true;
}

JWSResult AttestationSigner::generateSafetyNetJWS(const AttestationData& data) {
    JWSResult result;
    
    // Header: RS256
    result.header = createJwsHeader("RS256", "JWS");
    
    // Payload
    result.payload = createSafetyNetPayload(data);
    
    // Sign with RSA
    std::string signedData = result.header + "." + result.payload;
    auto signature = m_rsaKey.signHex(signedData, NID_sha256);
    if (signature) {
        result.signature = signature.value();
    }
    
    result.token = result.header + "." + result.payload + "." + result.signature;
    
    return result;
}

JWSResult AttestationSigner::generatePlayIntegrityJWS(const AttestationData& data) {
    JWSResult result;
    
    // Header: ES256
    result.header = createJwsHeader("ES256", "JWT");
    
    // Payload
    result.payload = createPlayIntegrityPayload(data);
    
    // Sign with EC
    std::string signedData = result.header + "." + result.payload;
    auto signature = m_ecKey.signHex(signedData);
    if (signature) {
        result.signature = signature.value();
    }
    
    result.token = result.header + "." + result.payload + "." + result.signature;
    
    return result;
}

bool AttestationSigner::generateCertificateChain(const std::string& subjectName, const std::string& issuerName, int validityDays) {
    // For a proper implementation, this would generate X.509 certificates
    // For now, we'll use placeholder certificates
    m_rootCert = "-----BEGIN CERTIFICATE-----\nMIIBkTCB+wIJAKbO...\n-----END CERTIFICATE-----";
    m_intermediateCert = "-----BEGIN CERTIFICATE-----\nMIIBjTCB+wIJAKbO...\n-----END CERTIFICATE-----";
    m_leafCert = "-----BEGIN CERTIFICATE-----\nMIIBjjCCAWS...\n-----END CERTIFICATE-----";
    return true;
}

std::vector<std::string> AttestationSigner::getCertificateChain() {
    return {m_leafCert, m_intermediateCert, m_rootCert};
}

void AttestationSigner::setKeyId(const std::string& keyId) {
    m_attestationKeyId = keyId;
}

std::string AttestationSigner::getKeyId() const {
    return m_attestationKeyId;
}

void AttestationSigner::setHardwareBacked(bool backed) {
    // This would set hardware backing flags in the attestation response
}

bool AttestationSigner::isHardwareBacked() const {
    return true; // Simulated as hardware-backed
}

std::string AttestationSigner::base64UrlEncode(const uint8_t* data, size_t length) {
    return bytesToBase64Url(data, length);
}

std::string AttestationSigner::base64UrlEncode(const std::vector<uint8_t>& data) {
    return base64UrlEncode(data.data(), data.size());
}

std::vector<uint8_t> AttestationSigner::base64UrlDecode(const std::string& encoded) {
    // Convert base64url to base64
    std::string base64 = encoded;
    std::replace(base64.begin(), base64.end(), '-', '+');
    std::replace(base64.begin(), base64.end(), '_', '/');
    
    // Pad with '='
    while (base64.size() % 4) {
        base64 += '=';
    }
    
    return base64ToBytes(base64);
}

std::string AttestationSigner::createJwsHeader(const std::string& alg, const std::string& typ) {
    std::stringstream ss;
    ss << "{\"alg\":\"" << alg << "\",\"typ\":\"" << typ << "\"}";
    return base64UrlEncode(reinterpret_cast<const uint8_t*>(ss.str().data()), ss.str().size());
}

std::string AttestationSigner::createSafetyNetPayload(const AttestationData& data) {
    std::stringstream ss;
    ss << "{";
    ss << "\"nonce\":\"" << data.nonce << "\",";
    ss << "\"timestamp\":" << data.timestamp << ",";
    ss << "\"packageName\":\"" << data.packageName << "\",";
    ss << "\"apkPackageName\":\"" << data.packageName << "\",";
    ss << "\"apkDigestSha256\":\"" << data.apkDigest << "\",";
    ss << "\"basicIntegrity\":" << (data.basicIntegrity == "true" ? "true" : "false") << ",";
    ss << "\"ctsProfileMatch\":" << (data.ctsProfileMatch == "true" ? "true" : "false") << ",";
    ss << "\"evaluationType\":\"" << data.evaluationType << "\",";
    ss << "\"deviceIntegrity\":[\"" << data.deviceIntegrity << "\"],";
    ss << "\"userInteractions\":{\"lastWeek\":1,\"lastMonth\":5,\"lastYear\":20},";
    ss << "\"environment\":{\"rootDetected\":false,\"debuggable\":false,\"hooksDetected\":false},";
    ss << "\"advice\":\"" << "\"";
    ss << "}";
    
    return base64UrlEncode(reinterpret_cast<const uint8_t*>(ss.str().data()), ss.str().size());
}

std::string AttestationSigner::createPlayIntegrityPayload(const AttestationData& data) {
    std::stringstream ss;
    ss << "{";
    ss << "\"deviceIntegrity\":[\"" << data.deviceIntegrity << "\"],";
    ss << "\"appIntegrity\":{\"appIntegrityType\":\"PLAY_RECOGNIZED\",\"packageName\":\"" << data.packageName << "\"},";
    ss << "\"accountDetails\":{\"accountActions\":\"NO_ACCOUNT\"},";
    ss << "\"nonce\":\"" << data.nonce << "\",";
    ss << "\"requestDetails\":{\"requestPackageName\":\"" << data.packageName << "\"},";
    ss << "\"timestamp\":{\"timeMillis\":\"" << data.timestamp << "\"},";
    ss << "\"securityProperties\":{\"deviceEncryptionStatus\":\"ENCRYPTED\",\"hardwareAttestationSupported\":true},";
    ss << "\"digest\":\"APK digest here\"";
    ss << "}";
    
    return base64UrlEncode(reinterpret_cast<const uint8_t*>(ss.str().data()), ss.str().size());
}

// ============================================
// NONCE GENERATOR
// ============================================
NonceGenerator::NonceGenerator() {}

NonceGenerator::~NonceGenerator() {}

std::string NonceGenerator::generate16() {
    auto bytes = m_rng.generateBytes(16);
    return bytesToHex(bytes.data(), 16);
}

std::string NonceGenerator::generate32() {
    auto bytes = m_rng.generateBytes(32);
    return bytesToHex(bytes.data(), 32);
}

std::string NonceGenerator::generate(size_t bytes) {
    auto bytesVec = m_rng.generateBytes(bytes);
    return bytesToHex(bytesVec.data(), bytesVec.size());
}

std::string NononceGenerator::generateBase64(size_t bytes) {
    return m_rng.generateBase64String(bytes);
}

std::string NonceGenerator::forSafetyNet() {
    return generate16();
}

std::string NonceGenerator::forPlayIntegrity() {
    return generate32();
}

std::string NonceGenerator::forDeviceIntegrity() {
    return generate32();
}

// ============================================
// DEVICE IDENTIFIER GENERATOR
// ============================================
DeviceIdentifierGenerator::DeviceIdentifierGenerator() {}

DeviceIdentifierGenerator::~DeviceIdentifierGenerator() {}

std::string DeviceIdentifierGenerator::generateIMEI(const std::string& tac) {
    if (tac.length() != 8) return "";
    
    std::string imei = tac;
    
    // Add 6 random digits (SNR)
    for (int i = 0; i < 6; ++i) {
        imei += std::to_string(m_rng.generateUint32() % 10);
    }
    
    // Add Luhn check digit
    imei += std::to_string(calculateLuhnCheckDigit(imei));
    
    return imei;
}

std::string DeviceIdentifierGenerator::generateAndroidId() {
    auto bytes = m_rng.generateBytes(8);
    return bytesToHex(bytes.data(), 8);
}

std::string DeviceIdentifierGenerator::generateGSFId() {
    uint64_t id = m_rng.generateUint64() % 9000000000000LL + 1000000000000LL;
    return std::to_string(id);
}

std::string DeviceIdentifierGenerator::generateSerialNumber(const std::string& manufacturer) {
    std::stringstream ss;
    
    if (manufacturer == "Samsung") {
        ss << "R5CR";
    } else if (manufacturer == "Google") {
        ss << "HT";
    } else if (manufacturer == "Xiaomi") {
        ss << "MLB";
    }
    
    auto hex = m_rng.generateHexString(8);
    ss << hex;
    
    return ss.str();
}

std::string DeviceIdentifierGenerator::generateDeviceUUID() {
    auto bytes = m_rng.generateBytes(16);
    std::string hex = bytesToHex(bytes.data(), 16);
    
    // Format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    return hex.substr(0, 8) + "-" + hex.substr(8, 4) + "-" + hex.substr(12, 4) + "-" + 
           hex.substr(16, 4) + "-" + hex.substr(20, 12);
}

std::string DeviceIdentifierGenerator::generateHardwareSerial() {
    return m_rng.generateHexString(32);
}

std::string DeviceIdentifierGenerator::generateMAC(const std::string& manufacturerOui) {
    std::string mac = manufacturerOui;
    auto bytes = m_rng.generateBytes(3);
    char separator = ':';
    
    for (size_t i = 0; i < 3; ++i) {
        mac += separator + bytesToHex(bytes.data() + i, 1);
    }
    
    return mac;
}

std::string DeviceIdentifierGenerator::generateWiFiMAC(const std::string& manufacturer) {
    static const std::map<std::string, std::string> ouiMap = {
        {"Samsung", "A0:CE:C8"}, {"Google", "4C:8D:79"}, {"Apple", "00:1A:2B"},
        {"Xiaomi", "64:09:80"}, {"OnePlus", "2A:53:4E"}, {"Huawei", "00:25:68"}
    };
    
    auto it = ouiMap.find(manufacturer);
    if (it != ouiMap.end()) {
        return generateMAC(it->second);
    }
    return generateMAC("00:1A:2B");
}

std::string DeviceIdentifierGenerator::generateBluetoothMAC(const std::string& manufacturer) {
    auto wifiMac = generateWiFiMAC(manufacturer);
    // Bluetooth MAC typically has the second-least-significant bit of the first octet set
    std::string firstOctet = wifiMac.substr(0, 2);
    int val = std::stoi(firstOctet, nullptr, 16);
    val |= 0x02; // Set local bit
    val &= 0xFE; // Clear multicast bit
    char buf[3];
    snprintf(buf, sizeof(buf), "%02X", val);
    return std::string(buf) + wifiMac.substr(2);
}

std::string DeviceIdentifierGenerator::generateBatterySerial() {
    std::stringstream ss;
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < 16; ++i) {
        ss << chars[m_rng.generateUint32() % 36];
    }
    return ss.str();
}

std::string DeviceIdentifierGenerator::generateKnoxId() {
    std::stringstream ss;
    ss << "0x";
    ss << m_rng.generateHexString(32);
    return ss.str();
}

int DeviceIdentifierGenerator::calculateLuhnCheckDigit(const std::string& partial) {
    int sum = 0;
    bool alternate = true;
    
    for (int i = static_cast<int>(partial.length()) - 1; i >= 0; --i) {
        int digit = partial[i] - '0';
        
        if (alternate) {
            digit *= 2;
            if (digit > 9) digit -= 9;
        }
        
        sum += digit;
        alternate = !alternate;
    }
    
    return (10 - (sum % 10)) % 10;
}

bool DeviceIdentifierGenerator::validateLuhn(const std::string& full) {
    if (full.length() != 15) return false;
    return calculateLuhnCheckDigit(full.substr(0, 14)) == (full[14] - '0');
}

// ============================================
// FINGERPRINT GENERATOR
// ============================================
FingerprintGenerator::FingerprintGenerator() {}

FingerprintGenerator::~FingerprintGenerator() {}

std::string FingerprintGenerator::generateBuildFingerprint(
    const std::string& brand, const std::string& device,
    const std::string& model, const std::string& buildId,
    const std::string& buildVersion) {
    
    std::stringstream ss;
    ss << brand << "/" << device << "/" << device << ":" << buildVersion << "/" 
       << buildId << "/" << generateSerialNumber(brand) << ":user/release-keys";
    return ss.str();
}

std::string FingerprintGenerator::generateBootImageId() {
    auto bytes = m_rng.generateBytes(32);
    return bytesToHex(bytes.data(), bytes.size());
}

std::string FingerprintGenerator::generateVendorBootImageId() {
    auto bytes = m_rng.generateBytes(32);
    return bytesToHex(bytes.data(), bytes.size());
}

std::string FingerprintGenerator::generateVbmetaDigest() {
    auto bytes = m_rng.generateBytes(32);
    return bytesToHex(bytes.data(), bytes.size());
}

std::string FingerprintGenerator::generateVerifiedBootKeyHash() {
    auto bytes = m_rng.generateBytes(32);
    return bytesToHex(bytes.data(), bytes.size());
}

uint64_t FingerprintGenerator::generateRollbackIndex() {
    return m_rng.generateUint64() % 1000;
}

bool FingerprintGenerator::isOemUnlockEnabled() {
    return false; // Device should appear locked
}

bool FingerprintGenerator::isDeviceLocked() {
    return true;
}

std::string FingerprintGenerator::getSelinuxStatus() {
    return "Enforcing";
}

bool FingerprintGenerator::isEnforcing() {
    return true;
}

// ============================================
// TIMING JITTER INJECTOR
// ============================================
TimingJitterInjector::TimingJitterInjector(double initialJitter)
    : m_jitterPercentage(initialJitter), m_enabled(false), m_baseUptime(0) {
    
    // Initialize realistic base uptime (days)
    m_baseUptime = m_rng.generateUint64() % (365 * 24 * 60 * 60 * 1000); // Up to 1 year in ms
}

TimingJitterInjector::~TimingJitterInjector() {}

void TimingJitterInjector::enable() { m_enabled = true; }
void TimingJitterInjector::disable() { m_enabled = false; }
bool TimingJitterInjector::isEnabled() const { return m_enabled; }

void TimingJitterInjector::setJitterPercentage(double percentage) {
    m_jitterPercentage = std::max(0.0, std::min(100.0, percentage));
}

double TimingJitterInjector::getJitterPercentage() const { return m_jitterPercentage; }

uint64_t TimingJitterInjector::injectTimestampJitter(uint64_t baseTimestamp) {
    if (!m_enabled || m_jitterPercentage == 0) return baseTimestamp;
    
    int64_t jitter = static_cast<int64_t>(
        baseTimestamp * m_jitterPercentage / 100.0 * 
        (m_rng.generateInt64() % 1000 / 500.0 - 1.0)
    );
    
    return baseTimestamp + jitter;
}

uint64_t TimingJitterInjector::getRealisticUptimeDays() {
    // Return uptime in milliseconds
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    return m_baseUptime + (now % (365LL * 24 * 60 * 60 * 1000));
}

uint64_t TimingJitterInjector::getBootTimestamp() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    return now - getRealisticUptimeDays();
}

uint64_t TimingJitterInjector::getMonotonicTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

void TimingJitterInjector::addExecutionDelay() {
    if (!m_enabled) return;
    
    int delayUs = static_cast<int>(m_jitterPercentage * 100);
    std::this_thread::sleep_for(std::chrono::microseconds(
        m_rng.generateUint64() % delayUs
    ));
}

void TimingJitterInjector::flushCache() {
    // Memory fence to prevent speculative execution timing leaks
    std::atomic_thread_fence(std::memory_order_seq_cst);
    
    // Touch memory to force cache eviction
    std::vector<char> buffer(4096);
    for (size_t i = 0; i < buffer.size(); i += 64) {
        buffer[i] = static_cast<char>(i);
    }
}

uint64_t TimingJitterInjector::getRDTSCWithJitter() {
    uint64_t tsc;
    #ifdef __x86_64__
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    tsc = ((uint64_t)hi << 32) | lo;
    
    if (m_enabled && m_jitterPercentage > 0) {
        int64_t jitter = tsc * m_jitterPercentage / 100;
        tsc += (m_rng.generateInt64() % jitter) - (jitter / 2);
    }
    #else
    tsc = getMonotonicTime() * 1000000; // Fallback to microseconds
    #endif
    
    return tsc;
}

int TimingJitterInjector::measureCacheLatency() {
    // Simple cache timing measurement
    std::vector<int> data(256);
    for (int i = 0; i < 256; ++i) data[i] = i;
    
    volatile int sink = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        sink += data[sink & 255];
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    return static_cast<int>(ns / 1000); // Return in microseconds
}

// ============================================
// ADVANCED SENSOR NOISE GENERATOR
// ============================================
AdvancedSensorNoiseGenerator::AdvancedSensorNoiseGenerator()
    : m_brownianState(0.0), m_pinkState(0.0), m_lastTimestamp(0) {}

AdvancedSensorNoiseGenerator::~AdvancedSensorNoiseGenerator() {}

double AdvancedSensorNoiseGenerator::gaussian(double mean, double stddev) {
    return m_rng.generateGaussian(mean, stddev);
}

float AdvancedSensorNoiseGenerator::gaussianF(float mean, float stddev) {
    return m_rng.generateGaussianF(mean, stddev);
}

double AdvancedSensorNoiseGenerator::pink(double value, double stddev) {
    // Simplified pink noise using exponential filter
    m_pinkState = 0.99 * m_pinkState + 0.01 * m_rng.generateGaussian(0, stddev);
    return value + m_pinkState;
}

float AdvancedSensorNoiseGenerator::pinkF(float value, float stddev) {
    return static_cast<float>(pink(value, stddev));
}

double AdvancedSensorNoiseGenerator::brownian(double value, double stepSize) {
    // Brownian (random walk) noise
    m_brownianState += m_rng.generateGaussian(0, stepSize);
    // Keep within reasonable bounds
    if (std::abs(m_brownianState) > 100) m_brownianState *= 0.9;
    return value + m_brownianState;
}

float AdvancedSensorNoiseGenerator::brownianF(float value, float stepSize) {
    return static_cast<float>(brownian(value, stepSize));
}

double AdvancedSensorNoiseGenerator::flicker(double value, double stddev) {
    // Very low frequency drift - flicker noise
    static double flickerState = 0;
    flickerState += m_rng.generateGaussian(0, stddev * 0.01);
    if (std::abs(flickerState) > 10) flickerState *= 0.95;
    return value + flickerState;
}

AdvancedSensorNoiseGenerator::Vector3D AdvancedSensorNoiseGenerator::addAccelerometerNoise(
    Vector3D base, bool isMoving) {
    
    double stddev = isMoving ? 0.05 : 0.002; // Higher noise when moving
    
    // Add combination of noise types
    double noiseX = pink(base.x, stddev) + brownian(0, stddev * 0.1);
    double noiseY = pink(base.y, stddev) + brownian(0, stddev * 0.1);
    double noiseZ = pink(base.z, stddev) + brownian(0, stddev * 0.1);
    
    return {base.x + noiseX, base.y + noiseY, base.z + noiseZ};
}

AdvancedSensorNoiseGenerator::Vector3D AdvancedSensorNoiseGenerator::addGyroscopeNoise(
    Vector3D base, bool isMoving) {
    
    double stddev = isMoving ? 0.1 : 0.01;
    
    return {
        pink(base.x, stddev),
        pink(base.y, stddev),
        pink(base.z, stddev)
    };
}

AdvancedSensorNoiseGenerator::Vector3D AdvancedSensorNoiseGenerator::addMagnetometerNoise(
    Vector3D base) {
    
    // Magnetometer has relatively stable readings with small noise
    return {
        gaussian(base.x, 0.5),
        gaussian(base.y, 0.5),
        gaussian(base.z, 0.5)
    };
}

AdvancedSensorNoiseGenerator::GPSData AdvancedSensorNoiseGenerator::addGPSNoise(
    GPSData base, bool isMoving) {
    
    double noiseLat = m_rng.generateGaussian(0, base.accuracy * 0.1);
    double noiseLon = m_rng.generateGaussian(0, base.accuracy * 0.1);
    
    if (isMoving) {
        // Add path smoothing when moving
        noiseLat *= 2;
        noiseLon *= 2;
    }
    
    return {
        base.latitude + noiseLat,
        base.longitude + noiseLon,
        std::max(1.0, base.accuracy + m_rng.generateGaussian(0, 0.5))
    };
}

uint64_t AdvancedSensorNoiseGenerator::addTimestampJitter(uint64_t baseTimestamp, double stddevMs) {
    int64_t jitter = static_cast<int64_t>(m_rng.generateGaussian(0, stddevMs));
    return baseTimestamp + jitter;
}

double AdvancedSensorNoiseGenerator::addBatteryTemperatureNoise(double baseTemp) {
    // Battery temperature drifts slowly
    static double tempDrift = 0;
    tempDrift += m_rng.generateGaussian(0, 0.01);
    if (std::abs(tempDrift) > 2) tempDrift *= 0.99;
    return baseTemp + tempDrift + m_rng.generateGaussian(0, 0.1);
}

int AdvancedSensorNoiseGenerator::addBatteryVoltageNoise(int baseVoltage) {
    return baseVoltage + m_rng.generateInt32() % 10 - 5;
}

int AdvancedSensorNoiseGenerator::addBatteryChargeNoise(int baseCharge) {
    return std::max(0, std::min(100, baseCharge + m_rng.generateInt32() % 3 - 1));
}

double AdvancedSensorNoiseGenerator::generateGaussian(double stddev) {
    std::normal_distribution<double> dist(0, stddev);
    return dist(m_rng);
}

double AdvancedSensorNoiseGenerator::generateUniform(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(m_rng);
}

// ============================================
// UTILITY FUNCTIONS
// ============================================
std::string bytesToHex(const uint8_t* data, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

std::string bytesToHex(const std::vector<uint8_t>& data) {
    return bytesToHex(data.data(), data.size());
}

std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> result;
    result.reserve(hex.length() / 2);
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byte = hex.substr(i, 2);
        result.push_back(static_cast<uint8_t>(std::stoi(byte, nullptr, 16)));
    }
    
    return result;
}

std::string bytesToBase64(const uint8_t* data, size_t length) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    int i = 0;
    int len = static_cast<int>(length);
    const unsigned char* bytes = data;
    
    while (len--) {
        unsigned char b = *(bytes++);
        result += chars[(b >> 2) & 0x3F];
        result += chars[((b << 4) | ((i + 1 < len) ? (unsigned char)(*(bytes)) >> 4 : 0)) & 0x3F];
        
        if (i++ < len) {
            result += chars[((*(bytes - 1) << 2) | ((i < len) ? (unsigned char)(*(bytes)) >> 6 : 0)) & 0x3F];
        }
        if (i++ < len) {
            result += chars[*(bytes++) & 0x3F];
        }
    }
    
    // Padding
    while (result.length() % 4) {
        result += '=';
    }
    
    return result;
}

std::string bytesToBase64(const std::string& data) {
    return bytesToBase64(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::vector<uint8_t> base64ToBytes(const std::string& base64) {
    static const uint8_t decodeTable[128] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
    };
    
    std::vector<uint8_t> result;
    int val = 0, bits = -8;
    
    for (char c : base64) {
        if (c == '=') break;
        if (c < 0 || c > 127) continue;
        
        int v = decodeTable[static_cast<unsigned char>(c)];
        if (v == 64) continue;
        
        val = (val << 6) | v;
        bits += 6;
        
        if (bits >= 0) {
            result.push_back(static_cast<uint8_t>((val >> bits) & 0xFF));
            bits -= 8;
        }
    }
    
    return result;
}

std::string bytesToBase64Url(const uint8_t* data, size_t length) {
    std::string base64 = bytesToBase64(data, length);
    // Convert to base64url
    std::replace(base64.begin(), base64.end(), '+', '-');
    std::replace(base64.begin(), base64.end(), '/', '_');
    // Remove padding
    while (!base64.empty() && base64.back() == '=') {
        base64.pop_back();
    }
    return base64;
}

std::string bytesToBase64Url(const std::vector<uint8_t>& data) {
    return bytesToBase64Url(data.data(), data.size());
}

std::vector<uint8_t> base64UrlToBytes(const std::string& base64Url) {
    std::string base64 = base64Url;
    // Convert from base64url
    std::replace(base64.begin(), base64.end(), '-', '+');
    std::replace(base64.begin(), base64.end(), '_', '/');
    // Add padding
    while (base64.length() % 4) {
        base64 += '=';
    }
    return base64ToBytes(base64);
}

void secureZero(void* ptr, size_t len) {
    volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
    while (len--) {
        *p++ = 0;
    }
}

void secureZeroString(std::string& str) {
    secureZero(const_cast<char*>(str.data()), str.size());
    str.clear();
}

std::string getOpenSSLError() {
    std::stringstream ss;
    unsigned long err;
    while ((err = ERR_get_error()) != 0) {
        ss << ERR_error_string(err, nullptr) << "; ";
    }
    return ss.str();
}

int getOpenSSLErrorCode() {
    return ERR_get_error();
}

// ============================================
// CERTIFICATE BUILDER (Stub implementation)
// ============================================
CertificateBuilder::CertificateBuilder() : m_validityDays(365), m_serialNumber(1), m_keyType(RSA_KEY_SIZE) {}

CertificateBuilder::~CertificateBuilder() {}

CertificateBuilder& CertificateBuilder::setSubject(const std::string& subject) {
    m_subjectName = subject;
    return *this;
}

CertificateBuilder& CertificateBuilder::setIssuer(const std::string& issuer) {
    m_issuerName = issuer;
    return *this;
}

CertificateBuilder& CertificateBuilder::setValidityDays(int days) {
    m_validityDays = days;
    return *this;
}

CertificateBuilder& CertificateBuilder::setSerialNumber(uint64_t serial) {
    m_serialNumber = serial;
    return *this;
}

CertificateBuilder& CertificateBuilder::setKeyType(int type) {
    m_keyType = type;
    return *this;
}

CertificateBuilder& CertificateBuilder::addExtension(const std::string& name, const std::string& value) {
    m_extensions.push_back({name, value});
    return *this;
}

std::optional<std::string> CertificateBuilder::buildSelfSigned(const RSAKeyPair& keyPair) {
    // Placeholder - would need full X.509 certificate generation
    return "-----BEGIN CERTIFICATE-----\nPLACEHOLDER\n-----END CERTIFICATE-----";
}

std::optional<std::string> CertificateBuilder::buildSelfSigned(const ECKeyPair& keyPair) {
    return buildSelfSigned(RSAKeyPair{});
}

std::optional<std::string> CertificateBuilder::buildSignedByCA(const RSAKeyPair& subjectKey, const RSAKeyPair& caKey) {
    return buildSelfSigned(subjectKey);
}

std::optional<std::string> CertificateBuilder::buildSignedByCA(const ECKeyPair& subjectKey, const ECKeyPair& caKey) {
    return buildSelfSigned(ECKeyPair{});
}

std::optional<std::vector<uint8_t>> CertificateBuilder::buildDer(const RSAKeyPair& keyPair) {
    return std::vector<uint8_t>();
}

std::string CertificateBuilder::getFingerprint(const std::string& pem) {
    auto der = base64ToBytes(pem);
    return SHA256Hasher::hashHex(der);
}

std::string CertificateBuilder::getFingerprint(const std::vector<uint8_t>& der) {
    return SHA256Hasher::hashHex(der);
}

} // namespace Crypto
} // namespace VirtualPhonePro
