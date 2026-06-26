#pragma once

/**
 * VirtualSecurityChip - Emulates Hardware Security Module
 * 
 * Provides cryptographic operations that appear to come from
 * a real hardware security chip (TEE/TrustZone):
 * 
 * Features:
 * - Keymaster Attestation
 * - Hardware-bound keys
 * - Secure random number generation
 * - RSA/ECDSA signing
 * - Attestation keys and certificates
 * - Verified Boot keys
 * - Rollback protection
 * 
 * Works with PlayIntegrityBypass for full attestation chain.
 * 
 * Protection Level: Enterprise
 */

#include "../VirtualPhonePro.hpp"
#include <openssl/ec.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <map>

namespace VirtualPhonePro {
namespace AntiDetect {

// Attestation challenge result
struct AttestationResult {
    bool success;
    std::string error;
    std::vector<uint8_t> attestationChain;
    std::string formattedString;
    uint32_t verificationLevel;
};

// Key info from attestation
struct KeyInfo {
    std::string keyAlias;
    std::vector<uint8_t> publicKey;
    std::string algorithm;
    uint32_t keySize;
    bool hardwareBacked;
    std::string keyOrigin; // "GENERATED", "IMPORTED", "KEYSTORE"
};

// Verified boot state
enum class VerifiedBootState {
    GREEN,    // Device is verified
    YELLOW,   // Device has warnings
    ORANGE,   // Device is unlocked
    RED       // Device verification failed
};

// Boot state with verified boot parameters
struct BootState {
    VerifiedBootState state;
    std::string bootloaderLocked;
    std::string deviceLocked;
    std::string verifiedBootKeyHash;
    std::string bootSignature;
    std::string bootStateTimestamp;
    std::string buildFingerprint;
};

class VirtualSecurityChip {
public:
    static VirtualSecurityChip& getInstance();
    
    // Initialize with device profile
    bool initialize(const std::string& deviceModel);
    
    // Generate attestation key pair
    bool generateAttestationKey(const std::string& keyAlias,
                               const std::string& algorithm,
                               bool hardwareBacked = true);
    
    // Create attestation result for challenge
    AttestationResult createAttestation(const std::vector<uint8_t>& challenge,
                                        const std::string& keyAlias);
    
    // Verify attestation signature
    bool verifyAttestationSignature(const std::vector<uint8_t>& attestationResult,
                                   const std::vector<uint8_t>& certificateChain);
    
    // Get key information
    KeyInfo getKeyInfo(const std::string& keyAlias);
    
    // List all keys
    std::vector<std::string> listKeys();
    
    // Delete key
    bool deleteKey(const std::string& keyAlias);
    
    // Generate secure random
    std::vector<uint8_t> generateRandom(size_t length);
    
    // Sign data with key
    std::vector<uint8_t> sign(const std::string& keyAlias,
                             const std::vector<uint8_t>& data);
    
    // Verify signature
    bool verify(const std::string& keyAlias,
               const std::vector<uint8_t>& data,
               const std::vector<uint8_t>& signature);
    
    // Get verified boot state
    BootState getVerifiedBootState();
    
    // Set verified boot state
    void setVerifiedBootState(VerifiedBootState state);
    
    // Generate verified boot certificate
    std::vector<uint8_t> generateVerifiedBootCert();
    
    // Get device identity key (device-unique)
    std::vector<uint8_t> getDeviceIdentityKey();
    
    // Generate rollback protection nonce
    uint64_t generateRollbackNonce();
    
    // Set security patch level
    void setSecurityPatchLevel(const std::string& level);
    
    // Get security patch level
    std::string getSecurityPatchLevel();
    
    // Generate debug info
    std::string generateDebugInfo();
    
private:
    VirtualSecurityChip();
    ~VirtualSecurityChip();
    VirtualSecurityChip(const VirtualSecurityChip&) = delete;
    VirtualSecurityChip& operator=(const VirtualSecurityChip&) = delete;
    
    // Key storage
    struct StoredKey {
        std::string alias;
        std::vector<uint8_t> publicKey;
        std::vector<uint8_t> privateKey;
        std::string algorithm;
        uint32_t keySize;
        bool hardwareBacked;
        int64_t creationTime;
    };
    
    std::map<std::string, StoredKey> m_keys;
    
    // Attestation chain (root -> intermediate -> leaf)
    std::vector<std::vector<uint8_t>> m_attestationChain;
    
    // Verified boot state
    BootState m_bootState;
    
    // Device identity key (persistent)
    std::vector<uint8_t> m_deviceIdentityKey;
    
    // Security patch level
    std::string m_securityPatchLevel;
    
    // Rollback nonce
    uint64_t m_rollbackNonce;
    
    // Device info
    std::string m_deviceModel;
    std::string m_manufacturer;
    std::string m_brand;
    
    // Initialize attestation chain
    void initializeAttestationChain();
    
    // Generate self-signed certificate
    std::vector<uint8_t> generateSelfSignedCert(const std::string& cn,
                                                 EVP_PKEY* pkey);
    
    // Build attestation statement
    std::vector<uint8_t> buildAttestationStatement(
        const std::vector<uint8_t>& challenge,
        const std::vector<uint8_t>& keyCertificate,
        const std::vector<uint8_t>& signature);
    
    // Parse ASN.1 encoded attestation
    bool parseAttestationResult(const std::vector<uint8_t>& data,
                               AttestationResult& result);
    
    // Generate formatted attestation string
    std::string generateFormattedString(const AttestationResult& result);
    
    // Initialize device identity key
    void initializeDeviceIdentityKey();
    
    // Cryptographic helper functions
    std::vector<uint8_t> rsaSign(EVP_PKEY* key, const uint8_t* data, size_t len);
    std::vector<uint8_t> ecdsaSign(EVP_PKEY* key, const uint8_t* data, size_t len);
    bool verifySignature(EVP_PKEY* key, const uint8_t* data, size_t dlen,
                        const uint8_t* sig, size_t slen);
    
    // Generate unique device ID
    std::string generateDeviceUniqueID();
};

} // namespace AntiDetect
} // namespace VirtualPhonePro
