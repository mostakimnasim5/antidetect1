#pragma once

/**
 * ProxyConfig - IP/Proxy Management with GeoIP & Timezone Detection
 * 
 * Features:
 * 1. IP/Proxy setup before profile creation
 * 2. GeoIP detection (country, city, ISP)
 * 3. Timezone detection from IP
 * 4. Associate proxy with device profile
 * 5. Skip option for no proxy
 */

#include "../VirtualPhonePro.hpp"
#include <string>
#include <map>
#include <optional>

namespace VirtualPhonePro {

// Proxy types supported
enum class ProxyType {
    NONE,
    HTTP,
    HTTPS,
    SOCKS4,
    SOCKS5,
    SSH_TUNNEL,
    VPN
};

// IP information from GeoIP
struct GeoIPInfo {
    std::string ip;
    std::string countryCode;      // BD, US, UK, etc.
    std::string countryName;      // Bangladesh, United States, etc.
    std::string city;             // Dhaka, New York, etc.
    std::string isp;              // ISP name
    std::string org;              // Organization
    std::string timezone;         // Asia/Dhaka, America/New_York, etc.
    std::string currency;         // BDT, USD, GBP, etc.
    double latitude;
    double longitude;
    std::string postalCode;
    std::string region;
};

// Proxy configuration
struct ProxyConfigData {
    ProxyType type;
    std::string host;
    int port;
    std::string username;          // Optional
    std::string password;         // Optional
    bool authenticated;
    
    // GeoIP info (detected or manual)
    GeoIPInfo geoIP;
    
    // Connection settings
    int timeout;                  // seconds
    int retries;
    bool rotateOnError;
};

// Timezone with offset info
struct TimezoneInfo {
    std::string timezone;         // Asia/Dhaka
    std::string displayName;     // Bangladesh (GMT+6)
    int utcOffset;               // +6 hours
    std::string abbreviation;     // BDT
    bool daylightSaving;          // DST active
};

class ProxyConfig {
public:
    static ProxyConfig& getInstance();
    
    // ============================================
    // IP/PROXY MANAGEMENT
    // ============================================
    
    // Check if proxy is enabled
    bool isEnabled() const;
    
    // Enable/disable proxy
    void enable();
    void disable();
    
    // Set proxy manually
    bool setProxy(
        const std::string& host,
        int port,
        ProxyType type = ProxyType::HTTPS,
        const std::string& username = "",
        const std::string& password = ""
    );
    
    // Set proxy from GeoIP URL (fetch proxy)
    bool setProxyFromURL(const std::string& proxyURL);
    
    // Clear proxy
    void clearProxy();
    
    // Get current proxy config
    std::optional<ProxyConfigData> getCurrentProxy() const;
    
    // Test proxy connection
    bool testProxy();
    bool testProxy(const ProxyConfigData& proxy);
    
    // ============================================
    // GEOIP DETECTION
    // ============================================
    
    // Detect info from IP address
    GeoIPInfo detectFromIP(const std::string& ip);
    
    // Detect from current public IP
    GeoIPInfo detectCurrentIP();
    
    // Get country flag emoji
    std::string getCountryFlag(const std::string& countryCode);
    
    // ============================================
    // TIMEZONE DETECTION
    // ============================================
    
    // Detect timezone from IP
    TimezoneInfo detectTimezone(const std::string& ip);
    
    // Get timezone from country code
    TimezoneInfo getTimezoneFromCountry(const std::string& countryCode);
    
    // Get all available timezones
    std::vector<std::string> getAvailableTimezones();
    
    // Set timezone manually
    void setTimezone(const std::string& timezone);
    
    // ============================================
    // DEVICE PROFILE INTEGRATION
    // ============================================
    
    // Get the primary IP for device
    std::string getPrimaryIP() const;
    
    // Get timezone for device
    TimezoneInfo getDeviceTimezone() const;
    
    // Get country code for device
    std::string getDeviceCountry() const;
    
    // Apply proxy to profile creation
    void applyToProfile(std::map<std::string, std::string>& profileData);
    
    // ============================================
    // BUILT-IN TIMEZONE DATABASE
    // ============================================
    
    // Common timezones with offsets
    static std::map<std::string, TimezoneInfo> getTimezoneDatabase();
    
    // Common country to timezone mapping
    static std::map<std::string, std::string> getCountryTimezoneMap();
    
private:
    ProxyConfig();
    ~ProxyConfig();
    
    // Internal helpers
    std::string fetchURL(const std::string& url);
    bool parseProxyURL(const std::string& url, ProxyConfigData& config);
    std::string ipToString(uint32_t ip);
    uint32_t stringToIP(const std::string& ip);
    
    // Validate IP address
    bool isValidIP(const std::string& ip);
    
    // Current proxy state
    bool m_enabled;
    std::optional<ProxyConfigData> m_currentProxy;
    
    // Cached GeoIP info
    std::optional<GeoIPInfo> m_currentGeoIP;
    
    // Device settings
    TimezoneInfo m_deviceTimezone;
    std::string m_primaryIP;
    
    // Built-in timezone database
    std::map<std::string, TimezoneInfo> m_timezoneDatabase;
    std::map<std::string, std::string> m_countryTimezoneMap;
};

// ============================================
// TIMEZONE HELPER FUNCTIONS
// ============================================

class TimezoneHelper {
public:
    // Get current time in specific timezone
    static time_t getTimeInTimezone(const std::string& timezone);
    
    // Get formatted time for timezone
    static std::string getFormattedTime(const std::string& timezone, const std::string& format = "%Y-%m-%d %H:%M:%S");
    
    // Get UTC offset for timezone
    static int getUTCOffset(const std::string& timezone);
    
    // Check if DST is active
    static bool isDST(const std::string& timezone);
};

} // namespace VirtualPhonePro
