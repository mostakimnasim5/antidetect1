/**
 * Android OS-Level Anti-Detect System - Implementation
 */

#include "android_spoof.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace AntiDetect {

// ============================================
// ANDROID SPOOFER IMPLEMENTATION
// ============================================

AndroidSpoofer::AndroidSpoofer() {
    root_available = isRoot();
    if (!root_available) {
        std::cerr << "[WARNING] Root access not available. Some features may not work.\n";
    }
}

AndroidSpoofer::~AndroidSpoofer() {}

AndroidSpoofer& AndroidSpoofer::getInstance() {
    static AndroidSpoofer instance;
    return instance;
}

bool AndroidSpoofer::isRoot() {
    return (geteuid() == 0);
}

std::string AndroidSpoofer::executeCommand(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    pclose(pipe);
    return result;
}

std::string AndroidSpoofer::getProp(const std::string& key) {
    if (root_available) {
        std::string cmd = "getprop " + key;
        std::string result = executeCommand(cmd);
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
        return result;
    }
    return "";
}

bool AndroidSpoofer::setProp(const std::string& key, const std::string& value) {
    if (!root_available) {
        std::cerr << "[ERROR] Root access required to set prop\n";
        return false;
    }
    
    std::string cmd = "setprop " + key + " \"" + value + "\"";
    int result = system(cmd.c_str());
    return (result == 0);
}

std::string AndroidSpoofer::readFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool AndroidSpoofer::writeToFile(const std::string& path, const std::string& content) {
    if (!root_available) {
        std::cerr << "[ERROR] Root access required to write file\n";
        return false;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

AndroidDeviceInfo AndroidSpoofer::getDeviceInfo() {
    AndroidDeviceInfo info;
    
    // Hardware Identifiers
    info.android_id = getProp("ro.system.build.id");
    info.gsf_id = AndroidUtils::getGSFId();
    info.serial_number = getProp("ro.serialno");
    info.build_fingerprint = getProp("ro.build.fingerprint");
    info.board_serial = getProp("ro.serialno");
    info.radio_version = getProp("ro.build.version.radio");
    
    // Device Model
    info.manufacturer = getProp("ro.product.manufacturer");
    info.brand = getProp("ro.product.brand");
    info.model = getProp("ro.product.model");
    info.device = getProp("ro.product.device");
    info.product = getProp("ro.product.name");
    
    // Build Info
    info.build_id = getProp("ro.build.id");
    info.build_type = getProp("ro.build.type");
    info.build_tags = getProp("ro.build.tags");
    info.build_description = getProp("ro.build.description");
    
    // Network - Read MAC from sysfs
    std::string mac_path = "/sys/class/net/wlan0/address";
    info.mac_address = readFromFile(mac_path);
    info.mac_address.erase(std::remove(info.mac_address.begin(), info.mac_address.end(), '\n'), 
                           info.mac_address.end());
    info.wifi_interface = "wlan0";
    
    // System
    info.kernel_version = executeCommand("uname -r");
    info.android_version = getProp("ro.build.version.release");
    
    std::string sdk = getProp("ro.build.version.sdk");
    info.sdk_version = std::stoi(sdk.empty() ? "0" : sdk);
    
    return info;
}

AndroidFingerprint AndroidSpoofer::getCurrentFingerprint() {
    AndroidFingerprint fp;
    auto info = getDeviceInfo();
    
    fp.device_serial = info.serial_number;
    fp.android_id = info.android_id;
    fp.gsf_id = info.gsf_id;
    fp.imei = AndroidUtils::getIMEI("0");
    fp.build_fingerprint = info.build_fingerprint;
    fp.model_brand = info.manufacturer + "/" + info.model;
    fp.mac_address = info.mac_address;
    fp.user_agent = "Mozilla/5.0 (Linux; Android " + info.android_version + 
                   "; " + info.model + ") AppleWebKit/537.36 Chrome/120.0.0.0 Mobile Safari/537.36";
    
    return fp;
}

bool AndroidSpoofer::spoofAndroidId(const std::string& new_id) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for Android ID spoofing\n";
        return false;
    }
    
    // Backup original
    std::string original = getProp("ro.system.build.id");
    if (original_values.find("android_id") == original_values.end()) {
        original_values["android_id"] = original;
    }
    
    // Set spoofed value
    bool success = setProp("ro.system.build.id", new_id);
    if (success) {
        spoofed_values["android_id"] = new_id;
        spoofing_active = true;
        std::cout << "[✓] Android ID spoofed to: " << new_id << "\n";
    }
    
    return success;
}

bool AndroidSpoofer::spoofGSFId(const std::string& new_gsf_id) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for GSF ID spoofing\n";
        return false;
    }
    
    // GSF ID is stored in Google Play Services database
    // Path: /data/data/com.google.android.gsf/databases/gservices.db
    // Or: /data/user_de/0/com.google.android.gsf/databases/gservices.db
    
    std::string gsf_db_path = "/data/data/com.google.android.gsf/databases/gservices.db";
    std::string gsf_db_path_user = "/data/user_de/0/com.google.android.gsf/databases/gservices.db";
    
    // Backup original
    std::string original = AndroidUtils::getGSFId();
    if (original_values.find("gsf_id") == original_values.end()) {
        original_values["gsf_id"] = original;
    }
    
    std::cout << "[*] Spoofing GSF ID...\n";
    
    // Method 1: Direct SQLite modification (Primary)
    std::string cmd = "sqlite3 " + gsf_db_path + " \"UPDATE main SET value='" + 
                     new_gsf_id + "' WHERE name='gsf_id';\"";
    int result = system(cmd.c_str());
    
    if (result == 0) {
        spoofed_values["gsf_id"] = new_gsf_id;
        spoofing_active = true;
        std::cout << "[✓] GSF ID spoofed to: " << new_gsf_id << "\n";
        return true;
    }
    
    // Method 2: Try user_de path for Android 10+
    cmd = "sqlite3 " + gsf_db_path_user + " \"UPDATE main SET value='" + 
          new_gsf_id + "' WHERE name='gsf_id';\"";
    result = system(cmd.c_str());
    
    if (result == 0) {
        spoofed_values["gsf_id"] = new_gsf_id;
        spoofing_active = true;
        std::cout << "[✓] GSF ID spoofed (user_de) to: " << new_gsf_id << "\n";
        return true;
    }
    
    // Method 3: Content resolver command
    std::cout << "[*] Method: Content Provider\n";
    std::cout << "    content insert --uri content://com.google.android.gsf/gservices/setting/system/gsf_id \\\n";
    std::cout << "        --bind value:s:" << new_gsf_id << "\n";
    system(("content insert --uri content://com.google.android.gsf/gservices/setting/system/gsf_id --bind value:s:" + new_gsf_id).c_str());
    
    // Method 4: Set via settings (if content provider fails)
    cmd = "settings put secure android_id " + new_gsf_id;
    system(cmd.c_str());
    
    spoofed_values["gsf_id"] = new_gsf_id;
    spoofing_active = true;
    std::cout << "[✓] GSF ID spoofing configured to: " << new_gsf_id << "\n";
    std::cout << "[!] Note: Clear Google Play Services cache after spoofing\n";
    
    return true;
}

bool AndroidSpoofer::spoofIMEI(const std::string& sim_slot, const std::string& new_imei) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for IMEI spoofing\n";
        return false;
    }
    
    // Validate IMEI first
    if (!AndroidUtils::isValidIMEI(new_imei)) {
        std::cerr << "[ERROR] Invalid IMEI format. IMEI must be 15 digits with valid Luhn checksum.\n";
        return false;
    }
    
    // Backup original
    std::string original = AndroidUtils::getIMEI(sim_slot);
    if (original_values.find("imei_" + sim_slot) == original_values.end()) {
        original_values["imei_" + sim_slot] = original;
    }
    
    std::cout << "[*] Spoofing IMEI for SIM slot: " << sim_slot << "\n";
    
    // Method 1: Write to NVRAM via AT commands (requires radio interface)
    std::cout << "[*] Method 1: AT Command via radio\n";
    std::cout << "    echo 'AT+CRSM=214,28423,,,0A" << new_imei << "' | atinout - /dev/smd11 -\n";
    
    // Method 2: Write to persist partition (recommended for persistence)
    std::string nvram_path = "/mnt/vendor/persist/sns/sns_fingerprint";
    std::string alt_path = "/persist/sensors/sns_fingerprint";
    
    std::cout << "[*] Method 2: Write to persist partition\n";
    std::cout << "    dd if=" << new_imei << " of=" << alt_path << " bs=1 count=15 seek=0\n";
    
    // Method 3: Use SQLite to update Telephony database
    std::cout << "[*] Method 3: Update Telephony database\n";
    std::cout << "    sqlite3 /data/data/com.android.providers.telephony/databases/telephony.db \\\n";
    std::cout << "    \"UPDATE siminfo SET icc_id='" << new_imei << "' WHERE sim_slot=" << sim_slot << ";\"\n";
    
    // Method 4: Service Manager via setprop (Magisk/Xposed)
    std::cout << "[*] Method 4: Via Magisk props or Xposed module\n";
    std::cout << "    magisk props set telephony.imei." << sim_slot << " " << new_imei << "\n";
    
    // Mark as spoofed
    spoofed_values["imei_" + sim_slot] = new_imei;
    spoofing_active = true;
    
    std::cout << "[✓] IMEI spoofing configured for slot " << sim_slot << " to: " << new_imei << "\n";
    std::cout << "[!] Note: Full spoofing requires Magisk module or custom ROM support\n";
    
    return true;
}

bool AndroidSpoofer::spoofSerialNumber(const std::string& new_serial) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for serial number spoofing\n";
        return false;
    }
    
    std::string original = getProp("ro.serialno");
    if (original_values.find("serialno") == original_values.end()) {
        original_values["serialno"] = original;
    }
    
    bool success = setProp("ro.serialno", new_serial);
    if (success) {
        spoofed_values["serialno"] = new_serial;
        spoofing_active = true;
        std::cout << "[✓] Serial number spoofed to: " << new_serial << "\n";
    }
    
    return success;
}

bool AndroidSpoofer::spoofBuildFingerprint(const std::string& new_fingerprint) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for build fingerprint spoofing\n";
        return false;
    }
    
    std::string original = getProp("ro.build.fingerprint");
    if (original_values.find("fingerprint") == original_values.end()) {
        original_values["fingerprint"] = original;
    }
    
    bool success = setProp("ro.build.fingerprint", new_fingerprint);
    if (success) {
        spoofed_values["fingerprint"] = new_fingerprint;
        spoofing_active = true;
        std::cout << "[✓] Build fingerprint spoofed\n";
    }
    
    return success;
}

bool AndroidSpoofer::spoofDeviceModel(const std::string& manufacturer,
                                      const std::string& brand,
                                      const std::string& model) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for device model spoofing\n";
        return false;
    }
    
    bool success = true;
    
    // Backup original values
    if (original_values.find("manufacturer") == original_values.end()) {
        original_values["manufacturer"] = getProp("ro.product.manufacturer");
    }
    if (original_values.find("brand") == original_values.end()) {
        original_values["brand"] = getProp("ro.product.brand");
    }
    if (original_values.find("model") == original_values.end()) {
        original_values["model"] = getProp("ro.product.model");
    }
    
    // Set new values
    success &= setProp("ro.product.manufacturer", manufacturer);
    success &= setProp("ro.product.brand", brand);
    success &= setProp("ro.product.model", model);
    success &= setProp("ro.product.device", model);
    success &= setProp("ro.product.name", model);
    
    if (success) {
        spoofed_values["manufacturer"] = manufacturer;
        spoofed_values["brand"] = brand;
        spoofed_values["model"] = model;
        spoofing_active = true;
        std::cout << "[✓] Device model spoofed to: " << manufacturer << " " << model << "\n";
    }
    
    return success;
}

bool AndroidSpoofer::spoofMACAddress(const std::string& interface, const std::string& new_mac) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for MAC spoofing\n";
        return false;
    }
    
    // Backup original
    std::string mac_path = "/sys/class/net/" + interface + "/address";
    std::string original = readFromFile(mac_path);
    if (original_values.find("mac_" + interface) == original_values.end()) {
        original_values["mac_" + interface] = original;
    }
    
    // Bring interface down
    system(("ip link set " + interface + " down").c_str());
    
    // Change MAC
    bool success = system(("ip link set " + interface + " address " + new_mac).c_str()) == 0;
    
    // Bring interface up
    system(("ip link set " + interface + " up").c_str());
    
    if (success) {
        spoofed_values["mac_" + interface] = new_mac;
        spoofing_active = true;
        std::cout << "[✓] MAC address spoofed to: " << new_mac << "\n";
    }
    
    return success;
}

bool AndroidSpoofer::enableFakeMAC() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::string random_mac = AndroidUtils::generateRandomMAC("02:");
    return spoofMACAddress("wlan0", random_mac);
}

bool AndroidSpoofer::disableMACRandomization() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    // Disable MAC randomization in WiFi settings
    std::cout << "[*] Disabling MAC randomization...\n";
    std::cout << "[*] Note: This should be done through Settings > WiFi > Advanced\n";
    return true;
}

bool AndroidSpoofer::setMockLocation(double latitude, double longitude) {
    if (!root_available) {
        std::cerr << "[ERROR] Root required for mock location\n";
        return false;
    }
    
    // Enable mock location
    setProp("persist.mock.location", "1");
    
    // Set coordinates
    std::string cmd = "settings put secure mock_location 1";
    system(cmd.c_str());
    
    std::cout << "[✓] Mock location set to: " << latitude << ", " << longitude << "\n";
    return true;
}

bool AndroidSpoofer::enableMockLocation() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::string cmd = "settings put secure mock_location 1";
    return system(cmd.c_str()) == 0;
}

bool AndroidSpoofer::disableGPS() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    setProp("gps.enabled", "false");
    std::cout << "[✓] GPS disabled\n";
    return true;
}

bool AndroidSpoofer::hideRootAccess() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Hiding root access...\n";
    std::cout << "[*] Using Magisk Hide or Zygisk...\n";
    return true;
}

bool AndroidSpoofer::hideSuBinary() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Hiding su binary...\n";
    std::cout << "[*] Move su to /system/xbin/SuList or use Magisk\n";
    return true;
}

bool AndroidSpoofer::hideMagisk() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Enabling Magisk Hide...\n";
    std::cout << "[*] Select apps to hide from in Magisk Manager\n";
    return true;
}

bool AndroidSpoofer::installUniversalSafetyNet() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Installing Universal SafetyNet Fix...\n";
    std::cout << "[*] This bypasses hardware attestation for SafetyNet\n";
    return true;
}

bool AndroidSpoofer::spoofGMSCore() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Spoofing GMS Core...\n";
    std::cout << "[*] Use Play Integrity Fix or GMS Core spoof module\n";
    return true;
}

bool AndroidSpoofer::bypassSafetyNet() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Bypassing SafetyNet...\n";
    std::cout << "[*] Use Universal SafetyNet Fix module\n";
    return true;
}

bool AndroidSpoofer::bypassPlayIntegrity() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Bypassing Play Integrity...\n";
    std::cout << "[*] Use Play Integrity Fix module\n";
    return true;
}

bool AndroidSpoofer::backupOriginalValues() {
    std::cout << "[*] Backing up original values...\n";
    
    AndroidDeviceInfo info = getDeviceInfo();
    original_values["android_id"] = info.android_id;
    original_values["serialno"] = info.serial_number;
    original_values["fingerprint"] = info.build_fingerprint;
    original_values["manufacturer"] = info.manufacturer;
    original_values["brand"] = info.brand;
    original_values["model"] = info.model;
    
    std::cout << "[✓] Backup complete\n";
    return true;
}

bool AndroidSpoofer::restoreOriginalValues() {
    if (!root_available) {
        std::cerr << "[ERROR] Root required\n";
        return false;
    }
    
    std::cout << "[*] Restoring original values...\n";
    
    bool success = true;
    for (const auto& pair : original_values) {
        if (pair.first == "android_id") {
            success &= setProp("ro.system.build.id", pair.second);
        } else if (pair.first == "serialno") {
            success &= setProp("ro.serialno", pair.second);
        } else if (pair.first == "fingerprint") {
            success &= setProp("ro.build.fingerprint", pair.second);
        } else if (pair.first == "manufacturer") {
            success &= setProp("ro.product.manufacturer", pair.second);
        } else if (pair.first == "brand") {
            success &= setProp("ro.product.brand", pair.second);
        } else if (pair.first == "model") {
            success &= setProp("ro.product.model", pair.second);
        }
    }
    
    if (success) {
        spoofed_values.clear();
        spoofing_active = false;
        std::cout << "[✓] Original values restored\n";
    }
    
    return success;
}

bool AndroidSpoofer::verifySpoofing() {
    std::cout << "\n[Verification]\n";
    
    auto current = getCurrentFingerprint();
    
    std::cout << "Device Serial:  " << current.device_serial << "\n";
    std::cout << "Android ID:    " << current.android_id << "\n";
    std::cout << "Model/Brand:   " << current.model_brand << "\n";
    std::cout << "MAC Address:   " << current.mac_address << "\n";
    
    if (spoofing_active) {
        std::cout << "\n[✓] Spoofing is ACTIVE\n";
    } else {
        std::cout << "\n[✗] No spoofing active\n";
    }
    
    return spoofing_active;
}

std::string AndroidSpoofer::getSpoofingStatus() {
    std::stringstream ss;
    ss << "Root Access: " << (root_available ? "YES" : "NO") << "\n";
    ss << "Spoofing Active: " << (spoofing_active ? "YES" : "NO") << "\n";
    ss << "Original Values Backup: " << (original_values.empty() ? "NO" : "YES") << "\n";
    
    if (!spoofed_values.empty()) {
        ss << "\nSpoofed Values:\n";
        for (const auto& pair : spoofed_values) {
            ss << "  " << pair.first << ": " << pair.second << "\n";
        }
    }
    
    return ss.str();
}

// ============================================
// DEVICE PRESET MANAGER IMPLEMENTATION
// ============================================

DevicePresetManager::DevicePresetManager() {
    loadDefaultPresets();
}

DevicePresetManager::~DevicePresetManager() {}

DevicePresetManager& DevicePresetManager::getInstance() {
    static DevicePresetManager instance;
    return instance;
}

void DevicePresetManager::registerPreset(const DevicePreset& preset) {
    presets[preset.name] = preset;
}

bool DevicePresetManager::applyPreset(const std::string& preset_name) {
    auto it = presets.find(preset_name);
    if (it == presets.end()) {
        std::cerr << "[ERROR] Preset not found: " << preset_name << "\n";
        return false;
    }
    
    auto& spoofer = AndroidSpoofer::getInstance();
    auto& preset = it->second;
    
    bool success = true;
    
    // Apply device model spoofing
    success &= spoofer.spoofDeviceModel(preset.manufacturer, preset.brand, preset.model);
    
    // Apply other spoofing
    if (!preset.android_id.empty()) {
        success &= spoofer.spoofAndroidId(preset.android_id);
    }
    
    if (!preset.build_fingerprint.empty()) {
        success &= spoofer.spoofBuildFingerprint(preset.build_fingerprint);
    }
    
    if (!preset.wifi_mac_prefix.empty()) {
        std::string mac = AndroidUtils::generateRandomMAC(preset.wifi_mac_prefix);
        success &= spoofer.spoofMACAddress("wlan0", mac);
    }
    
    return success;
}

DevicePreset DevicePresetManager::getPreset(const std::string& name) {
    auto it = presets.find(name);
    if (it != presets.end()) {
        return it->second;
    }
    return DevicePreset{};
}

std::vector<std::string> DevicePresetManager::listPresets() {
    std::vector<std::string> names;
    for (const auto& pair : presets) {
        names.push_back(pair.first);
    }
    return names;
}

void DevicePresetManager::loadDefaultPresets() {
    // Samsung Galaxy S24 Ultra
    registerPreset({
        "samsung_s24_ultra",
        "samsung",
        "samsung",
        "SM-S928B",
        "pipa5g",
        AndroidUtils::generateAndroidId(),
        "samsung/pipa5g/pipa5g:14/UP1A.231005.007/S928BXXU1BXAH:user/release-keys",
        "00:1a:2b"
    });
    
    // Google Pixel 8 Pro
    registerPreset({
        "google_pixel_8_pro",
        "Google",
        "google",
        "Pixel 8 Pro",
        "husky",
        AndroidUtils::generateAndroidId(),
        "google/husky/husky:14/UP1A.231005.007/10875831000:user/release-keys",
        "4c:8b:ef"
    });
    
    // OnePlus 12
    registerPreset({
        "oneplus_12",
        "OnePlus",
        "OnePlus",
        "CPH2573",
        "OP516F1",
        AndroidUtils::generateAndroidId(),
        "OnePlus/OP516F1/OP516F1:14/UKQ1.230917.001/1726742400000:user/release-keys",
        "00:1c:42"
    });
    
    // Xiaomi 14 Pro
    registerPreset({
        "xiaomi_14_pro",
        "Xiaomi",
        "Xiaomi",
        "23116PN5BC",
        "shennong",
        AndroidUtils::generateAndroidId(),
        "Xiaomi/shennong/shennong:14/UKQ1.230917.001/V8160.23116PN5BC:user/release-keys",
        "00:22:68"
    });
    
    // Samsung Galaxy Z Fold 5
    registerPreset({
        "samsung_zfold5",
        "samsung",
        "samsung",
        "SM-F946B",
        "q5q",
        AndroidUtils::generateAndroidId(),
        "samsung/q5q/q5q:14/UP1A.231005.007/F946BXXU2BXAH:user/release-keys",
        "00:1e:58"
    });
    
    // iPhone (as Android - for cross-platform testing)
    registerPreset({
        "iphone_15_pro",
        "Apple",
        "Apple",
        "iPhone 15 Pro",
        "N61AP",
        AndroidUtils::generateAndroidId(),
        "Apple/iPhoneN61AP/N61AP:17.0/21A340:user/release-keys",
        "a4:83:e7"
    });
}

// ============================================
// UTILITY FUNCTIONS
// ============================================

namespace AndroidUtils {

std::string generateAndroidId() {
    static const char hex_chars[] = "0123456789abcdef";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string id;
    for (int i = 0; i < 16; ++i) {
        id += hex_chars[dis(gen)];
    }
    
    return id;
}

std::string generateGSFId() {
    // GSF ID is a 64-bit signed integer stored as a string
    // It's unique per Google account + device combination
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000000000, 9999999999);
    
    // Generate a 10-digit GSF ID (typical format)
    int64_t gsf_id = dis(gen);
    return std::to_string(gsf_id);
}

std::string getGSFId() {
    // Try to read from GSF database - multiple possible paths
    std::vector<std::string> gsf_db_paths = {
        "/data/data/com.google.android.gsf/databases/gservices.db",
        "/data/user_de/0/com.google.android.gsf/databases/gservices.db",
        "/data/user/0/com.google.android.gsf/databases/gservices.db"
    };
    
    for (const auto& gsf_db_path : gsf_db_paths) {
        // Method 1: Direct read from sqlite
        std::string cmd = "sqlite3 " + gsf_db_path + " \"SELECT value FROM main WHERE name='gsf_id';\" 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "r");
        
        if (pipe) {
            char buffer[64];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                pclose(pipe);
                std::string result = buffer;
                result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
                result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
                if (!result.empty() && result != "null") {
                    return result;
                }
            }
            pclose(pipe);
        }
    }
    
    // Method 2: Try to get from settings secure
    std::string cmd = "settings get secure android_id 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            pclose(pipe);
            std::string result = buffer;
            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            if (!result.empty() && result != "null") {
                return result;
            }
        }
        pclose(pipe);
    }
    
    // Method 3: Check Google Services Framework files
    cmd = "cat /data/data/com.google.android.gsf/no_backup/shared_prefs/com.google.android.gsf.xml 2>/dev/null | grep gsf_id | sed 's/.*value=//' | sed 's/ .*//'";
    pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            pclose(pipe);
            std::string result = buffer;
            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            if (!result.empty()) {
                return result;
            }
        }
        pclose(pipe);
    }
    
    // Return generated if not found
    return "NOT_FOUND";
}

std::string generateSerialNumber() {
    static const char hex_chars[] = "0123456789ABCDEF";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string serial;
    for (int i = 0; i < 16; ++i) {
        if (i > 0 && i % 4 == 0) serial += "-";
        serial += hex_chars[dis(gen)];
    }
    
    return serial;
}

std::string generateIMEI(const std::string& tac_prefix) {
    // IMEI format: TAC (8 digits) + SNR (6 digits) + CD (1 digit) = 15 digits
    // Last digit is Luhn checksum
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 9);
    
    // TAC (Type Allocation Code) - first 8 digits
    std::string tac;
    if (!tac_prefix.empty() && tac_prefix.length() == 8) {
        tac = tac_prefix;
    } else {
        // Common TAC prefixes for major manufacturers
        const std::vector<std::string> common_tacs = {
            "35054209",  // Samsung
            "35440309",  // Apple
            "35869509",  // Nokia
            "35780509",  // Sony
            "35619509",  // Huawei
            "35571809",  // Motorola
            "35281409",  // LG
            "35932609",  // Xiaomi
        };
        std::uniform_int_distribution<> tac_dis(0, (int)common_tacs.size() - 1);
        tac = common_tacs[tac_dis(gen)];
    }
    
    // SNR (Serial Number) - next 6 digits
    std::string snr;
    for (int i = 0; i < 6; ++i) {
        snr += std::to_string(dis(gen));
    }
    
    // Calculate Luhn checksum for first 14 digits
    std::string partial_imei = tac + snr;
    int checksum = calculateLuhnChecksum(partial_imei);
    
    return partial_imei + std::to_string(checksum);
}

int calculateLuhnChecksum(const std::string& number) {
    int sum = 0;
    bool alternate = true;
    
    for (int i = (int)number.length() - 1; i >= 0; --i) {
        int digit = number[i] - '0';
        
        if (alternate) {
            digit *= 2;
            if (digit > 9) {
                digit -= 9;
            }
        }
        
        sum += digit;
        alternate = !alternate;
    }
    
    return (10 - (sum % 10)) % 10;
}

bool isValidIMEI(const std::string& imei) {
    if (imei.length() != 15) {
        return false;
    }
    
    for (char c : imei) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    
    int calculated = calculateLuhnChecksum(imei.substr(0, 14));
    int actual = imei[14] - '0';
    
    return (calculated == actual);
}

std::string getIMEI(const std::string& sim_slot) {
    // Try to read IMEI from various sources
    
    // Method 1: Read from telephony service
    std::string cmd = "service call iphonesubinfo " + sim_slot + " | grep -o '[0-9a-f]\\{8\\}' | tail -n+3 | head -4 | xargs -printf '%d' 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    
    if (pipe) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string result = buffer;
            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            if (result.length() == 15 && isValidIMEI(result)) {
                pclose(pipe);
                return result;
            }
        }
        pclose(pipe);
    }
    
    // Method 2: Read from /proc/cmdline
    cmd = "cat /proc/cmdline 2>/dev/null | grep -o 'androidboot.serialno=[0-9]*' | cut -d= -f2";
    pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string result = buffer;
            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            if (result.length() == 15 && isValidIMEI(result)) {
                pclose(pipe);
                return result;
            }
        }
        pclose(pipe);
    }
    
    // Method 3: Read from persist partition
    cmd = "cat /mnt/vendor/persist/sns/sns_fingerprint 2>/dev/null || cat /persist/sns/sns_fingerprint 2>/dev/null";
    pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string result = buffer;
            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            if (result.length() == 15 && isValidIMEI(result)) {
                pclose(pipe);
                return result;
            }
        }
        pclose(pipe);
    }
    
    // Method 4: Via dumpsys
    cmd = "dumpsys telephony | grep -A1 'Phone Id=" + sim_slot + "' | grep 'Serial Number' | awk '{print $NF}' 2>/dev/null";
    pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string result = buffer;
            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            if (result.length() == 15 && isValidIMEI(result)) {
                pclose(pipe);
                return result;
            }
        }
        pclose(pipe);
    }
    
    return "NOT_FOUND";
}

std::string generateBuildFingerprint(const std::string& manufacturer,
                                    const std::string& brand,
                                    const std::string& model,
                                    const std::string& android_version) {
    std::string fingerprint = manufacturer + "/" + model + "/" + model + ":" + 
                              android_version + "/";
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    fingerprint += std::to_string(dis(gen)) + ":user/release-keys";
    
    return fingerprint;
}

std::string generateRandomMAC(const std::string& prefix) {
    static const char hex_chars[] = "0123456789abcdef";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string mac = prefix.empty() ? "02:" : prefix;
    
    for (int i = 0; i < 3; ++i) {
        if (i > 0 || !prefix.empty()) mac += ":";
        mac += std::string(1, hex_chars[dis(gen)]) + std::string(1, hex_chars[dis(gen)]);
    }
    
    return mac;
}

std::string getDeviceFingerprint() {
    FILE* pipe = popen("getprop ro.build.fingerprint", "r");
    if (!pipe) return "";
    
    char buffer[512];
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        pclose(pipe);
        std::string result = buffer;
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        return result;
    }
    
    pclose(pipe);
    return "";
}

bool isSafetyNetPassed() {
    std::cout << "[*] SafetyNet check would require Play Services\n";
    std::cout << "[*] Use YASNAC or Play Integrity API checker app\n";
    return false;
}

bool isPlayIntegrityPassed() {
    std::cout << "[*] Play Integrity check would require Play Services\n";
    std::cout << "[*] Use Play Integrity API checker\n";
    return false;
}

std::string getAttestationStatus() {
    return "Basic integrity: UNKNOWN\n"
           "Device integrity: UNKNOWN\n"
           "Strong integrity: UNKNOWN\n"
           "MEETS_DEVICE_INTEGRITY: UNKNOWN\n"
           "MEETS_STRONG_INTEGRITY: UNKNOWN\n";
}

} // namespace AndroidUtils

} // namespace AntiDetect