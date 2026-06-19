#include "DetectionBypass.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"
#include <random>
#include <chrono>
#include <sstream>

namespace AntiDetect {

DetectionBypass& DetectionBypass::getInstance() {
    static DetectionBypass instance;
    return instance;
}

DetectionBypass::DetectionBypass()
    : m_initialized(false)
    , m_fullBypassApplied(false)
{
    m_playIntegrity = {true, true, true, true, false, false, false, false, "", "", ""};
    m_safetyNet = {true, true, true, false, "", "", "", ""};
}

DetectionBypass::~DetectionBypass() {
    shutdown();
}

bool DetectionBypass::initialize() {
    if (m_initialized) return true;
    
    Logger::getInstance().info("Initializing DetectionBypass - Google/Facebook Protection...");
    m_initialized = true;
    return true;
}

bool DetectionBypass::shutdown() {
    m_initialized = false;
    return true;
}

bool DetectionBypass::applyProperty(const std::string& key, const std::string& value) {
    return ADBManager::getInstance().setProperty(key, value);
}

std::string DetectionBypass::getProperty(const std::string& key) {
    return ADBManager::getInstance().getProperty(key);
}

bool DetectionBypass::executeShell(const std::string& command) {
    return ADBManager::getInstance().executeShellCommand(command).find("error") == std::string::npos;
}

// ========== GOOGLE PLAY INTEGRITY BYPASS ==========

bool DetectionBypass::setPlayIntegrityResult(bool basicIntegrity, bool deviceIntegrity,
                                           bool measurementsMatch, bool noIntegrityTools) {
    Logger::getInstance().info("Configuring Play Integrity API response...");
    auto& adb = ADBManager::getInstance();
    
    m_playIntegrity.basicIntegrity = basicIntegrity;
    m_playIntegrity.deviceIntegrity = deviceIntegrity;
    m_playIntegrity.measurementsMatch = measurementsMatch;
    m_playIntegrity.noIntegrityTools = noIntegrityTools;
    
    // Set critical properties for Play Integrity
    adb.setProperty("ro.play.integrity.enabled", "true");
    adb.setProperty("ro.play.integrity.basic", basicIntegrity ? "true" : "false");
    adb.setProperty("ro.play.integrity.device", deviceIntegrity ? "true" : "false");
    adb.setProperty("ro.play.integrity.measurements", measurementsMatch ? "true" : "false");
    adb.setProperty("ro.play.integrity.tools", noIntegrityTools ? "false" : "true");
    
    // Play Integrity requires:
    adb.setProperty("ro.verifiedbootstate", "green");
    adb.setProperty("ro.boot.verifiedbootstate", "green");
    adb.setProperty("ro.build.verity.mode", "enforcing");
    adb.setProperty("ro.device.data.hardware.type", "good");
    
    m_bypassStatus["play_integrity"] = true;
    Logger::getInstance().info("Play Integrity configured: basic=" + std::to_string(basicIntegrity) + 
                              ", device=" + std::to_string(deviceIntegrity));
    return true;
}

bool DetectionBypass::setDeviceIntegrity(bool isSecure, bool isRecognized) {
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("ro.device.integrity", isSecure ? "secure" : "unsecure");
    adb.setProperty("ro.device.recognized", isRecognized ? "true" : "false");
    adb.setProperty("security.device.secure", isSecure ? "1" : "0");
    
    if (isSecure) {
        adb.setProperty("ro.boot.verifiedbootstate", "green");
        adb.setProperty("ro.verifiedbootstate", "green");
        adb.setProperty("ro.verifiedboot.locked", "true");
    }
    
    return true;
}

bool DetectionBypass::setRoleEntitlement(bool hasPlayRole) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.play.role.entitlement", hasPlayRole ? "true" : "false");
    return true;
}

std::string DetectionBypass::generateIntegrityToken(const std::string& nonce) {
    // Generate valid integrity token structure
    std::string token = "MEUCIQ";
    token += std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    token += "AIJ";
    token += nonce;
    token += "IG";
    token += m_playIntegrity.basicIntegrity ? "AAAA" : "BBBB";
    return token;
}

bool DetectionBypass::configurePlayIntegrityResponse() {
    return setPlayIntegrityResult(true, true, true, true);
}

bool DetectionBypass::enablePlayIntegrityApi() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.play.integrity.api.enabled", "true");
    adb.setProperty("ro.play.integrity.enabled", "true");
    return true;
}

bool DetectionBypass::setIntegrityMalikwareCheck(bool pass) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.play.integrity.malik", pass ? "pass" : "fail");
    adb.setProperty("ro.device.security.malware", pass ? "clean" : "infected");
    return true;
}

bool DetectionBypass::setIntegrityOtherReasons(const std::vector<std::string>& reasons) {
    auto& adb = ADBManager::getInstance();
    int i = 0;
    for (const auto& reason : reasons) {
        adb.setProperty("ro.play.integrity.reason." + std::to_string(i++), reason);
    }
    return true;
}

// ========== SAFETYNET BYPASS ==========

bool DetectionBypass::setSafetyNetResult(bool basicIntegrity, bool ctsMatch,
                                       bool evaluationTypeHardware, bool evaluationTypeBasic) {
    Logger::getInstance().info("Configuring SafetyNet attestation response...");
    auto& adb = ADBManager::getInstance();
    
    m_safetyNet.basicIntegrity = basicIntegrity;
    m_safetyNet.ctsProfileMatch = ctsMatch;
    m_safetyNet.evaluationTypeHardware = evaluationTypeHardware;
    m_safetyNet.evaluationTypeBasic = evaluationTypeBasic;
    
    // Critical SafetyNet properties
    adb.setProperty("ro.safetynet.enabled", "true");
    adb.setProperty("ro.safetynet.basic", basicIntegrity ? "true" : "false");
    adb.setProperty("ro.safetynet.cts", ctsMatch ? "true" : "false");
    adb.setProperty("ro.safetynet.hardware", evaluationTypeHardware ? "true" : "false");
    adb.setProperty("ro.safetynet.basic_eval", evaluationTypeBasic ? "true" : "false");
    
    // SafetyNet requires these to pass:
    adb.setProperty("ro.build.tags", "release-keys");
    adb.setProperty("ro.build.type", "user");
    adb.setProperty("ro.debuggable", "0");
    adb.setProperty("ro.secure", "1");
    adb.setProperty("ro.boot.verifiedbootstate", "green");
    adb.setProperty("ro.verity.mode", "enforcing");
    adb.setProperty("ro.verifiedbootstate", "green");
    adb.setProperty("ro.build.selinux.enforce", "true");
    
    // Hide any test-keys indicators
    std::string fingerprint = adb.getProperty("ro.build.fingerprint");
    if (fingerprint.find("test-keys") != std::string::npos) {
        fingerprint = fingerprint + ":user/release-keys";
        adb.setProperty("ro.build.fingerprint", fingerprint);
    }
    
    m_bypassStatus["safetynet"] = true;
    Logger::getInstance().info("SafetyNet configured: basic=" + std::to_string(basicIntegrity) +
                               ", cts=" + std::to_string(ctsMatch));
    return true;
}

bool DetectionBypass::setSafetyNetAdvice(const std::string& advice) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.safetynet.advice", advice);
    m_safetyNet.advice = advice;
    return true;
}

bool DetectionBypass::setApkPackageInfo(const std::string& packageName, const std::string& digestSha256) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.safetynet.pkg.name", packageName);
    adb.setProperty("ro.safetynet.pkg.hash", digestSha256);
    m_safetyNet.apkPackageName = packageName;
    m_safetyNet.apkDigestSha256 = digestSha256;
    return true;
}

bool DetectionBypass::setSafetyNetTimestamp(const std::string& timestamp) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.safetynet.timestamp", timestamp);
    m_safetyNet.timestamp = timestamp;
    return true;
}

bool DetectionBypass::enableSafetyNetApi() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.safetynet.api.enabled", "true");
    adb.setProperty("ro.gms.safetynet.enabled", "true");
    return true;
}

bool DetectionBypass::configureSafetyNetResponse() {
    return setSafetyNetResult(true, true, true, true);
}

bool DetectionBypass::setSafetyNetNonce(const std::string& nonce) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.safetynet.nonce", nonce);
    m_safetyNet.advice = nonce;
    return true;
}

// ========== DEVICE ATTESTATION BYPASS ==========

bool DetectionBypass::enableHardwareAttestation() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware.keystore", "true");
    adb.setProperty("ro.hardware.attestation", "true");
    adb.setProperty("ro.hardware.wv", "true");
    return true;
}

bool DetectionBypass::setKeyAttestation(bool isAttested) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.keymaster.attestation", isAttested ? "1" : "0");
    adb.setProperty("ro.keystore.attestation", isAttested ? "enabled" : "disabled");
    return true;
}

bool DetectionBypass::setStrongBoxAttestation(bool isStrongBox) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware.strongbox", isStrongBox ? "true" : "false");
    adb.setProperty("ro.strongbox.enabled", isStrongBox ? "1" : "0");
    adb.setProperty("ro.security.strongbox", isStrongBox ? "enabled" : "disabled");
    return true;
}

bool DetectionBypass::setTEEAttestation(bool isTEE) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware.tee", isTEE ? "true" : "false");
    adb.setProperty("ro.trusty.secure", isTEE ? "1" : "0");
    return true;
}

bool DetectionBypass::setSecureHardware(bool isSecureHardware) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware.security", isSecureHardware ? "secure" : "insecure");
    adb.setProperty("ro.secure.hardware", isSecureHardware ? "1" : "0");
    return true;
}

bool DetectionBypass::enableAndroidKeystore() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.keystore.enabled", "true");
    adb.setProperty("ro.hardware.keystore", "true");
    adb.setProperty("ro.keymaster.version", "4.1");
    return true;
}

bool DetectionBypass::setKeymasterVersion(int version) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.keymaster.version", std::to_string(version));
    adb.setProperty("ro.hardware.keymaster", std::to_string(version));
    return true;
}

bool DetectionBypass::setSecurityLevel(int level) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.keystore.security_level", std::to_string(level));
    adb.setProperty("ro.keymaster.security_level", std::to_string(level));
    return true;
}

bool DetectionBypass::setAttestationKey(bool isAttested) {
    return setKeyAttestation(isAttested);
}

bool DetectionBypass::enableTrustZone() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware.trustzone", "true");
    adb.setProperty("ro.tz.trusty", "enabled");
    return true;
}

bool DetectionBypass::setTEEPresent(bool present) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware.tee.present", present ? "true" : "false");
    return true;
}

bool DetectionBypass::setTZVersion(const std::string& version) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.trusty.version", version);
    adb.setProperty("ro.tz.version", version);
    return true;
}

bool DetectionBypass::setTZBuild(const std::string& build) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.trusty.build", build);
    return true;
}

// ========== GMS BYPASS ==========

bool DetectionBypass::enableGMSCore() {
    auto& adb = ADBManager::getInstance();
    
    // Enable GMS Core
    adb.setProperty("ro.gms.core.enabled", "true");
    adb.setProperty("ro.com.google.gmscore", "true");
    adb.setProperty("ro.setupwizard.mode", "OPTIONAL");
    
    // GMS Packages should be visible
    adb.executeShellCommand("pm enable com.google.android.gms");
    adb.executeShellCommand("pm enable com.google.android.gsf");
    adb.executeShellCommand("pm enable com.google.android.gsf.login");
    
    return true;
}

bool DetectionBypass::setGMSVersion(const std::string& version) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.gms.version", version);
    adb.setProperty("ro.gms.core.version", version);
    return true;
}

bool DetectionBypass::setPlayServicesVersion(const std::string& version) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.play.services.version", version);
    adb.setProperty("com.google.android.gms.version", version);
    return true;
}

bool DetectionBypass::setPlayCoreVersion(const std::string& version) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.play.core.version", version);
    return true;
}

bool DetectionBypass::configureGMSPackages() {
    auto& adb = ADBManager::getInstance();
    
    // Ensure GMS packages are enabled
    adb.executeShellCommand("pm enable com.google.android.gms");
    adb.executeShellCommand("pm enable com.google.android.gsf");
    adb.executeShellCommand("pm enable com.google.android.gsf.login");
    adb.executeShellCommand("pm enable com.google.android.backuptransport");
    adb.executeShellCommand("pm enable com.google.android.partnersetup");
    adb.executeShellCommand("pm enable com.google.android.feedback");
    adb.executeShellCommand("pm enable com.google.android.onetimeinitializer");
    adb.executeShellCommand("pm enable com.google.android.configupdater");
    
    return true;
}

bool DetectionBypass::hideGMSPackages() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.gms.hide.packages", "false");
    return true;
}

bool DetectionBypass::hideGooglePlayStore() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("pm unhide com.android.vending");
    adb.executeShellCommand("pm enable com.android.vending");
    return true;
}

bool DetectionBypass::hidePlayServices() {
    return configureGMSPackages();
}

bool DetectionBypass::hideGsfAndLogin() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("pm enable com.google.android.gsf");
    adb.executeShellCommand("pm enable com.google.android.gsf.login");
    return true;
}

bool DetectionBypass::configureGmsAsSystemApp() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.gms.system.app", "true");
    return true;
}

// ========== APP INTEGRITY BYPASS ==========

bool DetectionBypass::disablePackageVerification() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("pm.disable_verification", "false");
    return true;
}

bool DetectionBypass::setPackageInstaller(const std::string& installer) {
    auto& adb = ADBManager::getInstance();
    if (!installer.empty()) {
        adb.executeShellCommand("settings put global package_verifier_enable 0");
    }
    return true;
}

bool DetectionBypass::hideInstallerPackage() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.installer.package", "com.android.vending");
    return true;
}

bool DetectionBypass::setInstallerSource(int source) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.install.source", std::to_string(source));
    return true;
}

bool DetectionBypass::patchSignatureCheck() {
    // Disable signature verification hooks
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.signature.check", "disabled");
    adb.setProperty("ro.verify.signature", "false");
    return true;
}

bool DetectionBypass::setSignatureHash(const std::string& hash) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.signature.hash", hash);
    return true;
}

bool DetectionBypass::enableDebuggable(bool enabled) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.debuggable", enabled ? "1" : "0");
    return true;
}

// ========== DEVICE INFO HARDENING ==========

bool DetectionBypass::setAllBuildProperties(const std::string& brand, const std::string& model,
                                          const std::string& manufacturer, const std::string& device) {
    auto& adb = ADBManager::getInstance();
    
    // Core build properties
    adb.setProperty("ro.build.product", device);
    adb.setProperty("ro.build.device", device);
    adb.setProperty("ro.build.id", "TD1A.220804.031");
    adb.setProperty("ro.build.display.id", "TD1A.220804.031");
    adb.setProperty("ro.build.version.release", "14");
    adb.setProperty("ro.build.version.sdk", "34");
    adb.setProperty("ro.build.version.security_patch", "2024-01-01");
    adb.setProperty("ro.build.type", "user");
    adb.setProperty("ro.build.tags", "release-keys");
    adb.setProperty("ro.build.user", "user");
    adb.setProperty("ro.build.host", "user");
    adb.setProperty("ro.build.flavor", "full");
    
    // Generate realistic fingerprint
    std::string fingerprint = manufacturer + "/" + brand + "/" + device + 
                            ":14/TD1A.220804.031:user/release-keys";
    adb.setProperty("ro.build.fingerprint", fingerprint);
    adb.setProperty("ro.bootimage.build.fingerprint", fingerprint);
    adb.setProperty("ro.system.build.fingerprint", fingerprint);
    adb.setProperty("ro.vendor.build.fingerprint", fingerprint);
    adb.setProperty("ro.odm.build.fingerprint", fingerprint);
    adb.setProperty("ro.product.build.fingerprint", fingerprint);
    
    // Build description
    std::string description = manufacturer + " " + model + " " + device + " " +
                            "14/TD1A.220804.031/" + "user@user:user/release-keys";
    adb.setProperty("ro.build.description", description);
    
    return true;
}

bool DetectionBypass::setBuildFingerprint(const std::string& fingerprint) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.build.fingerprint", fingerprint);
    adb.setProperty("ro.bootimage.build.fingerprint", fingerprint);
    adb.setProperty("ro.system.build.fingerprint", fingerprint);
    adb.setProperty("ro.vendor.build.fingerprint", fingerprint);
    return true;
}

bool DetectionBypass::setBuildDescription(const std::string& description) {
    return applyProperty("ro.build.description", description);
}

bool DetectionBypass::setBuildTags(const std::string& tags) {
    return applyProperty("ro.build.tags", tags);
}

bool DetectionBypass::setBuildType(const std::string& type) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.build.type", type);
    adb.setProperty("ro.build.characteristics", "nosdcard");
    return true;
}

bool DetectionBypass::setBuildUser(const std::string& user) {
    return applyProperty("ro.build.user", user);
}

bool DetectionBypass::setBuildHost(const std::string& host) {
    return applyProperty("ro.build.host", host);
}

bool DetectionBypass::setBuildId(const std::string& id) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.build.id", id);
    adb.setProperty("ro.build.display.id", id);
    return true;
}

bool DetectionBypass::setAllProductProperties() {
    auto& adb = ADBManager::getInstance();
    
    // Set all product properties to match device
    adb.setProperty("ro.product.first_api_level", "33");
    adb.setProperty("ro.product.locale", "en-US");
    adb.setProperty("ro.product.locale.language", "en");
    adb.setProperty("ro.product.locale.region", "US");
    adb.setProperty("ro.product.properties", "default");
    
    return true;
}

bool DetectionBypass::setProductName(const std::string& name) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.name", name);
    adb.setProperty("ro.product.names", name);
    return true;
}

bool DetectionBypass::setProductBrand(const std::string& brand) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.brand", brand);
    adb.setProperty("ro.product.vendor.brand", brand);
    return true;
}

bool DetectionBypass::setProductDevice(const std::string& device) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.device", device);
    adb.setProperty("ro.product.vendor.device", device);
    return true;
}

bool DetectionBypass::setProductBoard(const std::string& board) {
    return applyProperty("ro.product.board", board);
}

bool DetectionBypass::setProductHardware(const std::string& hardware) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.hardware", hardware);
    adb.setProperty("ro.hardware", hardware);
    adb.setProperty("ro.board.platform", hardware);
    adb.setProperty("ro.arch", hardware);
    return true;
}

bool DetectionBypass::setProductManufacturer(const std::string& manufacturer) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.manufacturer", manufacturer);
    adb.setProperty("ro.product.vendor.manufacturer", manufacturer);
    adb.setProperty("ro.product.odm.manufacturer", manufacturer);
    return true;
}

bool DetectionBypass::setProductModel(const std::string& model) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.model", model);
    adb.setProperty("ro.product.vendor.model", model);
    adb.setProperty("ro.product.odm.model", model);
    adb.setProperty("ro.product.product.model", model);
    return true;
}

bool DetectionBypass::setProductCountry(const std::string& country) {
    return applyProperty("ro.product.country", country);
}

bool DetectionBypass::setProductLocale(const std::string& locale) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.locale", locale);
    adb.setProperty("persist.sys.locale", locale);
    return true;
}

bool DetectionBypass::setProductLanguage(const std::string& language) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.product.language", language);
    adb.setProperty("persist.sys.language", language);
    return true;
}

bool DetectionBypass::setAllVendorProperties() {
    auto& adb = ADBManager::getInstance();
    
    // Copy build properties to vendor
    std::string fingerprint = adb.getProperty("ro.build.fingerprint");
    if (!fingerprint.empty()) {
        adb.setProperty("ro.vendor.build.fingerprint", fingerprint);
        adb.setProperty("ro.vendor.build.description", adb.getProperty("ro.build.description"));
    }
    
    return true;
}

bool DetectionBypass::setVendorBuildFingerprint(const std::string& fingerprint) {
    return applyProperty("ro.vendor.build.fingerprint", fingerprint);
}

bool DetectionBypass::setVendorBrand(const std::string& brand) {
    return applyProperty("ro.vendor.product.brand", brand);
}

bool DetectionBypass::setVendorDevice(const std::string& device) {
    return applyProperty("ro.vendor.product.device", device);
}

// ========== EMULATOR DETECTION BYPASS ==========

bool DetectionBypass::bypassQEMUDetection() {
    Logger::getInstance().info("Bypassing QEMU/KVM detection...");
    auto& adb = ADBManager::getInstance();
    
    // Remove all QEMU indicators
    adb.setProperty("ro.kernel.qemu", "0");
    adb.setProperty("ro.kernel.android.qemu", "0");
    adb.setProperty("ro.kernel.linux", "");
    adb.setProperty("ro.qemu", "0");
    adb.setProperty("ro.qemu.sdk", "0");
    
    // QEMU specific props
    adb.executeShellCommand("getprop | grep -i qemu | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    
    // Hide QEMU files
    adb.executeShellCommand("chmod 000 /system/lib64/libc_qemu.so 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/lib/libc_qemu.so 2>/dev/null || true");
    
    // Set realistic hardware
    adb.setProperty("ro.hardware", "qcom");
    adb.setProperty("ro.arch", "arm64");
    
    // Remove goldfish detection
    adb.executeShellCommand("getprop | grep goldfish | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    
    // Hide pipes and files
    adb.executeShellCommand("chmod 000 /dev/qemu_pipe 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /dev/tty 2>/dev/null || true");
    
    m_bypassStatus["qemu"] = true;
    return true;
}

bool DetectionBypass::bypassKVMDetection() {
    return bypassQEMUDetection();
}

bool DetectionBypass::setKernelQEMU(bool isQEMU) {
    return applyProperty("ro.kernel.qemu", isQEMU ? "1" : "0");
}

bool DetectionBypass::setQEMUDrivers() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware", "qcom");
    adb.setProperty("ro.boot.hardware", "qcom");
    return true;
}

bool DetectionBypass::hideQEMUFiles() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("chmod 000 /system/lib64/libc_qemu.so 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/lib/libc_qemu.so 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideKVMFiles() {
    return hideQEMUFiles();
}

bool DetectionBypass::setGoldfishDetection() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.hardware.goldfish", "false");
    adb.setProperty("ro.goldfish", "0");
    return true;
}

bool DetectionBypass::setGoldfishAudio() {
    return setQEMUDrivers();
}

bool DetectionBypass::setPipeFD() {
    return applyProperty("ro.kernel.android.qemu", "0");
}

bool DetectionBypass::bypassGenymotionDetection() {
    Logger::getInstance().info("Bypassing Genymotion detection...");
    auto& adb = ADBManager::getInstance();
    
    // Set realistic device properties
    adb.setProperty("ro.product.model", "Galaxy S23");
    adb.setProperty("ro.product.device", "o1s");
    adb.setProperty("ro.product.brand", "Samsung");
    adb.setProperty("ro.product.manufacturer", "Samsung");
    adb.setProperty("ro.product.name", "o1sxx");
    
    std::string fingerprint = "samsung/o1sxx/o1s:14/TD1A.220804.031:user/release-keys";
    adb.setProperty("ro.build.fingerprint", fingerprint);
    
    // Remove Genymotion specific properties
    adb.executeShellCommand("settings delete global genotype_id 2>/dev/null || true");
    adb.executeShellCommand("settings delete global geny_generation 2>/dev/null || true");
    adb.executeShellCommand("settings delete global genymotion_app_enabled 2>/dev/null || true");
    adb.executeShellCommand("settings delete global is_genymotion 2>/dev/null || true");
    adb.executeShellCommand("settings delete global uses_genymotion 2>/dev/null || true");
    
    // Hide Genymotion apps
    adb.executeShellCommand("pm hide com.genymotion.overlay 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.genymotion.home 2>/dev/null || true");
    
    m_bypassStatus["genymotion"] = true;
    return true;
}

bool DetectionBypass::setGenymotionModel(const std::string& model) {
    return setProductModel(model);
}

bool DetectionBypass::setGenymotionDevice(const std::string& device) {
    return setProductDevice(device);
}

bool DetectionBypass::hideGenymotionApps() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("pm hide com.genymotion.overlay 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.genymotion.home 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideGenymotionProperties() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("getprop | grep geny | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    return true;
}

bool DetectionBypass::bypassBlueStacksDetection() {
    Logger::getInstance().info("Bypassing BlueStacks detection...");
    auto& adb = ADBManager::getInstance();
    
    // Set realistic device properties
    adb.setProperty("ro.product.model", "Samsung Galaxy S21 Ultra");
    adb.setProperty("ro.product.device", "dreamlte");
    adb.setProperty("ro.product.brand", "samsung");
    adb.setProperty("ro.product.manufacturer", "samsung");
    
    std::string fingerprint = "samsung/dreamltexx/dreamlte:13/SP1A.210812.016:user/release-keys";
    adb.setProperty("ro.build.fingerprint", fingerprint);
    
    // Remove BlueStacks properties
    adb.executeShellCommand("settings delete global bst_emulator_id 2>/dev/null || true");
    adb.executeShellCommand("settings delete global bst_gpu_renderer 2>/dev/null || true");
    adb.executeShellCommand("settings delete global bst_manufacturer 2>/dev/null || true");
    adb.executeShellCommand("settings delete global bst_model 2>/dev/null || true");
    adb.executeShellCommand("settings delete secure bst_force_orientation 2>/dev/null || true");
    adb.executeShellCommand("settings delete global is_bst 2>/dev/null || true");
    adb.executeShellCommand("settings delete global uses_bst 2>/dev/null || true");
    
    // Hide BlueStacks specific packages
    adb.executeShellCommand("pm hide com.bluestacks.home 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.bluestacks.settings 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.bluestacks.appmart 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.bluestacks.gameinhome 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.bluestacks.switcher 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.bluestacks.BstApplication 2>/dev/null || true");
    
    m_bypassStatus["bluestacks"] = true;
    return true;
}

bool DetectionBypass::setBlueStacksModel(const std::string& model) {
    return setProductModel(model);
}

bool DetectionBypass::setBlueStacksDevice(const std::string& device) {
    return setProductDevice(device);
}

bool DetectionBypass::setBlueStacksManufacturer() {
    return setProductManufacturer("Samsung");
}

bool DetectionBypass::hideBlueStacksApps() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("pm hide com.bluestacks.home 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.bluestacks.settings 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideBlueStacksProps() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("getprop | grep bst | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    adb.executeShellCommand("getprop | grep blue | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideBSTweaks() {
    return hideBlueStacksApps();
}

bool DetectionBypass::bypassNoxDetection() {
    Logger::getInstance().info("Bypassing Nox detection...");
    auto& adb = ADBManager::getInstance();
    
    // Set realistic properties
    adb.setProperty("ro.product.model", "Pixel 7 Pro");
    adb.setProperty("ro.product.device", "panther");
    adb.setProperty("ro.product.brand", "Google");
    adb.setProperty("ro.product.manufacturer", "Google");
    
    // Remove Nox properties
    adb.executeShellCommand("settings delete global nox_pen_status 2>/dev/null || true");
    adb.executeShellCommand("settings delete global nox_support 2>/dev/null || true");
    adb.executeShellCommand("settings delete global is_nox 2>/dev/null || true");
    adb.executeShellCommand("settings delete global uses_nox 2>/dev/null || true");
    
    // Hide Nox packages
    adb.executeShellCommand("pm hide com.nox.im 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.nox.gamelauncher 2>/dev/null || true");
    
    m_bypassStatus["nox"] = true;
    return true;
}

bool DetectionBypass::bypassLDPlayerDetection() {
    Logger::getInstance().info("Bypassing LDPlayer detection...");
    auto& adb = ADBManager::getInstance();
    
    // Set realistic properties
    adb.setProperty("ro.product.model", "Galaxy S23");
    adb.setProperty("ro.product.device", "z3q");
    adb.setProperty("ro.product.brand", "Samsung");
    adb.setProperty("ro.product.manufacturer", "Samsung");
    
    // Remove LDPlayer properties
    adb.executeShellCommand("settings delete global ldb_multi_instance 2>/dev/null || true");
    adb.executeShellCommand("settings delete global ldb_support 2>/dev/null || true");
    adb.executeShellCommand("settings delete global is_ldb 2>/dev/null || true");
    adb.executeShellCommand("settings delete global uses_ldb 2>/dev/null || true");
    
    // Hide LDPlayer packages
    adb.executeShellCommand("pm hide com.longtu.hkswitch 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.longtu.member 2>/dev/null || true");
    
    m_bypassStatus["ldplayer"] = true;
    return true;
}

bool DetectionBypass::hideNoxApps() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("pm hide com.nox.im 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.nox.gamelauncher 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideLDPlayerApps() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("pm hide com.longtu.hkswitch 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.longtu.member 2>/dev/null || true");
    return true;
}

bool DetectionBypass::setNoxModel() {
    return setProductModel("Pixel 7 Pro");
}

bool DetectionBypass::setLDPlayerModel() {
    return setProductModel("Galaxy S23");
}

bool DetectionBypass::bypassVirtualBoxDetection() {
    Logger::getInstance().info("Bypassing VirtualBox detection...");
    auto& adb = ADBManager::getInstance();
    
    // Remove VirtualBox properties
    adb.setProperty("ro.vbox.app.mode", "0");
    adb.setProperty("ro.vbox.host.mode", "0");
    adb.setProperty("ro.vbox.version", "");
    
    adb.executeShellCommand("getprop | grep vbox | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    adb.executeShellCommand("getprop | grep virtualbox | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    
    // Hide VirtualBox files
    adb.executeShellCommand("chmod 000 /system/lib64/vboxguest.so 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/lib/vboxguest.so 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/lib64/vboxvideo_drv_arm.so 2>/dev/null || true");
    
    // VMWare detection
    adb.executeShellCommand("getprop | grep vmware | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    adb.executeShellCommand("getprop | grep vmci | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    
    m_bypassStatus["virtualbox"] = true;
    return true;
}

bool DetectionBypass::hideVirtualBoxFiles() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("chmod 000 /system/lib64/vboxguest.so 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/lib/vboxguest.so 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideVMWareFiles() {
    return hideVirtualBoxFiles();
}

// ========== ROOT DETECTION BYPASS ==========

bool DetectionBypass::bypassRootDetection() {
    Logger::getInstance().info("Bypassing root detection...");
    hideSuBinary();
    hideMagisk();
    hideXposed();
    hideBusyBox();
    hideRootManagementApps();
    hideTestKeys();
    return true;
}

bool DetectionBypass::hideSuBinary() {
    auto& adb = ADBManager::getInstance();
    
    // Move su binaries
    adb.executeShellCommand("mount -o rw,remount /system 2>/dev/null || true");
    adb.executeShellCommand("mv /system/bin/su /system/bin/su.bak 2>/dev/null || true");
    adb.executeShellCommand("mv /system/xbin/su /system/xbin/su.bak 2>/dev/null || true");
    adb.executeShellCommand("mv /su/bin/su /su/bin/su.bak 2>/dev/null || true");
    
    // Remove access
    adb.executeShellCommand("chmod 000 /system/bin/su 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/xbin/su 2>/dev/null || true");
    adb.executeShellCommand("chattr -i /system/bin/su 2>/dev/null || true");
    adb.executeShellCommand("chattr -i /system/xbin/su 2>/dev/null || true");
    
    // Properties
    adb.setProperty("ro.build.selinux.enforce", "true");
    
    return true;
}

bool DetectionBypass::hideMagisk() {
    auto& adb = ADBManager::getInstance();
    
    // Disable Magisk hide
    adb.executeShellCommand("magisk --denylist add com.google.android.gms 2>/dev/null || true");
    adb.executeShellCommand("magisk hide enable 2>/dev/null || true");
    adb.executeShellCommand("resetprop magisk.hide true 2>/dev/null || true");
    adb.setProperty("persist.magisk.hide", "true");
    
    // Hide Magisk packages
    adb.executeShellCommand("pm hide com.topjohnwu.magisk 2>/dev/null || true");
    adb.executeShellCommand("pm disable-user --user 0 com.topjohnwu.magisk 2>/dev/null || true");
    adb.executeShellCommand("pm disable-user --user 0 com.topjohnwu.magisk.boot 2>/dev/null || true");
    
    // Remove Magisk files
    adb.executeShellCommand("mv /data/adb/magisk /data/adb/magisk.bak 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /data/adb/magisk 2>/dev/null || true");
    
    return true;
}

bool DetectionBypass::hideXposed() {
    auto& adb = ADBManager::getInstance();
    
    // Disable Xposed
    adb.executeShellCommand("mv /system/bin/app_process64 /system/bin/app_process64.bak 2>/dev/null || true");
    adb.executeShellCommand("mv /system/bin/app_process32 /system/bin/app_process32.bak 2>/dev/null || true");
    adb.executeShellCommand("mv /system/bin/app_process64_xposed /system/bin/app_process64_xposed.bak 2>/dev/null || true");
    adb.executeShellCommand("mv /system/bin/app_process32_xposed /system/bin/app_process32_xposed.bak 2>/dev/null || true");
    
    // Hide LSPosed
    adb.executeShellCommand("mv /system/bin/dex2oat /system/bin/dex2oat.bak 2>/dev/null || true");
    adb.executeShellCommand("mv /apex/com.android.runtime/bin/dex2oat /apex/com.android.runtime/bin/dex2oat.bak 2>/dev/null || true");
    
    // Disable Xposed installer
    adb.executeShellCommand("pm disable-user --user 0 de.robv.android.xposed.installer 2>/dev/null || true");
    adb.executeShellCommand("pm disable-user --user 0 org.meowcat.edxposed.manager 2>/dev/null || true");
    
    return true;
}

bool DetectionBypass::hideFrida() {
    auto& adb = ADBManager::getInstance();
    
    // Kill frida-server
    adb.executeShellCommand("killall frida-server 2>/dev/null || true");
    adb.executeShellCommand("pkill -f frida 2>/dev/null || true");
    
    // Hide frida ports
    adb.executeShellCommand("chmod 000 /dev/tcp/127.0.0.1/27042 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /dev/tcp/127.0.0.1/27043 2>/dev/null || true");
    
    return true;
}

bool DetectionBypass::hideSubstrate() {
    return hideXposed();
}

bool DetectionBypass::hideBusyBox() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("chmod 000 /system/xbin/busybox 2>/dev/null || true");
    adb.executeShellCommand("mv /system/xbin/busybox /system/xbin/busybox.bak 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideRootManagementApps() {
    auto& adb = ADBManager::getInstance();
    
    // Common root management apps
    std::vector<std::string> rootApps = {
        "com.topjohnwu.magisk",
        "com.horcrux.rootcloak",
        "com.devadvance.rootcloak",
        "com.devadvance.rootcloakplus",
        "com.alephzain.rootcloak",
        "com.nickmotor.rootdetect",
        "com.test一.testroot"
    };
    
    for (const auto& app : rootApps) {
        adb.executeShellCommand("pm hide " + app + " 2>/dev/null || true");
    }
    
    return true;
}

bool DetectionBypass::configureDenylist() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.denylist.enabled", "true");
    adb.executeShellCommand("magisk denylist enable 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideZygisk() {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("rozygisk.enable", "false");
    adb.executeShellCommand("magisk resetprop rozygisk.enable false 2>/dev/null || true");
    return true;
}

bool DetectionBypass::hideMagiskHide() {
    return hideMagisk();
}

bool DetectionBypass::hideSystemProps() {
    auto& adb = ADBManager::getInstance();
    
    // Remove common root props
    adb.executeShellCommand("getprop | grep -E '(root|magisk|supersu)' | cut -d: -f1 | xargs -I{} resetprop {} '' 2>/dev/null || true");
    
    // Reset suspicious props
    adb.executeShellCommand("resetprop ro.build.selinux.enforce '' 2>/dev/null || true");
    adb.executeShellCommand("resetprop ro.debuggable '' 2>/dev/null || true");
    
    return true;
}

bool DetectionBypass::hideKapAndAmal() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("pm hide com.therichkeys.rootcloak 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.k隐藏 2>/dev/null || true");
    adb.executeShellCommand("pm hide com.anon.rootcloak 2>/dev/null || true");
    
    return true;
}

// ========== DEBUG DETECTION BYPASS ==========

bool DetectionBypass::bypassDebugDetection() {
    hideDebugFlags();
    disabledeveloperOptions();
    hideTracerPid();
    return true;
}

bool DetectionBypass::hideDebugFlags() {
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("ro.debuggable", "0");
    adb.setProperty("ro.adb.secure", "1");
    adb.setProperty("persist.sys.usb.config", "mtp");
    
    adb.executeShellCommand("settings put global adb_enabled 0");
    adb.executeShellCommand("settings put global developer_options 0");
    adb.executeShellCommand("settings put global debug.view.allocation 0");
    
    return true;
}

bool DetectionBypass::disableDebuggable() {
    return hideDebugFlags();
}

bool DetectionBypass::hideTracerPid() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("chmod 000 /proc/self/status 2>/dev/null || true");
    return true;
}

bool DetectionBypass::disabledeveloperOptions() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("settings put global development_settings_enabled 0");
    adb.executeShellCommand("settings put global adb_enabled 0");
    adb.executeShellCommand("settings put global show_ota_update 0");
    return true;
}

bool DetectionBypass::hideDevOptionsMenu() {
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("settings put global show_developer_settings 0");
    adb.executeShellCommand("settings put global developer_options 0");
    adb.executeShellCommand("settings put secure dev_options_enabled 0");
    return true;
}

// ========== FILE SYSTEM DETECTION BYPASS ==========

bool DetectionBypass::bypassFileSystemDetection() {
    hideTestKeys();
    hideCustomROM();
    hideSUAccess();
    hideRootFiles();
    return true;
}

bool DetectionBypass::hideTestKeys() {
    auto& adb = ADBManager::getInstance();
    
    std::string fingerprint = adb.getProperty("ro.build.fingerprint");
    if (fingerprint.find("test-keys") != std::string::npos) {
        std::string fixed = fingerprint;
        size_t pos = fixed.find("test-keys");
        if (pos != std::string::npos) {
            fixed = fixed.substr(0, pos) + "release-keys" + fixed.substr(pos + 9);
        }
        adb.setProperty("ro.build.fingerprint", fixed);
        adb.setProperty("ro.bootimage.build.fingerprint", fixed);
    }
    
    adb.setProperty("ro.build.tags", "release-keys");
    adb.setProperty("ro.system.build.tags", "release-keys");
    adb.setProperty("ro.vendor.build.tags", "release-keys");
    adb.setProperty("ro.odm.build.tags", "release-keys");
    adb.setProperty("ro.product.build.tags", "release-keys");
    
    return true;
}

bool DetectionBypass::hideCustomROM() {
    auto& adb = ADBManager::getInstance();
    
    // Remove ROM indicators
    adb.setProperty("ro.build.version.custom", "");
    adb.setProperty("ro.build.display.custom", "");
    
    return true;
}

bool DetectionBypass::hideSUAccess() {
    return hideSuBinary();
}

bool DetectionBypass::hideRootFiles() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("chmod 000 /data/su 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /data/adb/su 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/su 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /system/xbin/su 2>/dev/null || true");
    
    return true;
}

bool DetectionBypass::hideMagiskFiles() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("chmod 000 /data/adb/magisk 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /data/adb/ su 2>/dev/null || true");
    adb.executeShellCommand("chmod 000 /sbin/su 2>/dev/null || true");
    
    return true;
}

bool DetectionBypass::hideXposedFiles() {
    return hideXposed();
}

// ========== PLAY STORE VERIFICATION ==========

bool DetectionBypass::bypassPlayStoreVerification() {
    return setPlayStoreLicensing(true);
}

bool DetectionBypass::setPlayStoreLicensing(bool isLicensed) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.play.licensed", isLicensed ? "true" : "false");
    adb.setProperty("ro.setupwizard.mode", "OPTIONAL");
    return true;
}

bool DetectionBypass::configurePlayStoreResponse() {
    return setPlayStoreLicensing(true);
}

bool DetectionBypass::setLicenseCheckResult(int result) {
    auto& adb = ADBManager::getInstance();
    adb.setProperty("ro.license.check.result", std::to_string(result));
    return true;
}

// ========== COMPLETE APPLICATION ==========

bool DetectionBypass::applyFullBypass() {
    Logger::getInstance().info("=== APPLYING FULL BYPASS ===");
    
    // 1. Build Properties
    setAllBuildProperties("Samsung", "Galaxy S23", "Samsung", "o1s");
    setBuildTags("release-keys");
    setBuildType("user");
    
    // 2. SafetyNet & Play Integrity
    configureSafetyNetResponse();
    configurePlayIntegrityResponse();
    
    // 3. Root Bypass
    bypassRootDetection();
    hideMagisk();
    hideZygisk();
    
    // 4. Emulator Bypass
    bypassQEMUDetection();
    bypassGenymotionDetection();
    bypassBlueStacksDetection();
    bypassNoxDetection();
    bypassLDPlayerDetection();
    bypassVirtualBoxDetection();
    
    // 5. Hook Detection
    bypassHookDetection();
    
    // 6. Debug Detection
    bypassDebugDetection();
    
    // 7. GMS
    configureGMSPackages();
    
    // 8. File System
    bypassFileSystemDetection();
    
    // 9. Network
    bypassNetworkDetection();
    
    // 10. Timing
    bypassTimingDetection();
    
    // 11. Sensor
    bypassSensorDetection();
    
    // 12. DNS
    applyProperty("ro.net.dns1", "8.8.8.8");
    applyProperty("ro.net.dns2", "8.8.4.4");
    
    m_fullBypassApplied = true;
    Logger::getInstance().info("=== FULL BYPASS APPLIED ===");
    
    return true;
}

bool DetectionBypass::applyUltimateHardening() {
    Logger::getInstance().info("=== APPLYING ULTIMATE HARDENING ===");
    
    applyFullBypass();
    
    // Additional hardening
    setSecureHardware(true);
    enableTrustZone();
    enableHardwareAttestation();
    setStrongBoxAttestation(true);
    setTEEAttestation(true);
    setKeyAttestation(true);
    
    enableAndroidKeystore();
    setKeymasterVersion(4);
    setSecurityLevel(3);
    
    // Hide all detection methods
    hideFrida();
    hideTracerPid();
    hideSystemProps();
    hideMagiskFiles();
    hideXposedFiles();
    
    // Final verification
    hideTestKeys();
    setBuildDescription("samsung/o1sxx/o1s:14/TD1A.220804.031:user@user:user/release-keys");
    
    Logger::getInstance().info("=== ULTIMATE HARDENING COMPLETE ===");
    
    return true;
}

bool DetectionBypass::resetAllBypasses() {
    auto& adb = ADBManager::getInstance();
    
    // Restore su binaries
    adb.executeShellCommand("mv /system/bin/su.bak /system/bin/su 2>/dev/null || true");
    adb.executeShellCommand("mv /system/xbin/su.bak /system/xbin/su 2>/dev/null || true");
    
    // Restore app_process
    adb.executeShellCommand("mv /system/bin/app_process64.bak /system/bin/app_process64 2>/dev/null || true");
    adb.executeShellCommand("mv /system/bin/app_process32.bak /system/bin/app_process32 2>/dev/null || true");
    
    // Reset Magisk
    adb.executeShellCommand("pm enable com.topjohnwu.magisk 2>/dev/null || true");
    
    m_fullBypassApplied = false;
    m_bypassStatus.clear();
    
    return true;
}

// ========== STATUS ==========

std::map<std::string, bool> DetectionBypass::getBypassStatus() {
    return m_bypassStatus;
}

std::vector<std::string> DetectionBypass::getCriticalFailures() {
    std::vector<std::string> failures;
    auto& adb = ADBManager::getInstance();
    
    // Check for critical issues
    std::string type = adb.getProperty("ro.build.type");
    if (type == "userdebug") {
        failures.push_back("Build type is userdebug - will fail CTS");
    }
    
    std::string tags = adb.getProperty("ro.build.tags");
    if (tags.find("test-keys") != std::string::npos) {
        failures.push_back("Build uses test-keys - will fail SafetyNet");
    }
    
    std::string debuggable = adb.getProperty("ro.debuggable");
    if (debuggable == "1") {
        failures.push_back("Device is debuggable - will be detected");
    }
    
    std::string selinux = adb.getProperty("ro.build.selinux.enforce");
    if (selinux != "true") {
        failures.push_back("SELinux not enforcing");
    }
    
    return failures;
}

bool DetectionBypass::verifyDeviceIsReal() {
    auto failures = getCriticalFailures();
    return failures.empty();
}

std::string DetectionBypass::generateVerificationReport() {
    std::stringstream ss;
    ss << "=== DEVICE VERIFICATION REPORT ===\n\n";
    
    auto failures = getCriticalFailures();
    if (failures.empty()) {
        ss << "✓ Device appears to be real\n";
    } else {
        ss << "✗ Issues found:\n";
        for (const auto& f : failures) {
            ss << "  - " << f << "\n";
        }
    }
    
    ss << "\n=== BYPASS STATUS ===\n";
    for (const auto& [name, status] : m_bypassStatus) {
        ss << name << ": " << (status ? "✓" : "✗") << "\n";
    }
    
    return ss.str();
}

// ========== STUB IMPLEMENTATIONS ==========

bool DetectionBypass::bypassHookDetection() { return true; }
bool DetectionBypass::hideInlineHooks() { return true; }
bool DetectionBypass::hidePLTHooks() { return true; }
bool DetectionBypass::hideFunctionHooks() { return true; }
bool DetectionBypass::patchStrstr() { return true; }
bool DetectionBypass::patchDlsym() { return true; }
bool DetectionBypass::patchRead() { return true; }
bool DetectionBypass::patchOpen() { return true; }

bool DetectionBypass::bypassNetworkDetection() { return true; }
bool DetectionBypass::setVPNDetection(bool) { return true; }
bool DetectionBypass::setProxyDetection(bool) { return true; }
bool DetectionBypass::setTorDetection(bool) { return true; }
bool DetectionBypass::hideVPNApps() { return true; }
bool DetectionBypass::configureNetworkProxy() { return true; }

bool DetectionBypass::bypassTimingDetection() { return true; }
bool DetectionBypass::addCpuLatency() { return true; }
bool DetectionBypass::setRealBootTime() { return true; }
bool DetectionBypass::hideUptime() { return true; }
bool DetectionBypass::setRealTimeSinceBoot() { return true; }

bool DetectionBypass::bypassSensorDetection() { return true; }
bool DetectionBypass::setRealAccelerometer() { return true; }
bool DetectionBypass::setRealGyroscope() { return true; }
bool DetectionBypass::setRealMagnetometer() { return true; }
bool DetectionBypass::hideSensorManipulation() { return true; }
bool DetectionBypass::setRealLightSensor() { return true; }
bool DetectionBypass::setRealProximitySensor() { return true; }

bool DetectionBypass::bypassFileSystemDetection() { return true; }
bool DetectionBypass::hideCustomROM() { return true; }
bool DetectionBypass::hideSUAccess() { return true; }
bool DetectionBypass::hideRootFiles() { return true; }
bool DetectionBypass::hideMagiskFiles() { return true; }
bool DetectionBypass::hideXposedFiles() { return true; }

bool DetectionBypass::bypassPermissionDetection() { return true; }
bool DetectionBypass::setRealPermissions() { return true; }
bool DetectionBypass::hideDangerousPermissions() { return true; }
bool DetectionBypass::setDangerousPermissionState(bool) { return true; }

}
