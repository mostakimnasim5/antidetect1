#pragma once

/**
 * PlayIntegrityBypass - Google Play Integrity API Bypass
 * 
 * Provides complete Play Integrity response generation:
 * 
 * Integrity Tokens Contents:
 * - Device Integrity (CTS Profile Match)
 * - Basic Integrity
 * - Strong Integrity  
 * - MEETS_DEVICE_INTEGRITY
 * - MEETS_BASIC_INTEGRITY
 * - MEETS_STRONG_INTEGRITY
 * - MEETS_BASIC_DEVICE_INTEGRITY
 * 
 * nonce generation
 * 
 * Also bypasses:
 * - SafetyNet Attestation
 * - Play Services Availability
 * - BasicIntegrity (BI)
 * - DeviceIntegrity (DI)
 * - StrongIntegrity (SI)
 * - ClassicIntegrity (CI)
 * 
 * Protection Level: Enterprise
 */

#include "../VirtualPhonePro.hpp"
#include <vector>
#include <map>
#include <chrono>

namespace VirtualPhonePro {
namespace AntiDetect {

// Integrity verdict
enum class IntegrityVerdict {
    CTS_MATCH,
    CTS_PROFILE_MATCH,
    BASIC_INTEGRITY,
    STRONG_INTEGRITY,
    BASIC_DEVICE_INTEGRITY,
    MEETS_DEVICE_INTEGRITY,
    MEETS_BASIC_INTEGRITY,
    MEETS_STRONG_INTEGRITY,
    MEETS_BASIC_DEVICE_INTEGRITY,
    UNLICENSED
};

// Device integrity level
enum class DeviceIntegrityLevel {
    MEETS_DEVICE_INTEGRITY,      // Real device
    MEETS_BASIC_DEVICE_INTEGRITY, // emulator but with proper system
    DOES_NOT_MEET_DEVICE_INTEGRITY, // clearly VM
    INTEGRITY_STATUS_UNSPECIFIED
};

// App integrity
enum class AppIntegrity {
    PACKAGE_NAME_VERIFIED,
    SIGNATURE_VERIFIED,
    NO_MATCH
};

// Account integrity
enum class AccountIntegrity {
    ACCOUNT_ENABLED,
    ACCOUNT_DISABLED,
    NO_ACCOUNT
};

struct IntegrityResponse {
    bool success;
    std::string error;
    
    // Device integrity
    DeviceIntegrityLevel deviceIntegrity;
    std::string deviceEvaluationAge;
    std::vector<std::string> deviceIntegrityFlags;
    
    // Basic integrity
    bool basicIntegrity;
    std::vector<std::string> basicIntegrityLabels;
    
    // Strong integrity
    bool strongIntegrity;
    std::string integrityToken;
    
    // App integrity
    AppIntegrity appIntegrity;
    std::string packageName;
    std::string appVersionCode;
    
    // Account integrity
    AccountIntegrity accountIntegrity;
    
    // Request details
    std::string requestHash;
    std::string nonce;
    int64_t timestamp;
    
    // Device recognition
    std::string deviceRecognition;
    std::string device_cert_cpSig;
    
    // Play Protect
    bool playProtectEnabled;
    bool playProtectVerdict;
    
    // Debugging info
    bool isDebuggable;
    bool isEmulator;
    std::string evaluationType;
};

// SafetyNet Response (legacy)
struct SafetyNetResponse {
    bool isValidSignature;
    bool basicIntegrity;
    std::vector<std::string> advice;
    std::map<std::string, bool> evaluationType;
    std::string nonce;
    std::string timestamp;
    std::string apkPackageName;
    std::string apkDigestSha256;
    std::string deviceBrand;
    std::string deviceModel;
    std::string deviceManufacturer;
    std::string platform;
    int bootPatchLevel;
    int systemPatchLevel;
    std::string osVersion;
    std::string securityPatchLevel;
    std::string signature;
    std::string attestationResult;
};

class PlayIntegrityBypass {
public:
    static PlayIntegrityBypass& getInstance();
    
    // Initialize with device profile
    bool initialize(const std::string& deviceModel,
                   const std::string& androidVersion,
                   const std::string& securityPatch);
    
    // Generate complete integrity token
    IntegrityResponse generateIntegrityToken(const std::string& nonce,
                                           const std::string& packageName,
                                           const std::string& appVersion);
    
    // Generate SafetyNet attestation response
    SafetyNetResponse generateSafetyNetResponse(const std::string& nonce,
                                               const std::string& packageName,
                                               const std::vector<std::string>& packageCerts);
    
    // Check if device passes integrity
    bool passesIntegrity();
    bool passesBasicIntegrity();
    bool passesStrongIntegrity();
    
    // Generate nonce
    std::string generateNonce(const std::string& salt);
    
    // Configure integrity response
    void setDeviceIntegrity(DeviceIntegrityLevel level);
    void setBasicIntegrity(bool value);
    void setStrongIntegrity(bool value);
    
    // Enable/disable specific bypasses
    void enablePlayIntegrityBypass(bool enable);
    void enableSafetyNetBypass(bool enable);
    
    // Check if bypass is active
    bool isPlayIntegrityBypassActive();
    bool isSafetyNetBypassActive();
    
    // Set request hash
    void setRequestHash(const std::string& hash);
    
    // Get APK signing certificate digest
    void setAPKDigest(const std::string& digest);
    
    // Set Play Protect state
    void setPlayProtectEnabled(bool enabled);
    
    // Generate device recognition hex string
    std::string generateDeviceRecognition();
    
    // Debug info
    std::map<std::string, std::string> getDebugInfo();
    
private:
    PlayIntegrityBypass();
    ~PlayIntegrityBypass();
    PlayIntegrityBypass(const PlayIntegrityBypass&) = delete;
    PlayIntegrityBypass& operator=(const PlayIntegrityBypass&) = delete;
    
    // Device profile
    std::string m_deviceModel;
    std::string m_manufacturer;
    std::string m_brand;
    std::string m_androidVersion;
    std::string m_securityPatch;
    std::string m_buildId;
    
    // Integrity settings
    DeviceIntegrityLevel m_deviceIntegrity;
    bool m_basicIntegrity;
    bool m_strongIntegrity;
    bool m_playIntegrityBypassEnabled;
    bool m_safetyNetBypassEnabled;
    
    // APK info
    std::string m_packageName;
    std::string m_appVersion;
    std::string m_apkDigest;
    std::vector<std::string> m_signingCertDigests;
    
    // Request hash
    std::string m_requestHash;
    
    // Nonce history (to prevent replay)
    std::map<std::string, int64_t> m_nonceHistory;
    
    // Device recognition
    std::string m_deviceRecognition;
    
    // Play Protect
    bool m_playProtectEnabled;
    
    // Nonce generator
    std::string generateSecureNonce();
    
    // Build integrity token
    std::string buildIntegrityToken(const IntegrityResponse& response);
    
    // Sign integrity token
    std::vector<uint8_t> signIntegrityToken(const std::string& token);
    
    // Generate basic integrity labels
    std::vector<std::string> generateBasicIntegrityLabels();
    
    // Generate device integrity flags
    std::vector<std::string> generateDeviceIntegrityFlags();
    
    // Build SafetyNet response
    std::string buildSafetyNetResponse(const SafetyNetResponse& response);
    
    // Calculate APK digest from certificate
    std::string calculateAPKDigest(const std::string& certBase64);
    
    // Generate device recognition hex
    std::string buildDeviceRecognitionHex();
};

} // namespace AntiDetect
} // namespace VirtualPhonePro
