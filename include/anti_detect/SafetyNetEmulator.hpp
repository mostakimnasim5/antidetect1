#pragma once

/**
 * SafetyNetEmulator - Google SafetyNet & Play Integrity API Response Emulation
 * 
 * Generates JWS (JSON Web Signature) responses that are indistinguishable
 * from real SafetyNet/Play Integrity API responses.
 * 
 * IMPORTANT: This is software emulation only. Real attestation requires:
 * - Hardware-bound keys (StrongBox/TEE)
 * - Verified boot chain
 * - Google-signed certificates
 * 
 * This module helps with:
 * - Basic Integrity checks (most important)
 * - Device Integrity (if not using hardware attestation)
 * - CTS Profile Match
 */

#include "../VirtualPhonePro.hpp"
#include <map>
#include <string>
#include <vector>
#include <chrono>

namespace VirtualPhonePro {

// Attestation types
enum class AttestationType {
    SAFETY_NET,
    PLAY_INTEGRITY
};

// Integrity verdicts
enum class IntegrityVerdict {
    PASS,
    FAIL,
    UNCERTAIN
};

class SafetyNetEmulator {
public:
    static SafetyNetEmulator& getInstance();
    
    // Initialize with device profile
    void initialize(
        const std::string& manufacturer,
        const std::string& model,
        const std::string& brand,
        const std::string& androidVersion
    );
    
    // Generate SafetyNet attestation response (JWS format)
    std::string generateSafetyNetResponse(
        const std::string& nonce,
        const std::string& packageName,
        const std::string& signatureDigest
    );
    
    // Generate Play Integrity API response
    std::string generatePlayIntegrityResponse(
        const std::string& nonce,
        const std::string& packageName
    );
    
    // Get basic integrity result
    bool isBasicIntegrity();
    bool isCtsProfileMatch();
    bool isDeviceIntegrity();
    
    // Set device locked state
    void setDeviceLocked(bool locked);
    void setVerifiedBootState(const std::string& state);
    void setBetaApp(bool isBeta);
    
    // Set security patch level
    void setSecurityPatchLevel(const std::string& level);
    
    // Get debug status
    void setDebuggable(bool debuggable);
    bool isDebuggable();
    
    // Generate all attestation data at once
    std::map<std::string, std::string> generateAllAttestationData(
        const std::string& nonce,
        const std::string& packageName
    );
    
private:
    SafetyNetEmulator();
    ~SafetyNetEmulator();
    
    // JWS generation
    std::string createJWSHeader(const std::string& type);
    std::string createJWSPayload(const std::map<std::string, std::string>& claims);
    std::string createJWSSignature(const std::string& header, const std::string& payload);
    
    // SafetyNet specific
    std::string createSafetyNetPayload(
        const std::string& nonce,
        const std::string& packageName,
        const std::string& signatureDigest
    );
    
    // Play Integrity specific
    std::string createPlayIntegrityPayload(
        const std::string& nonce,
        const std::string& packageName
    );
    
    // Device evaluation
    std::map<std::string, std::string> evaluateDeviceIntegrity();
    
    // Helpers
    std::string base64UrlEncode(const std::string& data);
    std::string getCurrentTimestamp();
    std::string generateSecureRandomId(int length);
    
    // Device info
    std::string m_manufacturer;
    std::string m_model;
    std::string m_brand;
    std::string m_androidVersion;
    std::string m_securityPatch;
    
    // Integrity state
    bool m_deviceLocked;
    std::string m_verifiedBootState;
    bool m_debuggable;
    bool m_betaApp;
    
    // Cached evaluation
    std::map<std::string, bool> m_evaluationCache;
};

} // namespace VirtualPhonePro
