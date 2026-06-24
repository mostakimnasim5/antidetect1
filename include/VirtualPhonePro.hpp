#pragma once

/**
 * VirtualPhonePro - Enterprise Android Virtualization System
 * 
 * A complete anti-detection platform for running unlimited Android VMs
 * with unique fingerprints on a single computer.
 * 
 * Features:
 * - Multi-instance Android virtualization
 * - Hardware-level emulation
 * - Complete anti-detection suite
 * - Network virtualization
 * - Realistic fingerprint generation
 * 
 * Version: 1.0.0
 */

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <optional>
#include <array>
#include <tuple>

namespace VirtualPhonePro {

// ============================================
// VERSION INFORMATION
// ============================================
constexpr const char* VERSION = "1.0.0";
constexpr const char* VERSION_STRING = "VirtualPhonePro v1.0.0";
constexpr int MAJOR_VERSION = 1;
constexpr int MINOR_VERSION = 0;
constexpr int PATCH_VERSION = 0;

// ============================================
// PLATFORM DETECTION
// ============================================
#if defined(_WIN32) || defined(_WIN64)
    #define VPP_PLATFORM_WINDOWS
    #define VPP_PLATFORM_NAME "Windows"
#elif defined(__linux__)
    #define VPP_PLATFORM_LINUX
    #define VPP_PLATFORM_NAME "Linux"
#elif defined(__APPLE__)
    #define VPP_PLATFORM_MACOS
    #define VPP_PLATFORM_NAME "macOS"
#else
    #define VPP_PLATFORM_UNKNOWN
    #define VPP_PLATFORM_NAME "Unknown"
#endif

// ============================================
// HARDWARE VIRTUALIZATION SUPPORT
// ============================================
#if defined(__x86_64__) || defined(_M_X64)
    #define VPP_ARCH_X86_64
    #define VPP_ARCH_NAME "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define VPP_ARCH_ARM64
    #define VPP_ARCH_NAME "ARM64"
#endif

// ============================================
// EXPORT MACROS
// ============================================
#if defined(_WIN32)
    #ifdef VPP_BUILD_LIBRARY
        #define VPP_EXPORT __declspec(dllexport)
    #else
        #define VPP_EXPORT __declspec(dllimport)
    #endif
#else
    #define VPP_EXPORT __attribute__((visibility("default")))
#endif

// ============================================
// COMMON RESULT TYPES
// ============================================
struct VMResult {
    bool success;
    std::string message;
    std::string error;
    
    static VMResult Ok(const std::string& msg = "Success") {
        return {true, msg, ""};
    }
    
    static VMResult Error(const std::string& err) {
        return {false, "", err};
    }
};

struct VMStatus {
    enum class State {
        STOPPED,
        STARTING,
        RUNNING,
        PAUSED,
        SUSPENDED,
        STOPPING,
        ERROR
    };
    
    State state;
    int cpuUsage;
    int memoryUsage;
    int memoryTotal;
    int memoryUsed;
    uint64_t uptime;
    std::string error;
};

struct VMInfo {
    std::string id;
    std::string name;
    std::string profileId;
    std::string osVersion;
    std::string state;
    int cpuCores;
    int memoryMB;
    int storageMB;
    std::string macAddress;
    std::string ipAddress;
    std::string fingerprint;
};

// ============================================
// CONFIGURATION STRUCTURES
// ============================================
struct VMConfig {
    std::string name;
    std::string profileId;
    int cpuCores = 4;
    int memoryMB = 4096;
    int storageMB = 16384;
    int displayWidth = 1080;
    int displayHeight = 2400;
    int displayDPI = 480;
    bool gpuAcceleration = true;
    bool soundEnabled = true;
    bool networkEnabled = true;
    std::string networkMode = "nat"; // nat, bridge, host-only
    std::string dnsServers = "8.8.8.8,8.8.4.4";
    bool enableVPN = false;
    std::string vpnProfile;
    std::map<std::string, std::string> customFingerprint;
};

struct FingerprintConfig {
    std::string manufacturer;      // Samsung, Google, Xiaomi, etc.
    std::string model;             // SM-G998B, Pixel 7, etc.
    std::string brand;
    std::string androidVersion;    // 11, 12, 13, 14
    std::string securityPatch;     // 2023-12-01
    std::string buildId;           // SP1A.201217P
    std::string bootloader;         // G998BXXU1
    std::string hardware;          // qcom, exynos
    std::string board;             // O1
    std::string device;            // foresse
    std::string product;           // o1s
    std::string fingerprint;        // Full fingerprint string
    
    // Network
    std::string macAddress;
    std::string carrier;
    std::string countryCode;
    
    // Hardware
    std::string cpuModel;
    std::string cpuABI;
    int cpuCores = 8;
    std::string gpuVendor;
    std::string gpuRenderer;
    int screenWidth = 1080;
    int screenHeight = 2400;
    int screenDPI = 480;
    std::string sensorModel;
    
    // Location
    double latitude = 23.8103;
    double longitude = 90.4125;
    std::string timezone = "Asia/Dhaka";
    std::string locale = "en_US";
};

struct NetworkConfig {
    std::string mode = "nat";           // nat, bridge, host-only
    std::string macAddress;
    std::string ipAddress;
    std::string subnet;
    std::string gateway;
    std::string dns1 = "8.8.8.8";
    std::string dns2 = "8.8.4.4";
    bool vpnEnabled = false;
    std::string vpnServer;
    int vpnPort = 443;
};

struct AntiDetectionConfig {
    bool enableSafetyNetBypass = true;
    bool enablePlayIntegrityBypass = true;
    bool hideVirtualization = true;
    bool enableTimingProtection = true;
    bool enableHardwareAttestation = true;
    bool spoofSELinuxState = true;
    std::string selinuxState = "enforcing";
    bool hideDebugFlags = true;
    bool hideTestKeys = true;
    std::string verifiedBootState = "green";
};

struct SecurityConfig {
    bool enableIsolation = true;
    bool encryptStorage = true;
    std::string encryptionAlgorithm = "AES-256-GCM";
    bool enableSecureBoot = true;
    bool enableAppSigning = true;
};

// ============================================
// DEVICE PROFILE DATABASE
// ============================================
struct DeviceProfile {
    std::string id;
    std::string manufacturer;
    std::string model;
    std::string codename;
    std::string series;
    std::string androidMin;
    std::string androidMax;
    
    // Build info
    std::string fingerprintTemplate;
    std::string bootloaderPattern;
    std::string buildTags;
    
    // Hardware specs
    std::string cpuModel;
    std::string gpuModel;
    int ramMB;
    int storageGB;
    int screenWidth;
    int screenHeight;
    int screenDPI;
    
    // Region availability
    std::vector<std::string> availableRegions;
    std::vector<std::string> carriers;
};

struct CarrierProfile {
    std::string id;
    std::string name;
    std::string country;
    std::string mcc;
    std::string mnc;
    std::vector<std::string> devices;
};

struct RegionProfile {
    std::string id;
    std::string name;
    std::string countryCode;
    std::string timezone;
    std::string locale;
    double latitude;
    double longitude;
    std::vector<std::string> availableCarriers;
};

// ============================================
// SNAPSHOT TYPES
// ============================================
struct Snapshot {
    std::string id;
    std::string name;
    std::string description;
    std::string vmId;
    uint64_t timestamp;
    uint64_t sizeBytes;
    std::string state; // running, stopped, paused
};

// ============================================
// ENUMERATIONS
// ============================================
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

enum class NetworkMode {
    NAT,
    BRIDGE,
    HOST_ONLY,
    INTERNAL
};

enum class AndroidVersion {
    ANDROID_11 = 30,
    ANDROID_12 = 31,
    ANDROID_12L = 32,
    ANDROID_13 = 33,
    ANDROID_14 = 34
};

// ============================================
// CALLBACK TYPES
// ============================================
using StatusCallback = std::function<void(const VMStatus&)>;
using LogCallback = std::function<void(LogLevel, const std::string&)>;
using ProgressCallback = std::function<void(int percentage, const std::string& message)>;

// ============================================
// UTILITY FUNCTIONS
// ============================================
std::string generateUUID();
std::string generateMAC();
std::string generateIP();
std::string hashString(const std::string& input);
std::string getCurrentTimestamp();
bool fileExists(const std::string& path);
bool createDirectory(const std::string& path);

} // namespace VirtualPhonePro
