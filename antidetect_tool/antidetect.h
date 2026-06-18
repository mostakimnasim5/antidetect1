#ifndef ANTIDETECT_H
#define ANTIDETECT_H

#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>

namespace Antidetect {

struct BrowserFingerprint {
    std::string user_agent;
    std::string platform;
    std::string language;
    std::string screen_resolution;
    std::string timezone;
    std::string canvas_hash;
    std::string webgl_vendor;
    std::string webgl_renderer;
};

struct SystemInfo {
    std::string hostname;
    std::string os_version;
    std::string cpu_cores;
    std::string memory_info;
    std::string mac_address;
};

class AntidetectEngine {
public:
    AntidetectEngine();
    ~AntidetectEngine();
    
    // Browser fingerprint spoofing
    void generateRandomUserAgent();
    void generateRandomPlatform();
    void generateRandomLanguage();
    void generateRandomScreen();
    void generateRandomTimezone();
    void generateCanvasFingerprint();
    void generateWebGLFingerprint();
    
    // System info spoofing
    void generateSystemInfo();
    void spoofHostname(const std::string& hostname);
    std::string generateMACAddress();
    
    // All fingerprints
    BrowserFingerprint getBrowserFingerprint();
    SystemInfo getSystemInfo();
    
    // Export/Import profiles
    void saveProfile(const std::string& filename);
    bool loadProfile(const std::string& filename);
    
    // Testing utilities
    void printAllFingerprints();
    std::string generateRandomString(int length);

private:
    BrowserFingerprint browser_fp;
    SystemInfo system_info;
    std::mt19937 rng;
    
    std::string randomChoice(const std::vector<std::string>& choices);
    std::string generateHash(const std::string& input);
};

} // namespace Antidetect

#endif // ANTIDETECT_H