#include "antidetect.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <sys/utsname.h>
#include <unistd.h>

namespace Antidetect {

// Common User Agents for spoofing
const std::vector<std::string> USER_AGENTS = {
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 Chrome/119.0.0.0 Safari/537.36",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 Chrome/118.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:121.0) Gecko/20100101 Firefox/121.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 Safari/605.1.15",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Edge/120.0.0.0",
    "Mozilla/5.0 (X11; Ubuntu; Linux x86_64) Gecko/20100101 Firefox/121.0"
};

// Platforms
const std::vector<std::string> PLATFORMS = {
    "Win32", "MacIntel", "Linux x86_64", "Linux i686"
};

// Languages
const std::vector<std::string> LANGUAGES = {
    "en-US", "en-GB", "de-DE", "fr-FR", "es-ES", "ja-JP", "zh-CN", "ko-KR", "pt-BR", "it-IT"
};

// Screen Resolutions
const std::vector<std::string> SCREEN_RESOLUTIONS = {
    "1920x1080", "2560x1440", "1366x768", "1440x900", "1536x864", "3840x2160", "1280x720"
};

// Timezones
const std::vector<std::string> TIMEZONES = {
    "America/New_York", "America/Los_Angeles", "Europe/London", 
    "Europe/Berlin", "Asia/Tokyo", "Asia/Shanghai", "Australia/Sydney"
};

// WebGL Vendors
const std::vector<std::string> WEBGL_VENDORS = {
    "Intel Inc.", "NVIDIA Corporation", "AMD", "Apple Inc.", "VMware, Inc."
};

// WebGL Renderers
const std::vector<std::string> WEBGL_RENDERERS = {
    "Intel Iris OpenGL Engine", "NVIDIA GeForce GTX 1080/PCIe/SSE2",
    "AMD Radeon Pro 5500M", "Apple M1 Pro", "llvmpipe (LLVM 15.0.0)"
};

AntidetectEngine::AntidetectEngine() : rng(std::random_device{}()) {
    generateRandomUserAgent();
    generateRandomPlatform();
    generateRandomLanguage();
    generateRandomScreen();
    generateRandomTimezone();
    generateCanvasFingerprint();
    generateWebGLFingerprint();
    generateSystemInfo();
}

AntidetectEngine::~AntidetectEngine() {}

void AntidetectEngine::generateRandomUserAgent() {
    browser_fp.user_agent = randomChoice(USER_AGENTS);
}

void AntidetectEngine::generateRandomPlatform() {
    browser_fp.platform = randomChoice(PLATFORMS);
}

void AntidetectEngine::generateRandomLanguage() {
    browser_fp.language = randomChoice(LANGUAGES);
}

void AntidetectEngine::generateRandomScreen() {
    browser_fp.screen_resolution = randomChoice(SCREEN_RESOLUTIONS);
}

void AntidetectEngine::generateRandomTimezone() {
    browser_fp.timezone = randomChoice(TIMEZONES);
}

void AntidetectEngine::generateCanvasFingerprint() {
    std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string canvas_data;
    
    for (int i = 0; i < 32; ++i) {
        canvas_data += chars[rng() % chars.length()];
    }
    
    browser_fp.canvas_hash = generateHash(canvas_data);
}

void AntidetectEngine::generateWebGLFingerprint() {
    browser_fp.webgl_vendor = randomChoice(WEBGL_VENDORS);
    browser_fp.webgl_renderer = randomChoice(WEBGL_RENDERERS);
}

void AntidetectEngine::generateSystemInfo() {
    struct utsname sys_info;
    if (uname(&sys_info) == 0) {
        system_info.hostname = sys_info.nodename;
        system_info.os_version = std::string(sys_info.sysname) + " " + sys_info.release;
    }
    
    system_info.cpu_cores = std::to_string(sysconf(_SC_NPROCESSORS_ONLN));
    
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                std::istringstream iss(line);
                std::string key;
                long kb;
                iss >> key >> kb;
                system_info.memory_info = std::to_string(kb / 1024) + " MB";
                break;
            }
        }
    }
    
    system_info.mac_address = generateMACAddress();
}

std::string AntidetectEngine::generateMACAddress() {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (int i = 0; i < 6; ++i) {
        if (i > 0) ss << ":";
        ss << std::setw(2) << (rng() % 256);
    }
    
    return ss.str();
}

BrowserFingerprint AntidetectEngine::getBrowserFingerprint() {
    return browser_fp;
}

SystemInfo AntidetectEngine::getSystemInfo() {
    return system_info;
}

void AntidetectEngine::printAllFingerprints() {
    std::cout << "\n===========================================\n";
    std::cout << "         BROWSER FINGERPRINT\n";
    std::cout << "===========================================\n";
    std::cout << "User Agent:     " << browser_fp.user_agent << "\n";
    std::cout << "Platform:       " << browser_fp.platform << "\n";
    std::cout << "Language:       " << browser_fp.language << "\n";
    std::cout << "Screen:         " << browser_fp.screen_resolution << "\n";
    std::cout << "Timezone:       " << browser_fp.timezone << "\n";
    std::cout << "Canvas Hash:    " << browser_fp.canvas_hash << "\n";
    std::cout << "WebGL Vendor:   " << browser_fp.webgl_vendor << "\n";
    std::cout << "WebGL Renderer: " << browser_fp.webgl_renderer << "\n";
    
    std::cout << "\n===========================================\n";
    std::cout << "          SYSTEM INFORMATION\n";
    std::cout << "===========================================\n";
    std::cout << "Hostname:    " << system_info.hostname << "\n";
    std::cout << "OS Version:  " << system_info.os_version << "\n";
    std::cout << "CPU Cores:   " << system_info.cpu_cores << "\n";
    std::cout << "Memory:      " << system_info.memory_info << "\n";
    std::cout << "MAC Address: " << system_info.mac_address << "\n";
    std::cout << "===========================================\n\n";
}

void AntidetectEngine::saveProfile(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "[BrowserFingerprint]\n";
        file << "user_agent=" << browser_fp.user_agent << "\n";
        file << "platform=" << browser_fp.platform << "\n";
        file << "language=" << browser_fp.language << "\n";
        file << "screen_resolution=" << browser_fp.screen_resolution << "\n";
        file << "timezone=" << browser_fp.timezone << "\n";
        file << "canvas_hash=" << browser_fp.canvas_hash << "\n";
        file << "webgl_vendor=" << browser_fp.webgl_vendor << "\n";
        file << "webgl_renderer=" << browser_fp.webgl_renderer << "\n";
        
        file << "\n[SystemInfo]\n";
        file << "hostname=" << system_info.hostname << "\n";
        file << "os_version=" << system_info.os_version << "\n";
        file << "cpu_cores=" << system_info.cpu_cores << "\n";
        file << "memory_info=" << system_info.memory_info << "\n";
        file << "mac_address=" << system_info.mac_address << "\n";
        
        file.close();
        std::cout << "Profile saved to: " << filename << "\n";
    } else {
        std::cerr << "Error: Could not save profile!\n";
    }
}

bool AntidetectEngine::loadProfile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open profile file!\n";
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[') continue;
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "user_agent") browser_fp.user_agent = value;
            else if (key == "platform") browser_fp.platform = value;
            else if (key == "language") browser_fp.language = value;
            else if (key == "screen_resolution") browser_fp.screen_resolution = value;
            else if (key == "timezone") browser_fp.timezone = value;
            else if (key == "canvas_hash") browser_fp.canvas_hash = value;
            else if (key == "webgl_vendor") browser_fp.webgl_vendor = value;
            else if (key == "webgl_renderer") browser_fp.webgl_renderer = value;
            else if (key == "hostname") system_info.hostname = value;
            else if (key == "os_version") system_info.os_version = value;
            else if (key == "cpu_cores") system_info.cpu_cores = value;
            else if (key == "memory_info") system_info.memory_info = value;
            else if (key == "mac_address") system_info.mac_address = value;
        }
    }
    
    file.close();
    std::cout << "Profile loaded from: " << filename << "\n";
    return true;
}

std::string AntidetectEngine::randomChoice(const std::vector<std::string>& choices) {
    return choices[rng() % choices.size()];
}

std::string AntidetectEngine::generateHash(const std::string& input) {
    unsigned int hash = 5381;
    for (char c : input) {
        hash = ((hash << 5) + hash) + c;
    }
    
    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

std::string AntidetectEngine::generateRandomString(int length) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    for (int i = 0; i < length; ++i) {
        result += chars[rng() % chars.length()];
    }
    return result;
}

} // namespace Antidetect