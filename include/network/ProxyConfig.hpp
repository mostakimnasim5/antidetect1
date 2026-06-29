#pragma once

/**
 * ProxyConfig - IP/Proxy Management with GeoIP, Timezone, GPS & Language Sync
 * 
 * Features:
 * 1. IP/Proxy setup before profile creation
 * 2. GeoIP detection (country, city, ISP)
 * 3. Timezone detection from IP
 * 4. GPS coordinates unique per profile
 * 5. Language auto-sync with country
 * 6. Synchronized Identity (IP-GPS-Language-Timezone)
 */

#include "../VirtualPhonePro.hpp"
#include <string>
#include <map>
#include <optional>
#include <set>

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
    std::string language;         // en-US, bn-BD, etc.
    double latitude;
    double longitude;
    std::string postalCode;
    std::string region;
};

// Synchronized Device Identity
struct DeviceIdentity {
    std::string ip;
    std::string countryCode;
    std::string countryName;
    std::string city;
    std::string isp;
    std::string timezone;
    std::string language;          // Android locale
    std::string locale;            // en_US, bn_BD, etc.
    double latitude;                // Unique per profile
    double longitude;              // Unique per profile
    std::string dns;              // DNS servers
};

// Country profile
struct CountryProfile {
    std::string countryCode;
    std::string countryName;
    std::string primaryLanguage;
    std::string timezone;
    std::string currency;
    std::string locale;            // Android locale
    double latMin, latMax;         // GPS bounds
    double lonMin, lonMax;
    std::vector<std::string> mobileCarriers;
    
    // Generate unique coordinates within country
    double generateLatitude() const;
    double generateLongitude() const;
};

// Proxy configuration
struct ProxyConfigData {
    ProxyType type;
    std::string host;
    int port;
    std::string username;
    std::string password;
    bool authenticated;
    GeoIPInfo geoIP;
    int timeout;
    int retries;
    bool rotateOnError;
};

// Timezone with offset info
struct TimezoneInfo {
    std::string timezone;
    std::string displayName;
    int utcOffset;
    std::string abbreviation;
    bool daylightSaving;
};

class ProxyConfig {
public:
    static ProxyConfig& getInstance();
    
    // ============================================
    // IP/PROXY MANAGEMENT
    // ============================================
    bool isEnabled() const;
    void enable();
    void disable();
    
    bool setProxy(
        const std::string& host,
        int port,
        ProxyType type = ProxyType::HTTPS,
        const std::string& username = "",
        const std::string& password = ""
    );
    
    bool setProxyFromURL(const std::string& proxyURL);
    void clearProxy();
    std::optional<ProxyConfigData> getCurrentProxy() const;
    bool testProxy();
    
    // ============================================
    // SYNCHRONIZED IDENTITY (IP-GPS-LANGUAGE-TIMEZONE)
    // ============================================
    
    // With IP: Sync all identity with proxy IP
    DeviceIdentity createIdentityWithProxy(const std::string& proxyIP);
    
    // Without IP: Create unique identity per country
    DeviceIdentity createUniqueIdentity(const std::string& countryCode);
    
    // Get supported countries
    std::vector<CountryProfile> getSupportedCountries();
    
    // Get country info
    CountryProfile getCountryProfile(const std::string& countryCode);
    
    // ============================================
    // LANGUAGE MANAGEMENT
    // ============================================
    
    // Get language for country
    std::string getLanguageForCountry(const std::string& countryCode);
    
    // Get Android locale for country
    std::string getAndroidLocale(const std::string& countryCode);
    
    // Set custom language
    void setLanguage(const std::string& language);
    std::string getLanguage() const;
    
    // ============================================
    // GPS COORDINATES
    // ============================================
    
    // Generate unique GPS within country bounds
    GeoIPInfo generateUniqueGPS(const std::string& countryCode);
    
    // Generate unique IP within country (for root IP)
    std::string generateUniqueIP(const std::string& countryCode);
    
    // ============================================
    // GEOIP DETECTION
    // ============================================
    GeoIPInfo detectFromIP(const std::string& ip);
    GeoIPInfo detectCurrentIP();
    std::string getCountryFlag(const std::string& countryCode);
    
    // ============================================
    // TIMEZONE
    // ============================================
    TimezoneInfo detectTimezone(const std::string& ip);
    TimezoneInfo getTimezoneFromCountry(const std::string& countryCode);
    std::vector<std::string> getAvailableTimezones();
    void setTimezone(const std::string& timezone);
    
    // ============================================
    // DEVICE PROFILE INTEGRATION
    // ============================================
    std::string getPrimaryIP() const;
    TimezoneInfo getDeviceTimezone() const;
    std::string getDeviceCountry() const;
    void applyToProfile(std::map<std::string, std::string>& profileData);
    
private:
    ProxyConfig();
    ~ProxyConfig();
    
    void initializeCountryProfiles();
    
    std::string fetchURL(const std::string& url);
    bool parseProxyURL(const std::string& url, ProxyConfigData& config);
    std::string ipToString(uint32_t ip);
    uint32_t stringToIP(const std::string& ip);
    bool isValidIP(const std::string& ip);
    
    // Identity state
    bool m_enabled;
    std::optional<ProxyConfigData> m_currentProxy;
    std::optional<GeoIPInfo> m_currentGeoIP;
    std::string m_currentLanguage;
    
    // Device settings
    TimezoneInfo m_deviceTimezone;
    std::string m_primaryIP;
    
    // Country database
    std::map<std::string, CountryProfile> m_countries;
    std::map<std::string, std::string> m_countryTimezoneMap;
    std::map<std::string, TimezoneInfo> m_timezoneDatabase;
    
    // Used coordinates to ensure uniqueness
    std::set<std::string> m_usedCoordinates;
    std::set<std::string> m_usedIPs;
};

class TimezoneHelper {
public:
    static time_t getTimeInTimezone(const std::string& timezone);
    static std::string getFormattedTime(const std::string& timezone, const std::string& format = "%Y-%m-%d %H:%M:%S");
    static int getUTCOffset(const std::string& timezone);
    static bool isDST(const std::string& timezone);
};

} // namespace VirtualPhonePro
