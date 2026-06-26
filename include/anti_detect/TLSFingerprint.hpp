#pragma once

/**
 * TLSFingerprint - Advanced TLS/SSL Fingerprinting Prevention
 * 
 * Implements JA3/JA4 TLS fingerprinting bypass to make network
 * traffic appear as if it's coming from a real Android device.
 * 
 * Features:
 * - JA3 hash generation and spoofing
 * - JA4 fingerprint generation
 * - TLS 1.3 support
 * - Android-specific cipher suites
 * - SNI spoofing
 * - Certificate chain spoofing
 * 
 * Protection Level: Enterprise
 */

#include "../VirtualPhonePro.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <vector>
#include <map>

namespace VirtualPhonePro {
namespace AntiDetect {

// TLS Extension types
enum class TLSExtension {
    SERVER_NAME = 0,           // SNI
    MAX_FRAGMENT_LENGTH = 1,
    CLIENT_CERTIFICATE_URL = 2,
    TRUNCATED_HMAC = 4,
    STATUS_REQUEST = 5,        // OCSP stapling
    EllipticCurves = 10,      // Supported groups
    EcPointFormats = 11,
    SIGNATURE_ALGORITHMS = 13,
    USE_SRTP = 14,
    APPLICATION_LAYER_PROTOCOL = 16,
    SIGNED_CERTIFICATE_TIMESTAMP = 18,
    PADDING = 21,
    SESSION_TICKET = 35,
    TLS_1_3_COMPAT_MODE = 39,
    PSK_KEY_EXCHANGE_MODES = 41,
    RECORD_SIZE_LIMIT = 28,
    QUIC_TRANSPORT_PARAMETERS = 57,
    ENCRYPTED_CLIENT_HELLO = 65037
};

// OS-specific TLS configurations
struct OSTLSConfig {
    std::string osName;
    std::string browser;
    std::string version;
    
    // TLS Version
    uint16_t maxVersion;
    uint16_t minVersion;
    
    // Cipher suites (in preference order)
    std::vector<uint16_t> cipherSuites;
    
    // Extensions to include
    std::vector<TLSExtension> extensions;
    
    // Elliptic curves (supported groups)
    std::vector<uint16_t> ellipticCurves;
    
    // EC point formats
    std::vector<uint8_t> ecPointFormats;
    
    // Signature algorithms
    std::vector<uint16_t> signatureAlgorithms;
    
    // JA3 string components
    std::string ja3Components;
};

class TLSFingerprint {
public:
    static TLSFingerprint& getInstance();
    
    // Initialize with device profile
    bool initialize(const std::string& deviceModel);
    
    // JA3 Hash Generation
    std::string generateJA3Hash(const std::vector<uint16_t>& cipherSuites,
                               const std::vector<uint8_t>& extensions,
                               const std::vector<uint8_t>& ellipticCurves,
                               uint16_t tlsVersion);
    
    // JA4 Fingerprint Generation
    std::string generateJA4Fingerprint(uint16_t tlsVersion,
                                      const std::vector<uint16_t>& cipherSuites,
                                      const std::vector<uint8_t>& extensions);
    
    // Get device-specific TLS config
    OSTLSConfig getTLSConfig();
    
    // Set active device profile
    void setDeviceProfile(const std::string& profile);
    
    // SSL Context creation with fingerprint
    SSL_CTX* createSSLContext();
    
    // Configure existing SSL for fingerprinting
    bool configureSSL(SSL* ssl);
    
    // Get available OS profiles
    std::vector<std::string> getAvailableProfiles();
    
    // Generate SNI (Server Name Indication) for device
    std::string generateSNI(const std::string& domain);
    
    // Validate fingerprint matches expected OS
    bool validateFingerprint(const std::string& ja3Hash, 
                            const std::string& expectedOS);
    
    // Generate random but valid TLS behavior
    uint16_t getRandomTLSVersion();
    uint16_t getRandomCipherSuite();
    
private:
    TLSFingerprint();
    ~TLSFingerprint();
    TLSFingerprint(const TLSFingerprint&) = delete;
    TLSFingerprint& operator=(const TLSFingerprint&) = delete;
    
    // Current profile
    std::string m_currentProfile;
    
    // OS-specific configs
    std::map<std::string, OSTLSConfig> m_osProfiles;
    
    // Initialize predefined profiles
    void initializeOSProfiles();
    
    // Build cipher suite list for JA3
    std::string buildCipherSuiteString(const std::vector<uint16_t>& suites);
    
    // Build extensions string for JA3
    std::string buildExtensionsString(const std::vector<uint8_t>& exts);
    
    // Calculate MD5 hash (JA3 uses MD5)
    std::string md5(const std::string& input);
    
    // TLS 1.3 cipher suites
    std::vector<uint16_t> getTLS13CipherSuites();
    
    // TLS 1.2 cipher suites
    std::vector<uint16_t> getTLS12CipherSuites();
    
    // Android-specific ALPN protocols
    std::vector<std::string> getAndroidALPN();
    
    // Chrome OS specific
    std::vector<std::string> getChromeOSALPN();
    
    // Normalize cipher preference order
    std::vector<uint16_t> shuffleCipherOrder(const std::vector<uint16_t>& base,
                                             uint32_t seed);
};

} // namespace AntiDetect
} // namespace VirtualPhonePro
