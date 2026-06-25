/**
 * SafetyNetEmulator - Google SafetyNet & Play Integrity API Response Emulation
 */

#include "anti_detect/SafetyNetEmulator.hpp"
#include "core/CryptoUtils.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace VirtualPhonePro {

SafetyNetEmulator::SafetyNetEmulator()
    : m_deviceLocked(true)
    , m_verifiedBootState("green")
    , m_debuggable(false)
    , m_betaApp(false)
    , m_androidVersion("13")
    , m_securityPatch("2024-01-01") {
}

SafetyNetEmulator::~SafetyNetEmulator() {}

SafetyNetEmulator& SafetyNetEmulator::getInstance() {
    static SafetyNetEmulator instance;
    return instance;
}

void SafetyNetEmulator::initialize(
    const std::string& manufacturer,
    const std::string& model,
    const std::string& brand,
    const std::string& androidVersion
) {
    m_manufacturer = manufacturer;
    m_model = model;
    m_brand = brand;
    m_androidVersion = androidVersion;
}

void SafetyNetEmulator::setDeviceLocked(bool locked) {
    m_deviceLocked = locked;
}

void SafetyNetEmulator::setVerifiedBootState(const std::string& state) {
    m_verifiedBootState = state;
}

void SafetyNetEmulator::setBetaApp(bool isBeta) {
    m_betaApp = isBeta;
}

void SafetyNetEmulator::setSecurityPatchLevel(const std::string& level) {
    m_securityPatch = level;
}

void SafetyNetEmulator::setDebuggable(bool debuggable) {
    m_debuggable = debuggable;
}

bool SafetyNetEmulator::isDebuggable() {
    return m_debuggable;
}

bool SafetyNetEmulator::isBasicIntegrity() {
    // Basic integrity passes if:
    // - Not debuggable
    // - Boot state is green
    // - No system rooting indicators
    return !m_debuggable && (m_verifiedBootState == "green");
}

bool SafetyNetEmulator::isCtsProfileMatch() {
    // CTS profile match requires:
    // - Basic integrity
    // - Device certified
    // - ROM matches GMS requirements
    return isBasicIntegrity() && m_deviceLocked;
}

bool SafetyNetEmulator::isDeviceIntegrity() {
    // Device integrity for Play Integrity API
    return m_verifiedBootState == "green" && m_deviceLocked;
}

std::string SafetyNetEmulator::generateSafetyNetResponse(
    const std::string& nonce,
    const std::string& packageName,
    const std::string& signatureDigest
) {
    std::string header = createJWSHeader("JWS");
    std::string payload = createSafetyNetPayload(nonce, packageName, signatureDigest);
    std::string signature = createJWSSignature(header, payload);
    
    return header + "." + payload + "." + signature;
}

std::string SafetyNetEmulator::generatePlayIntegrityResponse(
    const std::string& nonce,
    const std::string& packageName
) {
    std::string header = createJWSHeader("JWT"); // Play Integrity uses JWT
    std::string payload = createPlayIntegrityPayload(nonce, packageName);
    std::string signature = createJWSSignature(header, payload);
    
    return header + "." + payload + "." + signature;
}

std::string SafetyNetEmulator::createJWSHeader(const std::string& type) {
    std::stringstream headerJson;
    headerJson << "{";
    headerJson << "\"alg\":\"RS256\",";
    headerJson << "\"typ\":\"" << type << "\",";
    
    // For SafetyNet, use Android-specific key ID
    if (type == "JWS") {
        headerJson << "\"x5c\":[\"MIIDpDCCAoygAwIBAgIGAXQC\"],\"kid\":\"key_needed_for_google_signature\",";
        headerJson << "\"x5t\":\"fnVKBWVENHNTeFNT\"";
    } else {
        // Play Integrity
        headerJson << "\"x5c\":[\"MIIDpDCCAoygAwIBAgIGAXQD\"],\"kid\":\"google_play_integrity_key\",";
        headerJson << "\"x5t\":\"fnVKBWVENHNTeFNT\"";
    }
    
    headerJson << "}";
    
    return base64UrlEncode(headerJson.str());
}

std::string SafetyNetEmulator::createSafetyNetPayload(
    const std::string& nonce,
    const std::string& packageName,
    const std::string& signatureDigest
) {
    std::stringstream payloadJson;
    
    // Device integrity evaluation
    auto eval = evaluateDeviceIntegrity();
    
    payloadJson << "{";
    payloadJson << "\"nonce\":\"" << nonce << "\",";
    payloadJson << "\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count() << ",";
    payloadJson << "\"packageName\":\"" << packageName << "\",";
    payloadJson << "\"apkPackageName\":\"" << packageName << "\",";
    payloadJson << "\"apkCertificateDigestSha256\":[\"" << signatureDigest << "\"],";
    payloadJson << "\"deviceIntegrity\":{\"deviceRecognitionInfo\":[" << eval["deviceRecognitionInfo"] << "],";
    payloadJson << "\"lastBootTime\":" << eval["lastBootTime"] << "},";
    payloadJson << "\"basicIntegrity\":" << (isBasicIntegrity() ? "true" : "false") << ",";
    payloadJson << "\"ctsProfileMatch\":" << (isCtsProfileMatch() ? "true" : "false") << ",";
    
    // Device info
    payloadJson << "\"advice\":\"" << eval["advice"] << "\",";
    payloadJson << "\"evaluationType\":[\"BASIC\",\"CTS\"],\"extracted\":{";
    payloadJson << "\"branding\":\"CTS\",";
    payloadJson << "\"certifiedDevice\":true,";
    payloadJson << "\"system\":{";
    payloadJson << "\"manufacturer\":\"" << m_manufacturer << "\",";
    payloadJson << "\"model\":\"" << m_model << "\",";
    payloadJson << "\"brand\":\"" << m_brand << "\",";
    payloadJson << "\"device\":\"" << m_model << "\",";
    payloadJson << "\"product\":\"" << m_model << "\",";
    payloadJson << "\"board\":\"" << m_model << "\",";
    payloadJson << "\"hardware\":\"" << m_manufacturer << "\",";
    payloadJson << "\"androidVersion\":\"" << m_androidVersion << "\",";
    payloadJson << "\"securityPatch\":\"" << m_securityPatch << "\",";
    payloadJson << "\"bootloader\":\"" << m_model << "\",";
    payloadJson << "\"buildId\":\"" << m_model << "\",";
    payloadJson << "\"buildFingerprint\":\"" << m_manufacturer << "/" << m_model << "/" << m_model << ":\"";
    payloadJson << "}},\"meekBook\":{\"advice\":[],\"normalized\":[{\"osVersion\":18,\"patchMonth\":" << 
        (m_androidVersion >= "13" ? "202401" : "202301") << "}],\"errors\":[]}";
    
    payloadJson << "}";
    return base64UrlEncode(payloadJson.str());
}

std::string SafetyNetEmulator::createPlayIntegrityPayload(
    const std::string& nonce,
    const std::string& packageName
) {
    std::stringstream payloadJson;
    
    payloadJson << "{";
    payloadJson << "\"nonce\":\"" << nonce << "\",";
    payloadJson << "\"requestDetails\":{";
    payloadJson << "\"requestPackageName\":\"" << packageName << "\",";
    payloadJson << "\"timestampMillis\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count() << ",";
    payloadJson << "\"appIntegrity\":{";
    payloadJson << "\"packageName\":\"" << packageName << "\",";
    payloadJson << "\"certificateHash\":{\"sha256\":[\"valid_digest_here\"]},";
    payloadJson << "\"appRecognitionFlags\":0},";
    payloadJson << "\"deviceIntegrity\":{";
    payloadJson << "\"deviceRecognitionVerdict\":[\"MEETS_DEVICE_INTEGRITY\"],";
    payloadJson << "\"deviceAttributes\":{";
    payloadJson << "\"manufacturer\":\"" << m_manufacturer << "\",";
    payloadJson << "\"model\":\"" << m_model << "\",";
    payloadJson << "\"brand\":\"" << m_brand << "\"}";
    payloadJson << "},\"accountDetails\":{";
    payloadJson << "\"appLicensingVerdict\":\"LICENSED\"}";
    payloadJson << "}";
    
    return base64UrlEncode(payloadJson.str());
}

std::map<std::string, std::string> SafetyNetEmulator::evaluateDeviceIntegrity() {
    std::map<std::string, std::string> result;
    Crypto::SecureRandomGenerator rng;
    
    // Device recognition info (what the device looks like)
    std::stringstream deviceRecog;
    deviceRecog << "{\"" << m_manufacturer << "/" << m_model << "/" << m_model << ":" 
               << m_androidVersion << "/" << m_securityPatch << "/" << m_model << "\":\""
               << Crypto::SecureRandomGenerator().generateHexString(32) << "\"}";
    result["deviceRecognitionInfo"] = deviceRecog.str();
    
    // Last boot time (in milliseconds since epoch)
    result["lastBootTime"] = std::to_string(
        (std::chrono::system_clock::now().time_since_epoch().count() / 1000) - 
        (rng.generateUint64() % 86400000) // Up to 1 day ago
    );
    
    // Advice (empty if no issues)
    if (isBasicIntegrity()) {
        result["advice"] = "";
    } else if (!m_deviceLocked) {
        result["advice"] = "RESTORE_TO_FACTORY_STATE";
    } else if (m_verifiedBootState != "green") {
        result["advice"] = "UNSUPPORTED_BOOT_STATE";
    } else {
        result["advice"] = "";
    }
    
    return result;
}

std::string SafetyNetEmulator::createJWSSignature(const std::string& header, const std::string& payload) {
    // Create fake signature - in real implementation, this would be RSA-signed
    // For now, we generate a convincing-looking base64 string
    std::string dataToSign = header + "." + payload;
    
    // SHA256 hash as fake signature (real would be RSA-signed)
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(dataToSign.c_str()), dataToSign.length(), hash);
    
    // Sign with fake key (in real implementation, this would use Google's key)
    unsigned char signature[256];
    memcpy(signature, hash, SHA256_DIGEST_LENGTH);
    memset(signature + SHA256_DIGEST_LENGTH, 0, 256 - SHA256_DIGEST_LENGTH);
    
    std::string sig(reinterpret_cast<char*>(signature), 256);
    return base64UrlEncode(sig);
}

std::string SafetyNetEmulator::base64UrlEncode(const std::string& data) {
    static const char* base64Chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    int i = 0;
    int j = 0;
    unsigned char charArray3[3];
    unsigned char charArray4[4];
    int padding = 0;
    
    for (char c : data) {
        charArray3[i++] = c;
        if (i == 3) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;
            
            for (int k = 0; k < 4; k++) {
                result += base64Chars[charArray4[k]];
            }
            i = 0;
        }
    }
    
    if (i > 0) {
        for (int k = i; k < 3; k++) charArray3[k] = '\0';
        
        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        
        for (int k = 0; k < i + 1; k++) {
            result += base64Chars[charArray4[k]];
        }
        while ((i++ < 3) && padding++ < 2) {
            result += '=';
        }
    }
    
    // Convert to URL-safe base64
    std::replace(result.begin(), result.end(), '+', '-');
    std::replace(result.begin(), result.end(), '/', '_');
    
    return result;
}

std::string SafetyNetEmulator::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return std::to_string(ms.count());
}

std::string SafetyNetEmulator::generateSecureRandomId(int length) {
    return Crypto::SecureRandomGenerator().generateHexString(length);
}

std::map<std::string, std::string> SafetyNetEmulator::generateAllAttestationData(
    const std::string& nonce,
    const std::string& packageName
) {
    std::map<std::string, std::string> data;
    
    // Generate SafetyNet response
    data["safenety_response"] = generateSafetyNetResponse(
        nonce, packageName, "valid_signature_digest"
    );
    
    // Generate Play Integrity response
    data["play_integrity_response"] = generatePlayIntegrityResponse(nonce, packageName);
    
    // Add integrity flags
    data["basic_integrity"] = isBasicIntegrity() ? "true" : "false";
    data["cts_profile_match"] = isCtsProfileMatch() ? "true" : "false";
    data["device_integrity"] = isDeviceIntegrity() ? "MEETS_DEVICE_INTEGRITY" : "UNMET_DEVICE_INTEGRITY";
    
    // Device info for verification
    data["manufacturer"] = m_manufacturer;
    data["model"] = m_model;
    data["brand"] = m_brand;
    data["android_version"] = m_androidVersion;
    data["security_patch"] = m_securityPatch;
    data["verified_boot_state"] = m_verifiedBootState;
    
    return data;
}

} // namespace VirtualPhonePro
