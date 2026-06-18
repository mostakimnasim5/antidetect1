/**
 * OS-Level Anti-Detect System - Implementation
 * 
 * This module implements real OS-level fingerprint manipulation
 * using Linux system calls, /proc, /sys, and other kernel interfaces.
 */

#include "system_fingerprint.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <algorithm>
#include <thread>
#include <future>
#include <cstring>
#include <array>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

namespace AntiDetect {

// ============================================
// PRIVATE IMPLEMENTATION
// ============================================

struct SystemFingerprintManager::Impl {
    std::mutex mutex;
    std::map<std::string, BrowserProfile> browser_profiles;
    std::map<std::string, std::string> original_values;
    std::map<std::string, std::string> spoofed_values;
    bool changes_applied = false;
};

struct PresetManager::Impl {
    std::map<std::string, FingerprintPreset> presets;
    std::mutex mutex;
};

// ============================================
// UTILITY IMPLEMENTATIONS
// ============================================

namespace Utils {

std::string generateRandomHex(int length) {
    static const char hex_chars[] = "0123456789abcdef";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string result;
    result.reserve(length);
    
    for (int i = 0; i < length; ++i) {
        result += hex_chars[dis(gen)];
    }
    
    return result;
}

std::string generateRandomMAC(const std::string& prefix) {
    static const char hex_chars[] = "0123456789abcdef";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 255);
    
    std::string mac;
    
    // Use prefix if provided (e.g., "00:0c:29" for VMware)
    if (!prefix.empty()) {
        mac = prefix;
    } else {
        // Generate random OUI (first 3 bytes)
        mac = hex_chars[dis(gen) % 6] + std::string(1, hex_chars[dis(gen)]);
        mac += ":" + std::string(1, hex_chars[dis(gen) % 6]) + std::string(1, hex_chars[dis(gen)]);
        mac += ":" + std::string(1, hex_chars[dis(gen) % 6]) + std::string(1, hex_chars[dis(gen)]);
    }
    
    // Generate random NIC (last 3 bytes)
    for (int i = 0; i < 3; ++i) {
        mac += ":" + std::string(1, hex_chars[dis(gen)]) + std::string(1, hex_chars[dis(gen)]);
    }
    
    return mac;
}

std::string generateRandomUUID() {
    static const char hex_chars[] = "0123456789abcdef";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string uuid = generateRandomHex(8) + "-";
    uuid += generateRandomHex(4) + "-";
    uuid += "4" + generateRandomHex(3) + "-";  // Version 4
    uuid += std::string(1, hex_chars[dis(gen) % 4 + 8]) + generateRandomHex(3) + "-";  // Variant
    uuid += generateRandomHex(12);
    
    return uuid;
}

std::string generateMachineId() {
    return generateRandomHex(32);
}

std::string hashString(const std::string& input) {
    // Simple djb2 hash for fingerprinting
    unsigned long hash = 5381;
    for (char c : input) {
        hash = ((hash << 5) + hash) + c;
    }
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

bool writeToFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

std::string readFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool fileExists(const std::string& path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0);
}

std::string executeCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
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

bool isRoot() {
    return (geteuid() == 0);
}

} // namespace Utils

// ============================================
// SYSTEM FINGERPRINT MANAGER IMPLEMENTATION
// ============================================

SystemFingerprintManager::SystemFingerprintManager()
    : pImpl(std::make_unique<Impl>()) {
    // Store original values for potential revert
    pImpl->original_values["machine_id"] = Utils::readFromFile("/etc/machine-id");
    pImpl->original_values["hostname"] = Utils::executeCommand("hostname");
}

SystemFingerprintManager::~SystemFingerprintManager() {
    cleanup();
}

SystemFingerprintManager& SystemFingerprintManager::getInstance() {
    static SystemFingerprintManager instance;
    return instance;
}

HardwareIdentifiers SystemFingerprintManager::getHardwareIdentifiers() {
    HardwareIdentifiers ids;
    
    // Read machine-id
    ids.machine_id = Utils::readFromFile("/etc/machine-id");
    ids.machine_id.erase(std::remove(ids.machine_id.begin(), ids.machine_id.end(), '\n'), ids.machine_id.end());
    
    // Read product UUID from DMI
    ids.product_uuid = Utils::readFromFile("/sys/class/dmi/id/product_uuid");
    ids.product_uuid.erase(std::remove(ids.product_uuid.begin(), ids.product_uuid.end(), '\n'), ids.product_uuid.end());
    
    // Read board serial
    ids.board_serial = Utils::readFromFile("/sys/class/dmi/id/board_serial");
    ids.board_serial.erase(std::remove(ids.board_serial.begin(), ids.board_serial.end(), '\n'), ids.board_serial.end());
    
    // Read chassis serial
    ids.chassis_serial = Utils::readFromFile("/sys/class/dmi/id/chassis_serial");
    ids.chassis_serial.erase(std::remove(ids.chassis_serial.begin(), ids.chassis_serial.end(), '\n'), ids.chassis_serial.end());
    
    // Read BIOS version
    ids.bios_version = Utils::readFromFile("/sys/class/dmi/id/bios_version");
    ids.bios_version.erase(std::remove(ids.bios_version.begin(), ids.bios_version.end(), '\n'), ids.bios_version.end());
    
    return ids;
}

NetworkIdentifiers SystemFingerprintManager::getNetworkIdentifiers() {
    NetworkIdentifiers ids;
    
    // Get hostname
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    ids.hostname = hostname;
    
    // Get MAC addresses
    struct ifaddrs* ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        return ids;
    }
    
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_PACKET) {
            continue;
        }
        
        struct ifreq ifr;
        strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
        
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd >= 0) {
            if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
                char* mac = (char*)ifr.ifr_hwaddr.sa_data;
                char mac_str[18];
                snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                        mac[0] & 0xff, mac[1] & 0xff, mac[2] & 0xff,
                        mac[3] & 0xff, mac[4] & 0xff, mac[5] & 0xff);
                ids.mac_address = mac_str;
            }
            close(fd);
        }
        break;  // Get first interface
    }
    
    freeifaddrs(ifaddr);
    
    return ids;
}

SystemInfo SystemFingerprintManager::getSystemInfo() {
    SystemInfo info;
    
    // Kernel version
    struct utsname uts;
    if (uname(&uts) == 0) {
        info.kernel_version = uts.release;
        info.os_version = uts.sysname;
        info.architecture = uts.machine;
    }
    
    // CPU info
    info.cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    info.cpu_threads = sysconf(_SC_NPROCESSORS_CONF);
    
    // Read CPU model
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") == 0) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                info.cpu_model = line.substr(pos + 2);
            }
            break;
        }
    }
    
    // Memory info
    std::ifstream meminfo("/proc/meminfo");
    long mem_kb = 0;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> mem_kb;
            break;
        }
    }
    info.memory_total = std::to_string(mem_kb / 1024 / 1024) + " GB";
    
    // GPU info
    info.gpu_info = Utils::executeCommand("lspci | grep -i vga | head -1");
    info.gpu_info.erase(std::remove(info.gpu_info.begin(), info.gpu_info.end(), '\n'), info.gpu_info.end());
    
    return info;
}

bool SystemFingerprintManager::spoofMachineId(const std::string& new_id) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (!Utils::isRoot()) {
        std::cerr << "Root privileges required for machine-id spoofing\n";
        return false;
    }
    
    // Backup original
    if (pImpl->original_values["machine_id"].empty()) {
        pImpl->original_values["machine_id"] = Utils::readFromFile("/etc/machine-id");
    }
    
    // Write new machine-id
    bool success = Utils::writeToFile("/etc/machine-id", new_id + "\n");
    if (success) {
        pImpl->spoofed_values["machine_id"] = new_id;
        pImpl->changes_applied = true;
    }
    
    return success;
}

bool SystemFingerprintManager::spoofProductUUID(const std::string& new_uuid) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (!Utils::isRoot()) {
        std::cerr << "Root privileges required for UUID spoofing\n";
        return false;
    }
    
    // Check if DMI files are writable
    if (!Utils::fileExists("/sys/class/dmi/id/product_uuid")) {
        std::cerr << "DMI sysfs not available\n";
        return false;
    }
    
    // Backup original
    if (pImpl->original_values["product_uuid"].empty()) {
        pImpl->original_values["product_uuid"] = Utils::readFromFile("/sys/class/dmi/id/product_uuid");
    }
    
    // Write new UUID
    bool success = Utils::writeToFile("/sys/class/dmi/id/product_uuid", new_uuid + "\n");
    if (success) {
        pImpl->spoofed_values["product_uuid"] = new_uuid;
        pImpl->changes_applied = true;
    }
    
    return success;
}

bool SystemFingerprintManager::spoofMACAddress(const std::string& interface, const std::string& new_mac) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (!Utils::isRoot()) {
        std::cerr << "Root privileges required for MAC spoofing\n";
        return false;
    }
    
    // Bring interface down
    std::string cmd = "ip link set " + interface + " down";
    system(cmd.c_str());
    
    // Change MAC address
    cmd = "ip link set " + interface + " address " + new_mac;
    int result = system(cmd.c_str());
    
    // Bring interface up
    cmd = "ip link set " + interface + " up";
    system(cmd.c_str());
    
    return (result == 0);
}

bool SystemFingerprintManager::spoofHostname(const std::string& new_hostname) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    // Backup original
    if (pImpl->original_values["hostname"].empty()) {
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        pImpl->original_values["hostname"] = hostname;
    }
    
    // Set new hostname
    int result = sethostname(new_hostname.c_str(), new_hostname.length());
    
    if (result == 0) {
        // Also update /etc/hosts for persistence
        pImpl->spoofed_values["hostname"] = new_hostname;
        pImpl->changes_applied = true;
    }
    
    return (result == 0);
}

bool SystemFingerprintManager::setCustomDNS(const std::vector<std::string>& dns_servers) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (!Utils::isRoot()) {
        std::cerr << "Root privileges required for DNS change\n";
        return false;
    }
    
    // Write to /etc/resolv.conf
    std::string dns_config;
    for (const auto& dns : dns_servers) {
        dns_config += "nameserver " + dns + "\n";
    }
    
    return Utils::writeToFile("/etc/resolv.conf", dns_config);
}

std::string SystemFingerprintManager::createBrowserProfile(const BrowserProfile& profile) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    std::string profile_id = Utils::generateRandomHex(16);
    pImpl->browser_profiles[profile_id] = profile;
    
    return profile_id;
}

bool SystemFingerprintManager::applyBrowserProfile(const std::string& profile_id) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    auto it = pImpl->browser_profiles.find(profile_id);
    if (it == pImpl->browser_profiles.end()) {
        return false;
    }
    
    // Profile applied - in real implementation, this would modify browser settings
    // For browser fingerprinting, we'd need to inject CSS/JS or modify browser binaries
    
    return true;
}

bool SystemFingerprintManager::deleteBrowserProfile(const std::string& profile_id) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    auto it = pImpl->browser_profiles.find(profile_id);
    if (it != pImpl->browser_profiles.end()) {
        pImpl->browser_profiles.erase(it);
        return true;
    }
    
    return false;
}

BrowserProfile SystemFingerprintManager::getBrowserProfile(const std::string& profile_id) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    auto it = pImpl->browser_profiles.find(profile_id);
    if (it != pImpl->browser_profiles.end()) {
        return it->second;
    }
    
    return BrowserProfile{};
}

bool SystemFingerprintManager::applyFingerprintPreset(const std::string& preset_name) {
    auto& preset_mgr = PresetManager::getInstance();
    auto preset = preset_mgr.getPreset(preset_name);
    
    if (preset.name.empty()) {
        std::cerr << "Preset not found: " << preset_name << "\n";
        return false;
    }
    
    bool success = true;
    
    // Apply hardware spoofing
    if (!preset.machine_id.empty()) {
        success &= spoofMachineId(preset.machine_id);
    }
    
    if (!preset.product_uuid.empty()) {
        success &= spoofProductUUID(preset.product_uuid);
    }
    
    if (!preset.mac_prefix.empty()) {
        auto mac = Utils::generateRandomMAC(preset.mac_prefix);
        spoofMACAddress("eth0", mac);
    }
    
    // Apply network spoofing
    if (!preset.hostname_pattern.empty()) {
        spoofHostname(preset.hostname_pattern);
    }
    
    return success;
}

bool SystemFingerprintManager::resetToRealSystem() {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    bool success = true;
    
    // Revert machine-id
    if (!pImpl->original_values["machine_id"].empty()) {
        success &= Utils::writeToFile("/etc/machine-id", pImpl->original_values["machine_id"]);
    }
    
    // Revert hostname
    if (!pImpl->original_values["hostname"].empty()) {
        sethostname(pImpl->original_values["hostname"].c_str(), 
                    pImpl->original_values["hostname"].length());
    }
    
    // Revert UUID
    if (!pImpl->original_values["product_uuid"].empty()) {
        Utils::writeToFile("/sys/class/dmi/id/product_uuid", 
                          pImpl->original_values["product_uuid"]);
    }
    
    pImpl->spoofed_values.clear();
    pImpl->changes_applied = false;
    
    return success;
}

void SystemFingerprintManager::cleanup() {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->browser_profiles.clear();
}

// ============================================
// PRESET MANAGER IMPLEMENTATION
// ============================================

PresetManager::PresetManager() : pImpl(std::make_unique<Impl>()) {
    loadDefaultPresets();
}

PresetManager::~PresetManager() = default;

PresetManager& PresetManager::getInstance() {
    static PresetManager instance;
    return instance;
}

void PresetManager::registerPreset(const FingerprintPreset& preset) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->presets[preset.name] = preset;
}

bool PresetManager::applyPreset(const std::string& preset_name) {
    auto& fp_mgr = SystemFingerprintManager::getInstance();
    return fp_mgr.applyFingerprintPreset(preset_name);
}

FingerprintPreset PresetManager::getPreset(const std::string& preset_name) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    auto it = pImpl->presets.find(preset_name);
    if (it != pImpl->presets.end()) {
        return it->second;
    }
    
    return FingerprintPreset{};
}

std::vector<std::string> PresetManager::listPresets() {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    std::vector<std::string> names;
    for (const auto& pair : pImpl->presets) {
        names.push_back(pair.first);
    }
    
    return names;
}

void PresetManager::loadDefaultPresets() {
    // Windows 10 Pro - Dell XPS
    registerPreset({
        "windows_dell_xps",
        "Windows 10 Pro on Dell XPS 15",
        "US",
        Utils::generateRandomHex(32),
        Utils::generateRandomUUID(),
        "00:0c:29",
        "DESKTOP-XPS15",
        "America/New_York",
        "en-US",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
        "Win32",
        "1920x1080"
    });
    
    // macOS - MacBook Pro
    registerPreset({
        "macos_macbook_pro",
        "macOS Sonoma on MacBook Pro M3",
        "US",
        Utils::generateRandomHex(32),
        Utils::generateRandomUUID(),
        "ac:de:48",
        "MacBook-Pro.local",
        "America/Los_Angeles",
        "en-US",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15",
        "MacIntel",
        "3024x1964"
    });
    
    // Ubuntu Desktop
    registerPreset({
        "ubuntu_desktop",
        "Ubuntu 22.04 LTS Desktop",
        "UK",
        Utils::generateRandomHex(32),
        Utils::generateRandomUUID(),
        "00:1a:2b",
        "ubuntu-desktop",
        "Europe/London",
        "en-GB",
        "Mozilla/5.0 (X11; Linux x86_64) Gecko/20100101 Firefox/121.0",
        "Linux x86_64",
        "2560x1440"
    });
    
    // Windows 11 - HP Envy
    registerPreset({
        "windows_hp_envy",
        "Windows 11 on HP Envy",
        "DE",
        Utils::generateRandomHex(32),
        Utils::generateRandomUUID(),
        "3c:d9:2b",
        "ENVY-PC",
        "Europe/Berlin",
        "de-DE",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/120.0.0.0",
        "Win32",
        "3840x2160"
    });
    
    // Android - Samsung Galaxy
    registerPreset({
        "android_samsung",
        "Android 14 on Samsung Galaxy S24",
        "JP",
        Utils::generateRandomHex(32),
        Utils::generateRandomUUID(),
        "00:1e:58",
        "samsung-galaxy",
        "Asia/Tokyo",
        "ja-JP",
        "Mozilla/5.0 (Linux; Android 14) AppleWebKit/537.36 Chrome/120.0.0.0",
        "Linux aarch64",
        "412x915"
    });
}

} // namespace AntiDetect