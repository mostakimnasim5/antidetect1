/**
 * Advanced Fingerprint Generator - Implementation
 */

#include "advanced_fingerprint.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <sys/utsname.h>

namespace AntiDetect {

// ============================================
// USER AGENT DATABASE
// ============================================

struct UserAgentDatabase {
    std::vector<std::string> windows_chrome = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Windows NT 11.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36"
    };
    
    std::vector<std::string> windows_firefox = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:121.0) Gecko/20100101 Firefox/121.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:120.0) Gecko/20100101 Firefox/120.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:122.0) Gecko/20100101 Firefox/122.0"
    };
    
    std::vector<std::string> windows_edge = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36 Edg/119.0.0.0"
    };
    
    std::vector<std::string> macos_safari = {
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Safari/605.1.15",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 13_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Safari/605.1.15",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_2) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15"
    };
    
    std::vector<std::string> macos_chrome = {
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 13_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    };
    
    std::vector<std::string> linux_firefox = {
        "Mozilla/5.0 (X11; Linux x86_64; rv:121.0) Gecko/20100101 Firefox/121.0",
        "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:121.0) Gecko/20100101 Firefox/121.0",
        "Mozilla/5.0 (X11; Linux x86_64; rv:120.0) Gecko/20100101 Firefox/120.0"
    };
    
    std::vector<std::string> linux_chrome = {
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36"
    };
    
    std::vector<std::string> android_chrome = {
        "Mozilla/5.0 (Linux; Android 14; SM-S918B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.6099.43 Mobile Safari/537.36",
        "Mozilla/5.0 (Linux; Android 14; Pixel 8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
        "Mozilla/5.0 (Linux; Android 14; SM-A546B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36"
    };
    
    std::vector<std::string> ios_safari = {
        "Mozilla/5.0 (iPhone; CPU iPhone OS 17_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Mobile/15E148 Safari/604.1",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 17_1_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Mobile/15E148 Safari/604.1",
        "Mozilla/5.0 (iPad; CPU OS 17_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Mobile/15E148 Safari/604.1"
    };
};

// Profile configurations
struct ProfileConfig {
    std::string name;
    std::string user_agent;
    std::string platform;
    std::string screen_resolution;
    std::string color_depth;
    int timezone_offset;
    std::string language;
    std::string accept_language;
    std::string webgl_vendor;
    std::string webgl_renderer;
    std::vector<std::string> fonts;
};

std::vector<ProfileConfig> getProfileConfigs() {
    return {
        // Windows 10 - Dell XPS 15
        {
            "Windows 10 Dell XPS",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
            "Win32",
            "1920x1080",
            "24",
            -5,
            "en-US",
            "en-US,en;q=0.9",
            "Intel Inc.",
            "Intel Iris OpenGL Engine",
            {"Segoe UI", "Arial", "Calibri", "Cambria", "Consolas"}
        },
        // macOS - MacBook Pro M3
        {
            "macOS MacBook Pro",
            "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15",
            "MacIntel",
            "3024x1964",
            "30",
            -8,
            "en-US",
            "en-US,en;q=0.9",
            "Apple Inc.",
            "Apple M3 Pro",
            {"SF Pro Display", "SF Pro Text", "Helvetica Neue", "Arial"}
        },
        // Ubuntu Desktop
        {
            "Ubuntu Desktop",
            "Mozilla/5.0 (X11; Linux x86_64; rv:121.0) Gecko/20100101 Firefox/121.0",
            "Linux x86_64",
            "2560x1440",
            "24",
            0,
            "en-GB",
            "en-GB,en;q=0.9",
            "Intel Inc.",
            "Intel Iris OpenGL Engine",
            {"Ubuntu", "Cantarell", "DejaVu Sans", "Liberation Sans"}
        },
        // Windows 11 - HP Envy
        {
            "Windows 11 HP Envy",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0",
            "Win32",
            "3840x2160",
            "30",
            1,
            "de-DE",
            "de-DE,de;q=0.9,en;q=0.8",
            "NVIDIA Corporation",
            "NVIDIA GeForce RTX 4070/PCIe/SSE2",
            {"Segoe UI Variable", "Arial", "Cambria", "Consolas"}
        },
        // Android - Samsung Galaxy S24
        {
            "Android Samsung",
            "Mozilla/5.0 (Linux; Android 14; SM-S918B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.6099.43 Mobile Safari/537.36",
            "Linux aarch64",
            "412x915",
            "24",
            9,
            "ja-JP",
            "ja-JP,en-US;q=0.9",
            "ARM",
            "Mali-G710 MC14",
            {"Roboto", "Noto Sans JP", "Samsung Sans"}
        }
    };
}

// ============================================
// IMPLEMENTATION
// ============================================

AdvancedFingerprintGenerator::AdvancedFingerprintGenerator()
    : rng(std::random_device{}()) {}

AdvancedFingerprintGenerator::~AdvancedFingerprintGenerator() {}

AdvancedFingerprintGenerator& AdvancedFingerprintGenerator::getInstance() {
    static AdvancedFingerprintGenerator instance;
    return instance;
}

std::string AdvancedFingerprintGenerator::generateRandomUserAgent(const std::string& os_type) {
    static UserAgentDatabase ua_db;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    if (os_type == "windows_chrome") {
        std::uniform_int_distribution<> dis(0, ua_db.windows_chrome.size() - 1);
        return ua_db.windows_chrome[dis(gen)];
    } else if (os_type == "windows_firefox") {
        std::uniform_int_distribution<> dis(0, ua_db.windows_firefox.size() - 1);
        return ua_db.windows_firefox[dis(gen)];
    } else if (os_type == "macos_safari") {
        std::uniform_int_distribution<> dis(0, ua_db.macos_safari.size() - 1);
        return ua_db.macos_safari[dis(gen)];
    } else if (os_type == "linux_firefox") {
        std::uniform_int_distribution<> dis(0, ua_db.linux_firefox.size() - 1);
        return ua_db.linux_firefox[dis(gen)];
    } else if (os_type == "android_chrome") {
        std::uniform_int_distribution<> dis(0, ua_db.android_chrome.size() - 1);
        return ua_db.android_chrome[dis(gen)];
    } else if (os_type == "ios_safari") {
        std::uniform_int_distribution<> dis(0, ua_db.ios_safari.size() - 1);
        return ua_db.ios_safari[dis(gen)];
    }
    
    // Default
    return ua_db.windows_chrome[0];
}

BrowserProfile AdvancedFingerprintGenerator::generateProfile(const std::string& preset_name) {
    BrowserProfile profile;
    
    auto configs = getProfileConfigs();
    ProfileConfig config;
    bool found = false;
    
    for (const auto& cfg : configs) {
        if (cfg.name.find(preset_name) != std::string::npos) {
            config = cfg;
            found = true;
            break;
        }
    }
    
    if (!found) {
        config = configs[0];  // Default to first config
    }
    
    profile.profile_id = Utils::generateRandomHex(16);
    profile.user_agent = config.user_agent;
    profile.platform = config.platform;
    profile.accept_language = config.accept_language;
    profile.accept_encoding = "gzip, deflate, br";
    profile.screen_resolution = config.screen_resolution;
    profile.color_depth = config.color_depth;
    profile.timezone = std::to_string(config.timezone_offset * 60);
    profile.webgl_vendor = config.webgl_vendor;
    profile.webgl_renderer = config.webgl_renderer;
    profile.hardware_acceleration = true;
    profile.webrtc_enabled = false;
    
    // Generate canvas fingerprint hash
    std::string canvas_data = config.user_agent + config.platform + profile.screen_resolution;
    profile.canvas_fingerprint = Utils::hashString(canvas_data);
    
    // Generate fonts hash
    std::string fonts_str;
    for (const auto& font : config.fonts) {
        fonts_str += font + ",";
    }
    profile.fonts_hash = Utils::hashString(fonts_str);
    
    // Generate audio fingerprint
    profile.audio_fingerprint = Utils::generateRandomHex(32);
    
    return profile;
}

CanvasFingerprint AdvancedFingerprintGenerator::generateCanvasFingerprint(const std::string& profile_type) {
    CanvasFingerprint fp;
    
    static const std::vector<std::string> elements = {
        "Hello World!", "Canvas Test", "Anti-Detect", "Fingerprint"
    };
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, elements.size() - 1);
    
    fp.drawn_elements.push_back(elements[dis(gen)]);
    fp.renderer_info = "2d";
    
    // Generate hash from drawn elements
    std::string data;
    for (const auto& elem : fp.drawn_elements) {
        data += elem;
    }
    fp.hash = Utils::hashString(data);
    
    return fp;
}

AudioFingerprint AdvancedFingerprintGenerator::generateAudioFingerprint() {
    AudioFingerprint fp;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(44100, 96000);
    
    fp.sample_rate = dis(gen);
    fp.channels = 2;
    fp.audio_context = "AudioContext";
    
    // Generate audio fingerprint hash
    std::string data = std::to_string(fp.sample_rate) + std::to_string(fp.channels);
    fp.hash = Utils::hashString(data);
    
    return fp;
}

WebGLFingerprint AdvancedFingerprintGenerator::generateWebGLFingerprint(const std::string& profile_type) {
    WebGLFingerprint fp;
    
    if (profile_type == "windows") {
        fp.vendor = "Intel Inc.";
        fp.renderer = "Intel Iris OpenGL Engine";
        fp.unmasked_vendor = "Intel Inc.";
        fp.unmasked_renderer = "Intel(R) Iris(R) Xe Graphics";
    } else if (profile_type == "macos") {
        fp.vendor = "Apple Inc.";
        fp.renderer = "Apple M3";
        fp.unmasked_vendor = "Apple Inc.";
        fp.unmasked_renderer = "Apple M3 Pro";
    } else if (profile_type == "linux") {
        fp.vendor = "Intel Inc.";
        fp.renderer = "Mesa/X.org";
        fp.unmasked_vendor = "Intel Open Source Technology Center";
        fp.unmasked_renderer = "Mesa Intel(R) UHD Graphics";
    } else if (profile_type == "android") {
        fp.vendor = "ARM";
        fp.renderer = "Mali-G710";
        fp.unmasked_vendor = "ARM";
        fp.unmasked_renderer = "Mali-G710 MC14";
    }
    
    fp.version = "WebGL 2.0";
    fp.shading_language_version = "WebGL GLSL ES 3.00";
    fp.extensions = "WEBGL_debug_renderer_info,WEBGL_debug_shaders";
    
    return fp;
}

FontFingerprint AdvancedFingerprintGenerator::generateFontFingerprint() {
    FontFingerprint fp;
    
    // Get system fonts
    fp.installed_fonts = getSystemFonts();
    fp.fallback_count = fp.installed_fonts.size();
    
    // Generate fonts hash
    std::string data;
    for (const auto& font : fp.installed_fonts) {
        data += font;
    }
    fp.fonts_hash = Utils::hashString(data);
    
    return fp;
}

std::vector<std::string> AdvancedFingerprintGenerator::getSystemFonts() {
    std::vector<std::string> fonts;
    
    // Read fonts from common locations
    std::ifstream fontconfig("/etc/fonts/fonts.conf");
    if (fontconfig.is_open()) {
        std::string line;
        while (std::getline(fontconfig, line)) {
            if (line.find("<font ") != std::string::npos) {
                size_t start = line.find(">");
                size_t end = line.find("</", start);
                if (start != std::string::npos && end != std::string::npos) {
                    fonts.push_back(line.substr(start + 1, end - start - 1));
                }
            }
        }
    }
    
    // Common fallback fonts
    if (fonts.empty()) {
        fonts = {"DejaVu Sans", "Liberation Sans", "Noto Sans", "Ubuntu", "Roboto"};
    }
    
    return fonts;
}

void AdvancedFingerprintGenerator::randomizeTiming() {
    // Add random delay to prevent timing attacks
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 100);
    
    std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));
}

void AdvancedFingerprintGenerator::addTimingNoise() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1, 10);
    
    // Add small random delays
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
}

// ============================================
// PROFILE DATABASE MANAGER
// ============================================

ProfileDatabaseManager::ProfileDatabaseManager() : db_path("profiles.db") {}

ProfileDatabaseManager::~ProfileDatabaseManager() {}

ProfileDatabaseManager& ProfileDatabaseManager::getInstance() {
    static ProfileDatabaseManager instance;
    return instance;
}

bool ProfileDatabaseManager::saveDatabase(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Anti-Detect Profile Database v1.0\n";
    file << "# Auto-generated - Do not edit manually\n\n";
    
    for (const auto& profile : database.saved_profiles) {
        file << "[Profile]\n";
        file << "id=" << profile.profile_id << "\n";
        file << "user_agent=" << profile.user_agent << "\n";
        file << "platform=" << profile.platform << "\n";
        file << "screen=" << profile.screen_resolution << "\n";
        file << "timezone=" << profile.timezone << "\n";
        file << "webgl_vendor=" << profile.webgl_vendor << "\n";
        file << "webgl_renderer=" << profile.webgl_renderer << "\n";
        file << "\n";
    }
    
    file.close();
    return true;
}

bool ProfileDatabaseManager::loadDatabase(const std::string& path) {
    db_path = path;
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    database.saved_profiles.clear();
    BrowserProfile current_profile;
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line == "[Profile]") {
            if (!current_profile.profile_id.empty()) {
                database.saved_profiles.push_back(current_profile);
            }
            current_profile = BrowserProfile{};
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "id") current_profile.profile_id = value;
            else if (key == "user_agent") current_profile.user_agent = value;
            else if (key == "platform") current_profile.platform = value;
            else if (key == "screen") current_profile.screen_resolution = value;
            else if (key == "timezone") current_profile.timezone = value;
            else if (key == "webgl_vendor") current_profile.webgl_vendor = value;
            else if (key == "webgl_renderer") current_profile.webgl_renderer = value;
        }
    }
    
    if (!current_profile.profile_id.empty()) {
        database.saved_profiles.push_back(current_profile);
    }
    
    file.close();
    return true;
}

bool ProfileDatabaseManager::addProfile(const BrowserProfile& profile) {
    database.saved_profiles.push_back(profile);
    return saveDatabase(db_path);
}

bool ProfileDatabaseManager::removeProfile(const std::string& profile_id) {
    auto it = std::remove_if(database.saved_profiles.begin(), 
                            database.saved_profiles.end(),
                            [&profile_id](const BrowserProfile& p) {
                                return p.profile_id == profile_id;
                            });
    
    if (it != database.saved_profiles.end()) {
        database.saved_profiles.erase(it, database.saved_profiles.end());
        return saveDatabase(db_path);
    }
    
    return false;
}

BrowserProfile ProfileDatabaseManager::getProfile(const std::string& profile_id) {
    for (const auto& profile : database.saved_profiles) {
        if (profile.profile_id == profile_id) {
            return profile;
        }
    }
    return BrowserProfile{};
}

std::vector<BrowserProfile> ProfileDatabaseManager::listProfiles() {
    return database.saved_profiles;
}

ProfileDatabase ProfileDatabaseManager::getDatabase() {
    return database;
}

} // namespace AntiDetect