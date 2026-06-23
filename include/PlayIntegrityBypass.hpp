#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <array>
#include <mutex>

namespace AntiDetect {

struct IntegrityToken {
    bool isValid;
    std::string deviceIntegrity;
    std::string accountIntegrity;
    std::string basicIntegrity;
    std::string requestsIntegrityDetails;
    std::string timestamp;
    
    // Detailed integrity signals
    bool secureFolder;
    bool KnoxDetected;
    bool DebugDetected;
    bool EmulatorDetected;
    bool RootDetected;
    bool VirtualizationDetected;
    bool screenLockEnabled;
    bool usesStorageEncryption;
    bool usesBootloader;
};

enum class IntegrityLevel {
    UNKNOWN,
    BASIC,
    CERTIFIED,
    MEETS_BASIC_INTEGRITY,
    MEETS_DEVICE_INTEGRITY,
    MEETS_STRONG_INTEGRITY,
    MEETS_VIRTUALIZED_INTEGRITY
};

struct IntegrityResult {
    bool success;
    std::string message;
    std::string error;
    IntegrityToken token;
};

class PlayIntegrityBypass {
public:
    static PlayIntegrityBypass& getInstance();
    
    PlayIntegrityBypass();
    ~PlayIntegrityBypass();
    
    bool initialize();
    bool isInitialized() const;
    void shutdown();
    
    // Play Integrity API Bypass
    IntegrityResult enableIntegrityBypass();
    IntegrityResult disableIntegrityBypass();
    IntegrityResult setIntegrityLevel(IntegrityLevel level);
    
    // Device Integrity
    IntegrityResult setDeviceIntegrity(const std::string& level);
    IntegrityResult setBasicIntegrity(bool meetsRequirements);
    IntegrityResult setCertifiedDevice(bool certified);
    
    // Account Integrity
    IntegrityResult setAccountIntegrity(const std::string& level);
    IntegrityResult setLicenseStatus(const std::string& status);
    
    // Hardware-backed Security
    IntegrityResult enableHardwareAttestation();
    IntegrityResult disableHardwareAttestation();
    IntegrityResult setSecureBoot(bool enabled);
    
    // TrustZone Key Emulation
    IntegrityResult emulateTrustZoneKey(const std::string& keyType);
    IntegrityResult setBootState(const std::string& state);
    IntegrityResult setVerifiedBootState(const std::string& state);
    
    // SafetyNet/Play Protect
    IntegrityResult bypassSafetyNet();
    IntegrityResult setSafetyNetAttestation(const std::string& result);
    IntegrityResult setPlayProtectStatus(const std::string& status);
    
    // Device Properties for Integrity
    IntegrityResult setDeviceAttributes(
        bool isSecure,
        bool isRealDevice,
        bool hasGoodScreen,
        bool hasGoodHardware
    );
    
    // Status
    IntegrityResult getIntegrityStatus();
    std::map<std::string, std::string> getDetailedStatus();
    
private:
    bool generateIntegrityResponse();
    std::string generateDeviceUniqueID();
    std::string generateKeyAttestation();
    
    void applyIntegritySettings();
    void generateRandomBytes(std::array<uint8_t, 32>& bytes);
    
    bool m_initialized;
    bool m_bypassEnabled;
    bool m_hardwareAttestationEnabled;
    
    IntegrityLevel m_currentLevel;
    IntegrityToken m_currentToken;
    
    std::string m_deviceUniqueID;
    std::string m_attestationKey;
    std::string m_safetyNetResponse;
    
    std::mutex m_mutex;
};

}
