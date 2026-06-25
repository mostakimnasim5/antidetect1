/**
 * CryptoUtils - Enterprise-Grade Cryptographic Utilities
 * 
 * This module provides cryptographically secure random generation,
 * proper SHA-256 hashing, RSA/EC key generation and signing
 * for attestation responses that pass real validation checks.
 */

#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <optional>
#include <memory>
#include <functional>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/ecdsa.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/opensslv.h>

namespace VirtualPhonePro {
namespace Crypto {

// ============================================
// CONSTANTS
// ============================================
constexpr size_t SHA256_DIGEST_LENGTH = 32;
constexpr size_t RSA_KEY_SIZE = 2048;
constexpr size_t EC_KEY_SIZE = 256;
constexpr size_t RANDOM_BYTES_STRONG = 32;
constexpr size_t MAX_CERT_CHAIN_LENGTH = 10;

// ============================================
// EXCEPTION TYPES
// ============================================
class CryptoException : public std::exception {
private:
    std::string m_message;
    int m_errorCode;
    
public:
    CryptoException(const std::string& msg, int errCode = 0)
        : m_message(msg), m_errorCode(errCode) {}
    
    const char* what() const noexcept override {
        return m_message.c_str();
    }
    
    int errorCode() const { return m_errorCode; }
};

// ============================================
// SECURE RANDOM GENERATOR
// ============================================
class SecureRandomGenerator {
private:
    std::array<uint8_t, 64> m_seedBuffer;
    bool m_initialized;
    
public:
    SecureRandomGenerator();
    ~SecureRandomGenerator();
    
    // Non-copyable
    SecureRandomGenerator(const SecureRandomGenerator&) = delete;
    SecureRandomGenerator& operator=(const SecureRandomGenerator&) = delete;
    
    // Generate cryptographically secure random bytes
    std::vector<uint8_t> generateBytes(size_t length);
    std::string generateHexString(size_t length);
    std::string generateBase64String(size_t length);
    
    // Generate specific types
    uint32_t generateUint32();
    uint64_t generateUint64();
    int32_t generateInt32();
    int64_t generateInt64();
    
    // Generate random with gaussian distribution for sensor simulation
    double generateGaussian(double mean, double stddev);
    float generateGaussianF(float mean, float stddev);
    
    // Fill existing buffer
    void fillBytes(uint8_t* buffer, size_t length);
    
    // Initialize with additional entropy
    void addEntropy(const uint8_t* data, size_t length);
    
    // Check if properly seeded
    bool isInitialized() const { return m_initialized; }
    
    // Singleton access
    static SecureRandomGenerator& getInstance();
};

// ============================================
// SHA-256 HASHING
// ============================================
class SHA256Hasher {
private:
    EVP_MD_CTX* m_context;
    bool m_finalized;
    
public:
    SHA256Hasher();
    ~SHA256Hasher();
    
    SHA256Hasher(const SHA256Hasher&) = delete;
    SHA256Hasher& operator=(const SHA256Hasher&) = delete;
    
    // Instance methods
    void update(const uint8_t* data, size_t length);
    void update(const std::string& data);
    std::array<uint8_t, SHA256_DIGEST_LENGTH> finalize();
    std::string finalizeHex();
    std::string finalizeBase64();
    
    // Static convenience methods
    static std::array<uint8_t, SHA256_DIGEST_LENGTH> hash(const uint8_t* data, size_t length);
    static std::array<uint8_t, SHA256_DIGEST_LENGTH> hash(const std::string& data);
    static std::string hashHex(const uint8_t* data, size_t length);
    static std::string hashHex(const std::string& data);
    static std::string hashBase64(const uint8_t* data, size_t length);
    static std::string hashBase64(const std::string& data);
    
    // HMAC-SHA256
    static std::array<uint8_t, SHA256_DIGEST_LENGTH> hmac(
        const uint8_t* key, size_t keyLen,
        const uint8_t* data, size_t dataLen);
    static std::string hmacHex(
        const std::string& key,
        const std::string& data);
};

// ============================================
// RSA KEY PAIR GENERATION & SIGNING
// ============================================
class RSAKeyPair {
private:
    RSA* m_rsa;
    bool m_hasPrivateKey;
    
public:
    RSAKeyPair();
    ~RSAKeyPair();
    
    RSAKeyPair(RSAKeyPair&& other) noexcept;
    RSAKeyPair& operator=(RSAKeyPair&& other) noexcept;
    
    // Non-copyable for private key safety
    RSAKeyPair(const RSAKeyPair&) = delete;
    RSAKeyPair& operator=(const RSAKeyPair&) = delete;
    
    // Generate new key pair
    bool generate(size_t keyBits = RSA_KEY_SIZE);
    
    // Load from PEM strings
    bool loadPrivateKey(const std::string& pem);
    bool loadPublicKey(const std::string& pem);
    
    // Export to PEM strings
    std::optional<std::string> exportPrivateKeyPem();
    std::optional<std::string> exportPublicKeyPem();
    
    // Sign data (PKCS#1 v1.5 padding)
    std::optional<std::vector<uint8_t>> sign(
        const uint8_t* data, size_t length,
        int digestType = NID_sha256);
    std::optional<std::string> signHex(
        const std::string& data,
        int digestType = NID_sha256);
    
    // Sign with PSS padding (more secure)
    std::optional<std::vector<uint8_t>> signPSS(
        const uint8_t* data, size_t length,
        int digestType = NID_sha256,
        int saltLen = RSA_PKCS1_SSLeay);
    std::optional<std::string> signPSSHex(
        const std::string& data,
        int digestType = NID_sha256);
    
    // Verify signature
    bool verify(
        const uint8_t* data, size_t length,
        const uint8_t* signature, size_t sigLength,
        int digestType = NID_sha256);
    bool verifyHex(
        const std::string& data,
        const std::string& signatureHex,
        int digestType = NID_sha256);
    
    // Encrypt/Decrypt
    std::optional<std::vector<uint8_t>> encrypt(
        const uint8_t* plaintext, size_t length,
        int padding = RSA_PKCS1_OAEP_PADDING);
    std::optional<std::vector<uint8_t>> decrypt(
        const uint8_t* ciphertext, size_t length,
        int padding = RSA_PKCS1_OAEP_PADDING);
    
    // Get key info
    size_t getKeySize() const;
    std::string getPublicKeyFingerprint();
    
    // Access raw RSA*
    RSA* get() { return m_rsa; }
    const RSA* get() const { return m_rsa; }
};

// ============================================
// EC KEY PAIR (for ES256 signatures)
// ============================================
class ECKeyPair {
private:
    EC_KEY* m_ecKey;
    int m_curve;
    
public:
    ECKeyPair(int curve = NID_X9_62_prime256v1); // P-256
    ~ECKeyPair();
    
    ECKeyPair(ECKeyPair&& other) noexcept;
    ECKeyPair& operator=(ECKeyPair&& other) noexcept;
    
    ECKeyPair(const ECKeyPair&) = delete;
    ECKeyPair& operator=(const ECKeyPair&) = delete;
    
    // Generate new key
    bool generate();
    
    // Export/Import
    std::optional<std::string> exportPrivateKeyPem();
    std::optional<std::string> exportPublicKeyPem();
    std::optional<std::vector<uint8_t>> exportPrivateKeyDer();
    std::optional<std::vector<uint8_t>> exportPublicKeyDer();
    
    // Sign (returns raw r,s components concatenated)
    std::optional<std::vector<uint8_t>> sign(
        const uint8_t* data, size_t length);
    std::optional<std::string> signHex(const std::string& data);
    
    // Verify
    bool verify(
        const uint8_t* data, size_t length,
        const uint8_t* signature, size_t sigLength);
    bool verifyHex(
        const std::string& data,
        const std::string& signatureHex);
    
    // Get key info
    int getCurve() const { return m_curve; }
    std::string getCurveName() const;
    
    EC_KEY* get() { return m_ecKey; }
    const EC_KEY* get() const { return m_ecKey; }
};

// ============================================
// ATTESTATION SIGNER (combines RSA + EC)
// ============================================
class AttestationSigner {
private:
    RSAKeyPair m_rsaKey;
    ECKeyPair m_ecKey;
    std::string m_attestationKeyId;
    
    // Certificate chain components
    std::string m_rootCert;
    std::string m_intermediateCert;
    std::string m_leafCert;
    
public:
    AttestationSigner();
    ~AttestationSigner();
    
    // Initialize with fresh keys
    bool initialize();
    
    // Generate attestation response
    struct AttestationData {
        std::string nonce;
        std::string timestamp;
        std::string packageName;
        std::string apkDigest;
        std::string basicIntegrity;
        std::string ctsProfileMatch;
        std::string evaluationType;
        std::string deviceIntegrity;
        std::string verifiedBootState;
        std::string securityLevel;
    };
    
    // Generate JWS for SafetyNet/Play Integrity
    struct JWSResult {
        std::string header;
        std::string payload;
        std::string signature;
        std::string token;  // header.payload.signature
    };
    
    // SafetyNet attestation (RS256)
    JWSResult generateSafetyNetJWS(const AttestationData& data);
    
    // Play Integrity token (ES256)
    JWSResult generatePlayIntegrityJWS(const AttestationData& data);
    
    // Generate certificate chain (X.509)
    bool generateCertificateChain(
        const std::string& subjectName,
        const std::string& issuerName,
        int validityDays = 365);
    
    // Get certificate chain
    std::vector<std::string> getCertificateChain();
    
    // Set attestation key ID
    void setKeyId(const std::string& keyId);
    std::string getKeyId() const;
    
    // Hardware-backed attestation flags
    void setHardwareBacked(bool backed);
    bool isHardwareBacked() const;
    
private:
    std::string base64UrlEncode(const uint8_t* data, size_t length);
    std::string base64UrlEncode(const std::vector<uint8_t>& data);
    std::vector<uint8_t> base64UrlDecode(const std::string& encoded);
    
    std::string createJwsHeader(const std::string& alg, const std::string& typ);
    std::string createSafetyNetPayload(const AttestationData& data);
    std::string createPlayIntegrityPayload(const AttestationData& data);
};

// ============================================
// NONCE GENERATOR
// ============================================
class NonceGenerator {
private:
    SecureRandomGenerator m_rng;
    
public:
    NonceGenerator();
    ~NonceGenerator();
    
    // Generate 16-byte nonce (standard for SafetyNet)
    std::string generate16();
    
    // Generate 32-byte nonce (standard for Play Integrity)
    std::string generate32();
    
    // Generate arbitrary length nonce
    std::string generate(size_t bytes);
    
    // Generate base64-encoded nonce
    std::string generateBase64(size_t bytes);
    
    // Generate nonce for specific purpose
    std::string forSafetyNet();
    std::string forPlayIntegrity();
    std::string forDeviceIntegrity();
};

// ============================================
// DEVICE IDENTIFIER GENERATOR
// ============================================
class DeviceIdentifierGenerator {
private:
    SecureRandomGenerator m_rng;
    SHA256Hasher m_hasher;
    
public:
    DeviceIdentifierGenerator();
    ~DeviceIdentifierGenerator();
    
    // Generate valid IMEI with proper Luhn check digit
    std::string generateIMEI(const std::string& tac);
    
    // Generate Android ID (16 hex chars)
    std::string generateAndroidId();
    
    // Generate GSF ID (Google Services Framework)
    std::string generateGSFId();
    
    // Generate serial number with proper format
    std::string generateSerialNumber(const std::string& manufacturer);
    
    // Generate device UUID
    std::string generateDeviceUUID();
    
    // Generate hardware serial
    std::string generateHardwareSerial();
    
    // Generate MAC address with proper OUI
    std::string generateMAC(const std::string& manufacturerOui);
    
    // Generate WiFi MAC for specific manufacturer
    std::string generateWiFiMAC(const std::string& manufacturer);
    
    // Generate Bluetooth MAC
    std::string generateBluetoothMAC(const std::string& manufacturer);
    
    // Generate battery serial
    std::string generateBatterySerial();
    
    // Generate Knox ID (Samsung)
    std::string generateKnoxId();
    
private:
    int calculateLuhnCheckDigit(const std::string& partial);
    bool validateLuhn(const std::string& full);
};

// ============================================
// FINGERPRINT GENERATOR
// ============================================
class FingerprintGenerator {
private:
    SecureRandomGenerator m_rng;
    SHA256Hasher m_hasher;
    
public:
    FingerprintGenerator();
    ~FingerprintGenerator();
    
    // Generate build fingerprint
    std::string generateBuildFingerprint(
        const std::string& brand,
        const std::string& device,
        const std::string& model,
        const std::string& buildId,
        const std::string& buildVersion);
    
    // Generate boot image ID
    std::string generateBootImageId();
    
    // Generate vendor boot image ID
    std::string generateVendorBootImageId();
    
    // Generate vbmeta digest
    std::string generateVbmetaDigest();
    
    // Generate verified boot key hash
    std::string generateVerifiedBootKeyHash();
    
    // Generate rollback index
    uint64_t generateRollbackIndex();
    
    // Generate device unlock state
    bool isOemUnlockEnabled();
    bool isDeviceLocked();
    
    // Generate SELinux status
    std::string getSelinuxStatus();
    bool isEnforcing();
};

// ============================================
// TIMING JITTER INJECTOR
// ============================================
class TimingJitterInjector {
private:
    SecureRandomGenerator m_rng;
    double m_jitterPercentage;
    bool m_enabled;
    uint64_t m_baseUptime;
    
public:
    TimingJitterInjector(double initialJitter = 0.0);
    ~TimingJitterInjector();
    
    void enable();
    void disable();
    bool isEnabled() const;
    
    void setJitterPercentage(double percentage);
    double getJitterPercentage() const;
    
    // Inject jitter into timestamp
    uint64_t injectTimestampJitter(uint64_t baseTimestamp);
    
    // Get realistic uptime (days of uptime)
    uint64_t getRealisticUptimeDays();
    
    // Get boot timestamp
    uint64_t getBootTimestamp();
    
    // Get current monotonic time
    uint64_t getMonotonicTime();
    
    // Add execution delay
    void addExecutionDelay();
    
    // Flush CPU cache to prevent timing attacks
    void flushCache();
    
    // Get RDTSC value with jitter
    uint64_t getRDTSCWithJitter();
    
    // Generate cache timing noise
    int measureCacheLatency();
};

// ============================================
// SENSOR NOISE GENERATOR
// ============================================
class AdvancedSensorNoiseGenerator {
private:
    SecureRandomGenerator m_rng;
    
    // State for noise models
    double m_brownianState;
    double m_pinkState;
    uint64_t m_lastTimestamp;
    
public:
    AdvancedSensorNoiseGenerator();
    ~AdvancedSensorNoiseGenerator();
    
    // Gaussian (white) noise
    double gaussian(double mean, double stddev);
    float gaussianF(float mean, float stddev);
    
    // Pink noise (1/f noise) - more realistic sensor noise
    double pink(double value, double stddev);
    float pinkF(float value, float stddev);
    
    // Brownian (random walk) noise
    double brownian(double value, double stepSize);
    float brownianF(float value, float stepSize);
    
    // Flicker noise (1/f²) - for very low frequency drift
    double flicker(double value, double stddev);
    
    // Apply realistic sensor noise to accelerometer data
    struct Vector3D {
        double x, y, z;
    };
    
    Vector3D addAccelerometerNoise(Vector3D base, bool isMoving = false);
    Vector3D addGyroscopeNoise(Vector3D base, bool isMoving = false);
    Vector3D addMagnetometerNoise(Vector3D base);
    
    // GPS noise with realistic accuracy
    struct GPSData {
        double latitude;
        double longitude;
        double accuracy;
    };
    
    GPSData addGPSNoise(GPSData base, bool isMoving = false);
    
    // Timestamp jitter
    uint64_t addTimestampJitter(uint64_t baseTimestamp, double stddevMs = 1.0);
    
    // Battery sensor noise
    double addBatteryTemperatureNoise(double baseTemp);
    int addBatteryVoltageNoise(int baseVoltage);
    int addBatteryChargeNoise(int baseCharge);
    
private:
    double generateGaussian(double stddev);
    double generateUniform(double min, double max);
};

// ============================================
// X.509 CERTIFICATE BUILDER
// ============================================
class CertificateBuilder {
private:
    std::string m_subjectName;
    std::string m_issuerName;
    int m_validityDays;
    uint64_t m_serialNumber;
    int m_keyType;  // RSA or EC
    
    std::vector<std::pair<std::string, std::string>> m_extensions;
    
public:
    CertificateBuilder();
    ~CertificateBuilder();
    
    CertificateBuilder& setSubject(const std::string& subject);
    CertificateBuilder& setIssuer(const std::string& issuer);
    CertificateBuilder& setValidityDays(int days);
    CertificateBuilder& setSerialNumber(uint64_t serial);
    CertificateBuilder& setKeyType(int type);
    
    CertificateBuilder& addExtension(const std::string& name, const std::string& value);
    
    // Build self-signed certificate
    std::optional<std::string> buildSelfSigned(const RSAKeyPair& keyPair);
    std::optional<std::string> buildSelfSigned(const ECKeyPair& keyPair);
    
    // Build certificate signed by CA
    std::optional<std::string> buildSignedByCA(
        const RSAKeyPair& subjectKey,
        const RSAKeyPair& caKey);
    std::optional<std::string> buildSignedByCA(
        const ECKeyPair& subjectKey,
        const ECKeyPair& caKey);
    
    // Generate certificate in DER format
    std::optional<std::vector<uint8_t>> buildDer(const RSAKeyPair& keyPair);
    
    // Get certificate fingerprint
    static std::string getFingerprint(const std::string& pem);
    static std::string getFingerprint(const std::vector<uint8_t>& der);
};

// ============================================
// UTILITY FUNCTIONS
// ============================================

// Hex encoding/decoding
std::string bytesToHex(const uint8_t* data, size_t length);
std::string bytesToHex(const std::vector<uint8_t>& data);
std::vector<uint8_t> hexToBytes(const std::string& hex);

// Base64 encoding/decoding
std::string bytesToBase64(const uint8_t* data, size_t length);
std::string bytesToBase64(const std::string& data);
std::vector<uint8_t> base64ToBytes(const std::string& base64);

// Base64URL encoding/decoding (for JWT)
std::string bytesToBase64Url(const uint8_t* data, size_t length);
std::string bytesToBase64Url(const std::vector<uint8_t>& data);
std::vector<uint8_t> base64UrlToBytes(const std::string& base64Url);

// Secure memory operations
void secureZero(void* ptr, size_t len);
void secureZeroString(std::string& str);

// Get OpenSSL error string
std::string getOpenSSLError();
int getOpenSSLErrorCode();

// Initialize OpenSSL (call once at startup)
void initializeOpenSSL();
void cleanupOpenSSL();

} // namespace Crypto
} // namespace VirtualPhonePro
