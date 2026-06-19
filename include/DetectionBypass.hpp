#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace AntiDetect {

// Google Play Integrity API Categories
struct PlayIntegrityResult {
    bool basicIntegrity;
    bool deviceIntegrity;
    bool measurementsMatch;
    bool noIntegrityTools;
    bool notRecognizedDevice;
    bool notSecure;
    bool notSupported;
    bool roleEntitlement;
    std::string nonce;
    std::string requestHash;
    std::string timestamp;
};

// SafetyNet Attestation Response
struct SafetyNetResult {
    bool basicIntegrity;
    bool ctsProfileMatch;
    bool evaluationTypeBasic;
    bool evaluationTypeHardwareBacked;
    std::string apkPackageName;
    std::string apkDigestSha256;
    std::string timestamp;
    std::string advice;
};

// Device Integrity Levels
enum class IntegrityLevel {
    NONE = 0,
    BASIC = 1,
    HARDWARE = 2,
    PLAY_SERVICES = 3,
    CTS_MATCH = 4,
    FULL = 5
};

// Detection Categories
enum class DetectionCategory {
    EMULATOR,
    ROOT,
    HOOK,
    VIRTUALIZATION,
    INTEGRITY,
    ENVIRONMENT,
    TIMING,
    NETWORK,
    DEVICE_INFO
};

struct DetectionCheck {
    DetectionCategory category;
    std::string name;
    std::string property;
    std::string expectedValue;
    bool isCritical;
    std::string bypassMethod;
};

// Comprehensive Detection Bypass System
class DetectionBypass {
public:
    static DetectionBypass& getInstance();
    
    bool initialize();
    bool shutdown();
    
    // ========== GOOGLE PLAY INTEGRITY BYPASS ==========
    
    // Play Integrity API Response
    bool setPlayIntegrityResult(bool basicIntegrity, bool deviceIntegrity, 
                               bool measurementsMatch, bool noIntegrityTools = true);
    bool setDeviceIntegrity(bool isSecure, bool isRecognized);
    bool setRoleEntitlement(bool hasPlayRole);
    std::string generateIntegrityToken(const std::string& nonce);
    
    // Required Actions for Play Integrity
    bool configurePlayIntegrityResponse();
    bool enablePlayIntegrityApi();
    bool setIntegrityMalikwareCheck(bool pass);
    bool setIntegrityOtherReasons(const std::vector<std::string>& reasons);
    
    // ========== SAFETYNET BYPASS ==========
    
    bool setSafetyNetResult(bool basicIntegrity, bool ctsMatch, 
                          bool evaluationTypeHardware, bool evaluationTypeBasic);
    bool setSafetyNetAdvice(const std::string& advice);
    bool setApkPackageInfo(const std::string& packageName, const std::string& digestSha256);
    bool setSafetyNetTimestamp(const std::string& timestamp);
    
    // SafetyNet API Configuration
    bool enableSafetyNetApi();
    bool configureSafetyNetResponse();
    bool setSafetyNetNonce(const std::string& nonce);
    
    // ========== DEVICE ATTESTATION BYPASS ==========
    
    // Hardware Attestation
    bool enableHardwareAttestation();
    bool setKeyAttestation(bool isAttested);
    bool setStrongBoxAttestation(bool isStrongBox);
    bool setTEEAttestation(bool isTEE);
    bool setSecureHardware(bool isSecureHardware);
    
    // Keystore
    bool enableAndroidKeystore();
    bool setKeymasterVersion(int version);
    bool setSecurityLevel(int level);
    bool setAttestationKey(bool isAttested);
    
    // TEE/TrustZone
    bool enableTrustZone();
    bool setTEEPresent(bool present);
    bool setTZVersion(const std::string& version);
    bool setTZBuild(const std::string& build);
    
    // ========== GMS (GOOGLE MOBILE SERVICES) BYPASS ==========
    
    bool enableGMSCore();
    bool setGMSVersion(const std::string& version);
    bool setPlayServicesVersion(const std::string& version);
    bool setPlayCoreVersion(const std::string& version);
    bool configureGMSPackages();
    
    // GMS Detection Hiding
    bool hideGMSPackages();
    bool hideGooglePlayStore();
    bool hidePlayServices();
    bool hideGsfAndLogin();
    bool configureGmsAsSystemApp();
    
    // ========== APP INTEGRITY BYPASS ==========
    
    // Package Verification
    bool disablePackageVerification();
    bool setPackageInstaller(const std::string& installer);
    bool hideInstallerPackage();
    bool setInstallerSource(int source);
    
    // Signature Verification
    bool patchSignatureCheck();
    bool setSignatureHash(const std::string& hash);
    bool enableDebuggable(bool enabled);
    
    // ========== DEVICE INFO HARDENING ==========
    
    // All Build Properties
    bool setAllBuildProperties(const std::string& brand, const std::string& model,
                              const std::string& manufacturer, const std::string& device);
    bool setBuildFingerprint(const std::string& fingerprint);
    bool setBuildDescription(const std::string& description);
    bool setBuildTags(const std::string& tags);
    bool setBuildType(const std::string& type);
    bool setBuildUser(const std::string& user);
    bool setBuildHost(const std::string& host);
    bool setBuildId(const std::string& id);
    
    // Product Properties
    bool setAllProductProperties();
    bool setProductName(const std::string& name);
    bool setProductBrand(const std::string& brand);
    bool setProductDevice(const std::string& device);
    bool setProductBoard(const std::string& board);
    bool setProductHardware(const std::string& hardware);
    bool setProductManufacturer(const std::string& manufacturer);
    bool setProductModel(const std::string& model);
    bool setProductCountry(const std::string& country);
    bool setProductLocale(const std::string& locale);
    bool setProductLanguage(const std::string& language);
    
    // Vendor Properties
    bool setAllVendorProperties();
    bool setVendorBuildFingerprint(const std::string& fingerprint);
    bool setVendorBrand(const std::string& brand);
    bool setVendorDevice(const std::string& device);
    
    // ========== EMULATOR DETECTION BYPASS ==========
    
    // QEMU/KVM Detection
    bool bypassQEMUDetection();
    bool bypassKVMDetection();
    bool setKernelQEMU(bool isQEMU);
    bool setQEMUDrivers();
    bool hideQEMUFiles();
    bool hideKVMFiles();
    bool setGoldfishDetection();
    bool setGoldfishAudio();
    bool setPipeFD();
    
    // Genymotion Detection
    bool bypassGenymotionDetection();
    bool setGenymotionModel(const std::string& model);
    bool setGenymotionDevice(const std::string& device);
    bool hideGenymotionApps();
    bool hideGenymotionProperties();
    
    // BlueStacks Detection
    bool bypassBlueStacksDetection();
    bool setBlueStacksModel(const std::string& model);
    bool setBlueStacksDevice(const std::string& device);
    bool setBlueStacksManufacturer();
    bool hideBlueStacksApps();
    bool hideBlueStacksProps();
    bool hideBSTweaks();
    
    // Nox/ldplayer Detection
    bool bypassNoxDetection();
    bool bypassLDPlayerDetection();
    bool hideNoxApps();
    bool hideLDPlayerApps();
    bool setNoxModel();
    bool setLDPlayerModel();
    
    // VirtualBox Detection
    bool bypassVirtualBoxDetection();
    bool hideVirtualBoxFiles();
    bool hideVMWareFiles();
    
    // ========== ROOT DETECTION BYPASS ==========
    
    bool bypassRootDetection();
    bool hideSuBinary();
    bool hideMagisk();
    bool hideXposed();
    bool hideFrida();
    bool hideSubstrate();
    bool hideBusyBox();
    bool hideRootManagementApps();
    bool configureDenylist();
    bool hideZygisk();
    bool hideMagiskHide();
    bool hideSystemProps();
    bool hideKapAndAmal();
    
    // ========== HOOK DETECTION BYPASS ==========
    
    bool bypassHookDetection();
    bool hideInlineHooks();
    bool hidePLTHooks();
    bool hideFunctionHooks();
    bool patchStrstr();
    bool patchDlsym();
    bool patchRead();
    bool patchOpen();
    
    // ========== DEBUG DETECTION BYPASS ==========
    
    bool bypassDebugDetection();
    bool hideDebugFlags();
    bool disableDebuggable();
    bool hideTracerPid();
    bool disabledeveloperOptions();
    bool hideDevOptionsMenu();
    
    // ========== NETWORK DETECTION BYPASS ==========
    
    bool bypassNetworkDetection();
    bool setVPNDetection(bool isVPN);
    bool setProxyDetection(bool isProxy);
    bool setTorDetection(bool isTor);
    bool hideVPNApps();
    bool configureNetworkProxy();
    
    // ========== TIMING DETECTION BYPASS ==========
    
    bool bypassTimingDetection();
    bool addCpuLatency();
    bool setRealBootTime();
    bool hideUptime();
    bool setRealTimeSinceBoot();
    
    // ========== SENSOR DETECTION BYPASS ==========
    
    bool bypassSensorDetection();
    bool setRealAccelerometer();
    bool setRealGyroscope();
    bool setRealMagnetometer();
    bool hideSensorManipulation();
    bool setRealLightSensor();
    bool setRealProximitySensor();
    
    // ========== FILE SYSTEM DETECTION BYPASS ==========
    
    bool bypassFileSystemDetection();
    bool hideTestKeys();
    bool hideCustomROM();
    bool hideSUAccess();
    bool hideRootFiles();
    bool hideMagiskFiles();
    bool hideXposedFiles();
    
    // ========== PERMISSION DETECTION BYPASS ==========
    
    bool bypassPermissionDetection();
    bool setRealPermissions();
    bool hideDangerousPermissions();
    bool setDangerousPermissionState(bool granted);
    
    // ========== PLAY STORE VERIFICATION ==========
    
    bool bypassPlayStoreVerification();
    bool setPlayStoreLicensing(bool isLicensed);
    bool configurePlayStoreResponse();
    bool setLicenseCheckResult(int result);
    
    // ========== COMPLETE APPLICATION ==========
    
    bool applyFullBypass();
    bool applyUltimateHardening();
    bool resetAllBypasses();
    
    // ========== STATUS ==========
    
    std::map<std::string, bool> getBypassStatus();
    std::vector<std::string> getCriticalFailures();
    bool verifyDeviceIsReal();
    std::string generateVerificationReport();

private:
    DetectionBypass();
    ~DetectionBypass();
    
    DetectionBypass(const DetectionBypass&) = delete;
    DetectionBypass& operator=(const DetectionBypass&) = delete;
    
    bool applyProperty(const std::string& key, const std::string& value);
    std::string getProperty(const std::string& key);
    bool executeShell(const std::string& command);
    
    std::map<std::string, bool> m_bypassStatus;
    std::vector<std::string> m_appliedBypasses;
    
    PlayIntegrityResult m_playIntegrity;
    SafetyNetResult m_safetyNet;
    
    bool m_initialized;
    bool m_fullBypassApplied;
};

}
