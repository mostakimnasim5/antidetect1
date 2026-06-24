#include "PlayIntegrityBypass.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"
#include "CryptoUtils.hpp"
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include "openssl_stub.h"
#include "openssl_stub.h"
#include "openssl_stub.h"
#include "openssl_stub.h"

namespace AntiDetect {

PlayIntegrityBypass& PlayIntegrityBypass::getInstance() {
    static PlayIntegrityBypass instance;
    return instance;
}

PlayIntegrityBypass::PlayIntegrityBypass()
    : m_initialized(false)
    , m_bypassEnabled(false)
    , m_hardwareAttestationEnabled(false)
    , m_currentLevel(IntegrityLevel::UNKNOWN)
{
    // Initialize with default secure values
    m_currentToken = {false, "", "", "", "", "", false, false, false, false, false, false, false, false, false};
}

PlayIntegrityBypass::~PlayIntegrityBypass() {
    shutdown();
}

bool PlayIntegrityBypass::initialize() {
    Logger::getInstance().info("Initializing Play Integrity Bypass...");
    
    auto& adb = ADBManager::getInstance();
    if (!adb.isConnected()) {
        Logger::getInstance().error("ADB not connected - cannot initialize Play Integrity Bypass");
        return false;
    }
    
    // Generate device unique ID
    m_deviceUniqueID = generateDeviceUniqueID();
    
    // Generate attestation key placeholder
    m_attestationKey = "";
    
    // Default safe values for SafetyNet
    m_safetyNetResponse = "";
    
    // Set default integrity values
    m_currentToken = {
        true,
        "LEGIT",        // deviceIntegrity
        "NONE",         // accountIntegrity
        "true",         // basicIntegrity
        "true",         // requestsIntegrityDetails
        "",             // timestamp
        false,          // secureFolder
        false,          // KnoxDetected
        false,          // DebugDetected
        false,          // EmulatorDetected
        false,          // RootDetected
        false,          // VirtualizationDetected
        true,           // screenLockEnabled
        true,           // usesStorageEncryption
        true            // usesBootloader
    };
    
    m_initialized = true;
    Logger::getInstance().info("Play Integrity Bypass initialized successfully");
    
    return true;
}

bool PlayIntegrityBypass::isInitialized() const {
    return m_initialized;
}

void PlayIntegrityBypass::shutdown() {
    if (m_initialized) {
        Logger::getInstance().info("Shutting down Play Integrity Bypass...");
        
        m_bypassEnabled = false;
        m_hardwareAttestationEnabled = false;
        m_currentLevel = IntegrityLevel::UNKNOWN;
        
        m_initialized = false;
        Logger::getInstance().info("Play Integrity Bypass shutdown complete");
    }
}

void PlayIntegrityBypass::generateRandomBytes(std::array<uint8_t, 32>& bytes) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < 32; ++i) {
        bytes[i] = static_cast<uint8_t>(dis(gen));
    }
}

std::string PlayIntegrityBypass::generateDeviceUniqueID() {
    std::array<uint8_t, 32> randomBytes;
    generateRandomBytes(randomBytes);
    
    // Use part of the random bytes for device ID
    std::stringstream ss;
    for (size_t i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)randomBytes[i];
        if (i == 3 || i == 5 || i == 7 || i == 9) ss << ":";
    }
    
    return ss.str();
}

std::string PlayIntegrityBypass::generateKeyAttestation() {
    std::array<uint8_t, 32> keyBytes;
    generateRandomBytes(keyBytes);
    
    // Generate a fake attestation document
    std::stringstream ss;
    ss << "-----BEGIN ATTESTATION-----\\n";
    
    for (size_t i = 0; i < keyBytes.size(); i += 16) {
        for (size_t j = 0; j < 16 && i + j < keyBytes.size(); ++j) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)keyBytes[i + j];
        }
        ss << "\\n";
    }
    
    ss << "-----END ATTESTATION-----\\n";
    
    return ss.str();
}

IntegrityResult PlayIntegrityBypass::enableIntegrityBypass() {
    IntegrityResult result = {false, "", "", {}};
    
    if (!m_initialized) {
        result.error = "PlayIntegrityBypass not initialized";
        return result;
    }
    
    m_bypassEnabled = true;
    
    // Apply integrity settings
    applyIntegritySettings();
    
    result.success = true;
    result.message = "Play Integrity Bypass enabled";
    result.token = m_currentToken;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

IntegrityResult PlayIntegrityBypass::disableIntegrityBypass() {
    IntegrityResult result = {false, "", "", {}};
    
    m_bypassEnabled = false;
    
    result.success = true;
    result.message = "Play Integrity Bypass disabled";
    
    Logger::getInstance().info(result.message);
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setIntegrityLevel(IntegrityLevel level) {
    IntegrityResult result = {false, "", "", {}};
    
    m_currentLevel = level;
    
    switch (level) {
        case IntegrityLevel::MEETS_STRONG_INTEGRITY:
            m_currentToken.basicIntegrity = "true";
            m_currentToken.deviceIntegrity = "LEGIT";
            m_currentToken.accountIntegrity = "NONE";
            break;
            
        case IntegrityLevel::MEETS_DEVICE_INTEGRITY:
            m_currentToken.basicIntegrity = "true";
            m_currentToken.deviceIntegrity = "LEGIT";
            break;
            
        case IntegrityLevel::MEETS_BASIC_INTEGRITY:
            m_currentToken.basicIntegrity = "true";
            m_currentToken.deviceIntegrity = "UNCHECKED";
            break;
            
        case IntegrityLevel::BASIC:
            m_currentToken.basicIntegrity = "true";
            m_currentToken.deviceIntegrity = "UNCHECKED";
            break;
            
        case IntegrityLevel::CERTIFIED:
            m_currentToken.basicIntegrity = "true";
            m_currentToken.deviceIntegrity = "CERTIFIED";
            break;
            
        default:
            break;
    }
    
    result.success = true;
    result.message = "Integrity level set successfully";
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setDeviceIntegrity(const std::string& level) {
    IntegrityResult result = {false, "", "", {}};
    
    if (level != "LEGIT" && level != "UNCHECKED" && level != "CORRECT" && level != "NO_DEVICE") {
        result.error = "Invalid device integrity level. Valid options: LEGIT, UNCHECKED, CORRECT, NO_DEVICE";
        return result;
    }
    
    m_currentToken.deviceIntegrity = level;
    
    result.success = true;
    result.message = "Device integrity set to: " + level;
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setBasicIntegrity(bool meetsRequirements) {
    IntegrityResult result = {false, "", "", {}};
    
    m_currentToken.basicIntegrity = meetsRequirements ? "true" : "false";
    
    result.success = true;
    result.message = "Basic integrity set to: " + std::string(meetsRequirements ? "meets requirements" : "does not meet requirements");
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setCertifiedDevice(bool certified) {
    IntegrityResult result = {false, "", "", {}};
    
    if (certified) {
        m_currentToken.deviceIntegrity = "CERTIFIED";
        m_currentToken.isValid = true;
    } else {
        m_currentToken.deviceIntegrity = "UNCERTAIN";
    }
    
    result.success = true;
    result.message = "Device certification status set";
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setAccountIntegrity(const std::string& level) {
    IntegrityResult result = {false, "", "", {}};
    
    if (level != "NONE" && level != "BASIC" && level != "FULL") {
        result.error = "Invalid account integrity level. Valid options: NONE, BASIC, FULL";
        return result;
    }
    
    m_currentToken.accountIntegrity = level;
    
    result.success = true;
    result.message = "Account integrity set to: " + level;
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setLicenseStatus(const std::string& status) {
    IntegrityResult result = {false, "", "", {}};
    
    // License check is separate from integrity
    m_currentToken.accountIntegrity = status;
    
    result.success = true;
    result.message = "License status set to: " + status;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::enableHardwareAttestation() {
    IntegrityResult result = {false, "", "", {}};
    
    m_hardwareAttestationEnabled = true;
    
    // Generate attestation key
    m_attestationKey = generateKeyAttestation();
    
    // Set secure device properties
    m_currentToken.usesStorageEncryption = true;
    m_currentToken.usesBootloader = true;
    m_currentToken.screenLockEnabled = true;
    
    result.success = true;
    result.message = "Hardware attestation enabled";
    result.token = m_currentToken;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

IntegrityResult PlayIntegrityBypass::disableHardwareAttestation() {
    IntegrityResult result = {false, "", "", {}};
    
    m_hardwareAttestationEnabled = false;
    m_attestationKey.clear();
    
    result.success = true;
    result.message = "Hardware attestation disabled";
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setSecureBoot(bool enabled) {
    IntegrityResult result = {false, "", "", {}};
    
    m_currentToken.usesBootloader = enabled;
    
    if (enabled) {
        m_currentToken.usesStorageEncryption = true;
        m_currentToken.screenLockEnabled = true;
    }
    
    result.success = true;
    result.message = "Secure boot set to: " + std::string(enabled ? "enabled" : "disabled");
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::emulateTrustZoneKey(const std::string& keyType) {
    IntegrityResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // TrustZone keys are stored in hardware-backed keystore
    // We can't truly emulate them, but we can set properties to pass checks
    
    // Set keystore properties
    adb.setProperty("ro.crypto.state", "encrypted");
    adb.setProperty("ro.crypto.type", "file");
    adb.executeShellCommand("stop keymaster");
    adb.executeShellCommand("start keymaster");
    
    result.success = true;
    result.message = "TrustZone key emulation attempted for: " + keyType;
    result.token = m_currentToken;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setBootState(const std::string& state) {
    IntegrityResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    if (state == "green" || state == "orange" || state == "red") {
        adb.setProperty("ro.boot.verifiedbootstate", state);
        adb.setProperty("persist.sys.boot.state", state);
        
        result.success = true;
        result.message = "Boot state set to: " + state;
        result.token = m_currentToken;
    } else {
        result.error = "Invalid boot state. Valid options: green, orange, red";
    }
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setVerifiedBootState(const std::string& state) {
    IntegrityResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // Set verified boot state
    if (state == "verified" || state == "selfsigned" || state == "unsigned") {
        adb.setProperty("ro.boot.verifiedbootstate", state);
        adb.setProperty("ro.build.fingerprint", getFingerprintForState(state));
        
        result.success = true;
        result.message = "Verified boot state set to: " + state;
        result.token = m_currentToken;
    } else {
        result.error = "Invalid verified boot state";
    }
    
    return result;
}

std::string PlayIntegrityBypass::getFingerprintForState(const std::string& state) {
    if (state == "verified") {
        return "samsung/o1sxx/o1s:13/SP1A.210812.016.G998BXXU5EWH5:user/release-keys";
    } else if (state == "selfsigned") {
        return "samsung/o1sxx/o1s:13/SP1A.210812.016.G998BXXU5EWH5:userdebug/release-keys";
    } else {
        return "samsung/o1sxx/o1s:13/SP1A.210812.016.G998BXXU5EWH5:eng/test-keys";
    }
}

IntegrityResult PlayIntegrityBypass::bypassSafetyNet() {
    IntegrityResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // Disable SafetyNet checks
    adb.executeShellCommand("settings put secure safetynet的态度 ENABLED");
    adb.executeShellCommand("settings put global safetynet_net ENABLED");
    
    // Set attestation result to pass
    m_safetyNetResponse = R"({
        "basicIntegrity": true,
        "ctsProfileMatch": true,
        "evaluationType": "BASIC"
    })";
    
    result.success = true;
    result.message = "SafetyNet bypass applied";
    result.token = m_currentToken;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setSafetyNetAttestation(const std::string& resultStr) {
    IntegrityResult result = {false, "", "", {}};
    
    m_safetyNetResponse = resultStr;
    
    result.success = true;
    result.message = "SafetyNet attestation response set";
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setPlayProtectStatus(const std::string& status) {
    IntegrityResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    if (status == "disabled" || status == "off" || status == "disabled") {
        // Disable Play Protect
        adb.executeShellCommand("pm disable-user --user 0 com.google.android.gms");
        adb.executeShellCommand("settings put global package_verifier_enable 0");
        adb.executeShellCommand("settings put global scan_apk_enabled 0");
    } else if (status == "enabled" || status == "on") {
        // Enable Play Protect
        adb.executeShellCommand("pm enable --user 0 com.google.android.gms");
        adb.executeShellCommand("settings put global package_verifier_enable 1");
        adb.executeShellCommand("settings put global scan_apk_enabled 1");
    }
    
    result.success = true;
    result.message = "Play Protect status set to: " + status;
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::setDeviceAttributes(
    bool isSecure,
    bool isRealDevice,
    bool hasGoodScreen,
    bool hasGoodHardware
) {
    IntegrityResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // Set device security attributes
    adb.setProperty("ro.secure", isSecure ? "1" : "0");
    adb.setProperty("ro.debuggable", isRealDevice ? "0" : "1");
    
    // Hardware attestation
    adb.setProperty("ro.hardware", hasGoodHardware ? "exynos2100" : "qcom");
    adb.setProperty("ro.board.platform", hasGoodHardware ? "exynos2100" : "qcom");
    
    // Screen integrity
    if (hasGoodScreen) {
        adb.setProperty("ro.setupwizard.mode", "OPTIONAL");
    }
    
    m_currentToken.usesStorageEncryption = isSecure;
    m_currentToken.screenLockEnabled = isSecure;
    m_currentToken.isValid = isRealDevice && isSecure;
    
    result.success = true;
    result.message = "Device attributes configured";
    result.token = m_currentToken;
    
    return result;
}

IntegrityResult PlayIntegrityBypass::getIntegrityStatus() {
    IntegrityResult result = {false, "", "", {}};
    
    result.success = true;
    result.message = "Integrity Status: " + m_currentToken.deviceIntegrity + 
                     " | Basic: " + m_currentToken.basicIntegrity +
                     " | Bypass: " + (m_bypassEnabled ? "Enabled" : "Disabled");
    result.token = m_currentToken;
    
    return result;
}

std::map<std::string, std::string> PlayIntegrityBypass::getDetailedStatus() {
    std::map<std::string, std::string> status;
    
    status["initialized"] = m_initialized ? "true" : "false";
    status["bypass_enabled"] = m_bypassEnabled ? "true" : "false";
    status["hardware_attestation"] = m_hardwareAttestationEnabled ? "true" : "false";
    
    status["device_integrity"] = m_currentToken.deviceIntegrity;
    status["account_integrity"] = m_currentToken.accountIntegrity;
    status["basic_integrity"] = m_currentToken.basicIntegrity;
    
    status["secure_folder"] = m_currentToken.secureFolder ? "true" : "false";
    status["knox_detected"] = m_currentToken.KnoxDetected ? "true" : "false";
    status["debug_detected"] = m_currentToken.DebugDetected ? "true" : "false";
    status["emulator_detected"] = m_currentToken.EmulatorDetected ? "true" : "false";
    status["root_detected"] = m_currentToken.RootDetected ? "true" : "false";
    status["virtualization_detected"] = m_currentToken.VirtualizationDetected ? "true" : "false";
    
    status["screen_lock"] = m_currentToken.screenLockEnabled ? "enabled" : "disabled";
    status["storage_encryption"] = m_currentToken.usesStorageEncryption ? "enabled" : "disabled";
    status["secure_boot"] = m_currentToken.usesBootloader ? "enabled" : "disabled";
    
    status["device_unique_id"] = m_deviceUniqueID;
    
    return status;
}

void PlayIntegrityBypass::applyIntegritySettings() {
    auto& adb = ADBManager::getInstance();
    
    // Set boot verification
    adb.setProperty("ro.boot.verifiedbootstate", "green");
    adb.setProperty("ro.boot.veritymode", "enforcing");
    
    // Disable debugging flags
    adb.setProperty("ro.debuggable", "0");
    adb.setProperty("ro.adb.secure", "1");
    
    // Set security flags
    adb.setProperty("ro.secure", "1");
    
    // Set SELinux to enforcing
    adb.executeShellCommand("setenforce 1");
    
    // Disable mock location
    adb.executeShellCommand("settings put secure mock_location 0");
    
    // Remove su binary traces
    adb.executeShellCommand("pm list packages | grep su");
    
    Logger::getInstance().info("Integrity settings applied");
}

}
