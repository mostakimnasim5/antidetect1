/**
 * ProxyConfig - IP/Proxy Management with GeoIP & Timezone Detection
 */

#include "network/ProxyConfig.hpp"
#include "core/CryptoUtils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <curl/curl.h>

namespace VirtualPhonePro {

// Static callback for curl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

ProxyConfig::ProxyConfig() 
    : m_enabled(false), m_currentLanguage("en-US") {
    
    // Initialize country profiles
    initializeCountryProfiles();
    
    // Initialize timezone database
    m_timezoneDatabase = {
        // Asia
        {"Asia/Dhaka", {"Asia/Dhaka", "Bangladesh (GMT+6)", 6, "BDT", false}},
        {"Asia/Kolkata", {"Asia/Kolkata", "India (GMT+5:30)", 5, "IST", false}},
        {"Asia/Tokyo", {"Asia/Tokyo", "Japan (GMT+9)", 9, "JST", false}},
        {"Asia/Shanghai", {"Asia/Shanghai", "China (GMT+8)", 8, "CST", false}},
        {"Asia/Singapore", {"Asia/Singapore", "Singapore (GMT+8)", 8, "SGT", false}},
        {"Asia/Dubai", {"Asia/Dubai", "UAE (GMT+4)", 4, "GST", false}},
        {"Asia/Jakarta", {"Asia/Jakarta", "Indonesia (GMT+7)", 7, "WIB", false}},
        {"Asia/Manila", {"Asia/Manila", "Philippines (GMT+8)", 8, "PHT", false}},
        {"Asia/Kuala_Lumpur", {"Asia/Kuala_Lumpur", "Malaysia (GMT+8)", 8, "MYT", false}},
        {"Asia/Kathmandu", {"Asia/Kathmandu", "Nepal (GMT+5:45)", 5, "NPT", false}},
        {"Asia/Colombo", {"Asia/Colombo", "Sri Lanka (GMT+5:30)", 5, "IST", false}},
        
        // Europe
        {"Europe/London", {"Europe/London", "UK (GMT+0)", 0, "GMT", false}},
        {"Europe/Paris", {"Europe/Paris", "France (GMT+1)", 1, "CET", false}},
        {"Europe/Berlin", {"Europe/Berlin", "Germany (GMT+1)", 1, "CET", false}},
        {"Europe/Madrid", {"Europe/Madrid", "Spain (GMT+1)", 1, "CET", false}},
        {"Europe/Rome", {"Europe/Rome", "Italy (GMT+1)", 1, "CET", false}},
        {"Europe/Amsterdam", {"Europe/Amsterdam", "Netherlands (GMT+1)", 1, "CET", false}},
        {"Europe/Moscow", {"Europe/Moscow", "Russia (GMT+3)", 3, "MSK", false}},
        
        // America
        {"America/New_York", {"America/New_York", "USA East (GMT-5)", -5, "EST", false}},
        {"America/Los_Angeles", {"America/Los_Angeles", "USA West (GMT-8)", -8, "PST", false}},
        {"America/Chicago", {"America/Chicago", "USA Central (GMT-6)", -6, "CST", false}},
        {"America/Toronto", {"America/Toronto", "Canada (GMT-5)", -5, "EST", false}},
        {"America/Vancouver", {"America/Vancouver", "Canada West (GMT-8)", -8, "PST", false}},
        {"America/Sao_Paulo", {"America/Sao_Paulo", "Brazil (GMT-3)", -3, "BRT", false}},
        {"America/Mexico_City", {"America/Mexico_City", "Mexico (GMT-6)", -6, "CST", false}},
        
        // Africa
        {"Africa/Lagos", {"Africa/Lagos", "Nigeria (GMT+1)", 1, "WAT", false}},
        {"Africa/Nairobi", {"Africa/Nairobi", "Kenya (GMT+3)", 3, "EAT", false}},
        {"Africa/Cairo", {"Africa/Cairo", "Egypt (GMT+2)", 2, "EET", false}},
        
        // Oceania
        {"Australia/Sydney", {"Australia/Sydney", "Australia East (GMT+10)", 10, "AEST", false}},
        {"Australia/Perth", {"Australia/Perth", "Australia West (GMT+8)", 8, "AWST", false}},
        {"Pacific/Auckland", {"Pacific/Auckland", "New Zealand (GMT+12)", 12, "NZST", false}},
    };
    
    // Country to timezone mapping
    m_countryTimezoneMap = {
        {"BD", "Asia/Dhaka"},
        {"IN", "Asia/Kolkata"},
        {"JP", "Asia/Tokyo"},
        {"CN", "Asia/Shanghai"},
        {"SG", "Asia/Singapore"},
        {"AE", "Asia/Dubai"},
        {"ID", "Asia/Jakarta"},
        {"PH", "Asia/Manila"},
        {"MY", "Asia/Kuala_Lumpur"},
        {"NP", "Asia/Kathmandu"},
        {"LK", "Asia/Colombo"},
        {"TH", "Asia/Bangkok"},
        {"VN", "Asia/Ho_Chi_Minh"},
        {"KR", "Asia/Seoul"},
        {"TW", "Asia/Taipei"},
        {"HK", "Asia/Hong_Kong"},
        {"UK", "Europe/London"},
        {"GB", "Europe/London"},
        {"US", "America/New_York"},
        {"CA", "America/Toronto"},
        {"AU", "Australia/Sydney"},
        {"DE", "Europe/Berlin"},
        {"FR", "Europe/Paris"},
        {"ES", "Europe/Madrid"},
        {"IT", "Europe/Rome"},
        {"NL", "Europe/Amsterdam"},
        {"RU", "Europe/Moscow"},
        {"BR", "America/Sao_Paulo"},
        {"MX", "America/Mexico_City"},
        {"NG", "Africa/Lagos"},
        {"KE", "Africa/Nairobi"},
        {"EG", "Africa/Cairo"},
        {"NZ", "Pacific/Auckland"},
    };
    
    // Default timezone (UTC)
    m_deviceTimezone = m_timezoneDatabase["UTC"];
}

ProxyConfig::~ProxyConfig() {}

ProxyConfig& ProxyConfig::getInstance() {
    static ProxyConfig instance;
    return instance;
}

bool ProxyConfig::isEnabled() const {
    return m_enabled && m_currentProxy.has_value();
}

void ProxyConfig::enable() {
    m_enabled = true;
}

void ProxyConfig::disable() {
    m_enabled = false;
}

bool ProxyConfig::setProxy(
    const std::string& host,
    int port,
    ProxyType type,
    const std::string& username,
    const std::string& password
) {
    ProxyConfigData config;
    config.type = type;
    config.host = host;
    config.port = port;
    config.username = username;
    config.password = password;
    config.authenticated = !username.empty();
    config.timeout = 30;
    config.retries = 3;
    config.rotateOnError = false;
    
    // Validate IP/host
    if (!isValidIP(host) && !isValidHostname(host)) {
        std::cerr << "[ProxyConfig] Invalid host: " << host << std::endl;
        return false;
    }
    
    // Validate port
    if (port <= 0 || port > 65535) {
        std::cerr << "[ProxyConfig] Invalid port: " << port << std::endl;
        return false;
    }
    
    // Detect GeoIP for the proxy IP
    std::cout << "[ProxyConfig] Detecting GeoIP for " << host << "..." << std::endl;
    config.geoIP = detectFromIP(host);
    
    // Update primary IP
    m_primaryIP = host;
    
    // Update timezone from GeoIP
    if (!config.geoIP.timezone.empty()) {
        auto it = m_timezoneDatabase.find(config.geoIP.timezone);
        if (it != m_timezoneDatabase.end()) {
            m_deviceTimezone = it->second;
        }
    }
    
    m_currentProxy = config;
    m_currentGeoIP = config.geoIP;
    m_enabled = true;
    
    std::cout << "[ProxyConfig] Proxy set: " << host << ":" << port << std::endl;
    std::cout << "[ProxyConfig] Country: " << config.geoIP.countryName 
              << " (" << config.geoIP.countryCode << ")" << std::endl;
    std::cout << "[ProxyConfig] Timezone: " << m_deviceTimezone.timezone << std::endl;
    std::cout << "[ProxyConfig] ISP: " << config.geoIP.isp << std::endl;
    
    return true;
}

bool ProxyConfig::setProxyFromURL(const std::string& proxyURL) {
    ProxyConfigData config;
    if (!parseProxyURL(proxyURL, config)) {
        return false;
    }
    
    return setProxy(config.host, config.port, config.type, 
                   config.username, config.password);
}

void ProxyConfig::clearProxy() {
    m_currentProxy = std::nullopt;
    m_currentGeoIP = std::nullopt;
    m_enabled = false;
    m_primaryIP = "";
}

std::optional<ProxyConfigData> ProxyConfig::getCurrentProxy() const {
    return m_currentProxy;
}

bool ProxyConfig::testProxy() {
    if (!m_currentProxy.has_value()) {
        return false;
    }
    return testProxy(m_currentProxy.value());
}

bool ProxyConfig::testProxy(const ProxyConfigData& proxy) {
    std::cout << "[ProxyConfig] Testing proxy: " << proxy.host << ":" << proxy.port << std::endl;
    
    // Simple connectivity test
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://httpbin.org/ip");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, proxy.timeout);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // Set proxy
    std::string proxyStr;
    switch (proxy.type) {
        case ProxyType::HTTP:
            proxyStr = "http://";
            break;
        case ProxyType::HTTPS:
            proxyStr = "https://";
            break;
        case ProxyType::SOCKS4:
            proxyStr = "socks4://";
            break;
        case ProxyType::SOCKS5:
            proxyStr = "socks5://";
            break;
        default:
            proxyStr = "http://";
    }
    proxyStr += proxy.host + ":" + std::to_string(proxy.port);
    
    curl_easy_setopt(curl, CURLOPT_PROXY, proxyStr.c_str());
    
    // Auth if needed
    if (proxy.authenticated) {
        std::string auth = proxy.username + ":" + proxy.password;
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, auth.c_str());
    }
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res == CURLE_OK) {
        std::cout << "[ProxyConfig] Proxy test SUCCESS" << std::endl;
        return true;
    } else {
        std::cerr << "[ProxyConfig] Proxy test FAILED: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
}

GeoIPInfo ProxyConfig::detectFromIP(const std::string& ip) {
    GeoIPInfo info;
    info.ip = ip;
    
    // If it's a private IP, use default
    if (isPrivateIP(ip)) {
        info.countryCode = "US";
        info.countryName = "United States";
        info.city = "New York";
        info.isp = "Private Network";
        info.timezone = "America/New_York";
        info.currency = "USD";
        info.latitude = 40.7128;
        info.longitude = -74.0060;
        return info;
    }
    
    // Try to fetch from ip-api.com (free GeoIP service)
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;
        std::string url = "http://ip-api.com/json/" + ip + "?fields=status,country,countryCode,city,isp,org,timezone,currency,lat,lon,regionName,zip";
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            // Parse JSON response (simple parsing)
            try {
                // Extract fields using regex
                std::regex statusRegex(R"("status":"([^"]+)")");
                std::regex countryRegex(R"("country":"([^"]+)")");
                std::regex countryCodeRegex(R"("countryCode":"([^"]+)")");
                std::regex cityRegex(R"("city":"([^"]+)")");
                std::regex ispRegex(R"("isp":"([^"]+)")");
                std::regex orgRegex(R"("org":"([^"]+)")");
                std::regex timezoneRegex(R"("timezone":"([^"]+)")");
                std::regex currencyRegex(R"("currency":"([^"]+)")");
                std::regex latRegex(R"("lat":([0-9.-]+))");
                std::regex lonRegex(R"("lon":([0-9.-]+))");
                std::regex regionRegex(R"("regionName":"([^"]+)")");
                std::regex zipRegex(R"("zip":"([^"]+)")");
                
                std::smatch match;
                
                if (std::regex_search(response, match, statusRegex) && match[1] == "success") {
                    if (std::regex_search(response, match, countryRegex)) info.countryName = match[1];
                    if (std::regex_search(response, match, countryCodeRegex)) info.countryCode = match[1];
                    if (std::regex_search(response, match, cityRegex)) info.city = match[1];
                    if (std::regex_search(response, match, ispRegex)) info.isp = match[1];
                    if (std::regex_search(response, match, orgRegex)) info.org = match[1];
                    if (std::regex_search(response, match, timezoneRegex)) info.timezone = match[1];
                    if (std::regex_search(response, match, currencyRegex)) info.currency = match[1];
                    if (std::regex_search(response, match, latRegex)) info.latitude = std::stod(match[1]);
                    if (std::regex_search(response, match, lonRegex)) info.longitude = std::stod(match[1]);
                    if (std::regex_search(response, match, regionRegex)) info.region = match[1];
                    if (std::regex_search(response, match, zipRegex)) info.postalCode = match[1];
                }
            } catch (...) {
                // Fallback on parse error
            }
        }
        
        curl_easy_cleanup(curl);
    }
    
    // If timezone is empty, try to get from country
    if (info.timezone.empty() && !info.countryCode.empty()) {
        auto it = m_countryTimezoneMap.find(info.countryCode);
        if (it != m_countryTimezoneMap.end()) {
            info.timezone = it->second;
        }
    }
    
    return info;
}

GeoIPInfo ProxyConfig::detectCurrentIP() {
    std::string currentIP;
    
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            currentIP = response;
        }
        
        curl_easy_cleanup(curl);
    }
    
    return detectFromIP(currentIP);
}

std::string ProxyConfig::getCountryFlag(const std::string& countryCode) {
    // Convert country code to flag emoji
    // A -> 127397 in unicode
    std::string flag;
    for (char c : countryCode) {
        flag += "\xF0\x9F\x87";
        flag += (char)(c - 'A' + 0xA5);
    }
    return flag;
}

TimezoneInfo ProxyConfig::detectTimezone(const std::string& ip) {
    GeoIPInfo geo = detectFromIP(ip);
    
    if (!geo.timezone.empty()) {
        auto it = m_timezoneDatabase.find(geo.timezone);
        if (it != m_timezoneDatabase.end()) {
            return it->second;
        }
    }
    
    return getTimezoneFromCountry(geo.countryCode);
}

TimezoneInfo ProxyConfig::getTimezoneFromCountry(const std::string& countryCode) {
    auto tzIt = m_countryTimezoneMap.find(countryCode);
    if (tzIt != m_countryTimezoneMap.end()) {
        auto it = m_timezoneDatabase.find(tzIt->second);
        if (it != m_timezoneDatabase.end()) {
            return it->second;
        }
    }
    
    // Default to UTC
    return m_timezoneDatabase["UTC"];
}

std::vector<std::string> ProxyConfig::getAvailableTimezones() {
    std::vector<std::string> timezones;
    for (const auto& pair : m_timezoneDatabase) {
        timezones.push_back(pair.first);
    }
    return timezones;
}

void ProxyConfig::setTimezone(const std::string& timezone) {
    auto it = m_timezoneDatabase.find(timezone);
    if (it != m_timezoneDatabase.end()) {
        m_deviceTimezone = it->second;
        std::cout << "[ProxyConfig] Timezone set to: " << m_deviceTimezone.displayName << std::endl;
    } else {
        std::cerr << "[ProxyConfig] Unknown timezone: " << timezone << std::endl;
    }
}

std::string ProxyConfig::getPrimaryIP() const {
    if (m_currentProxy.has_value()) {
        return m_currentProxy->host;
    }
    return m_primaryIP;
}

TimezoneInfo ProxyConfig::getDeviceTimezone() const {
    return m_deviceTimezone;
}

std::string ProxyConfig::getDeviceCountry() const {
    if (m_currentGeoIP.has_value()) {
        return m_currentGeoIP->countryCode;
    }
    return "US";
}

void ProxyConfig::applyToProfile(std::map<std::string, std::string>& profileData) {
    if (m_currentProxy.has_value()) {
        profileData["proxy_ip"] = m_currentProxy->host;
        profileData["proxy_port"] = std::to_string(m_currentProxy->port);
        profileData["proxy_type"] = [&]() -> std::string {
            switch (m_currentProxy->type) {
                case ProxyType::HTTP: return "http";
                case ProxyType::HTTPS: return "https";
                case ProxyType::SOCKS4: return "socks4";
                case ProxyType::SOCKS5: return "socks5";
                default: return "none";
            }
        }();
    }
    
    if (m_currentGeoIP.has_value()) {
        profileData["country_code"] = m_currentGeoIP->countryCode;
        profileData["country_name"] = m_currentGeoIP->countryName;
        profileData["city"] = m_currentGeoIP->city;
        profileData["isp"] = m_currentGeoIP->isp;
        profileData["timezone"] = m_currentGeoIP->timezone;
        profileData["currency"] = m_currentGeoIP->currency;
        profileData["latitude"] = std::to_string(m_currentGeoIP->latitude);
        profileData["longitude"] = std::to_string(m_currentGeoIP->longitude);
    }
    
    profileData["device_timezone"] = m_deviceTimezone.timezone;
    profileData["utc_offset"] = std::to_string(m_deviceTimezone.utcOffset);
}

std::map<std::string, TimezoneInfo> ProxyConfig::getTimezoneDatabase() {
    return ProxyConfig::getInstance().m_timezoneDatabase;
}

std::map<std::string, std::string> ProxyConfig::getCountryTimezoneMap() {
    return ProxyConfig::getInstance().m_countryTimezoneMap;
}

// Private helpers
bool ProxyConfig::isValidIP(const std::string& ip) {
    std::regex ipRegex(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    std::smatch match;
    
    if (!std::regex_match(ip, match, ipRegex)) {
        return false;
    }
    
    for (int i = 1; i <= 4; i++) {
        int octet = std::stoi(match[i]);
        if (octet < 0 || octet > 255) {
            return false;
        }
    }
    
    return true;
}

bool ProxyConfig::isValidHostname(const std::string& hostname) {
    std::regex hostnameRegex(R"(^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)");
    return std::regex_match(hostname, hostnameRegex);
}

bool ProxyConfig::isPrivateIP(const std::string& ip) {
    if (!isValidIP(ip)) return false;
    
    std::istringstream iss(ip);
    std::string octet;
    std::vector<int> octets;
    
    while (std::getline(iss, octet, '.')) {
        octets.push_back(std::stoi(octet));
    }
    
    if (octets.size() != 4) return false;
    
    // 10.0.0.0/8
    if (octets[0] == 10) return true;
    
    // 172.16.0.0/12
    if (octets[0] == 172 && octets[1] >= 16 && octets[1] <= 31) return true;
    
    // 192.168.0.0/16
    if (octets[0] == 192 && octets[1] == 168) return true;
    
    // 127.0.0.0/8 (localhost)
    if (octets[0] == 127) return true;
    
    return false;
}

std::string ProxyConfig::fetchURL(const std::string& url) {
    std::string response;
    
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    
    return response;
}

bool ProxyConfig::parseProxyURL(const std::string& url, ProxyConfigData& config) {
    // Format: protocol://host:port or protocol://user:pass@host:port
    std::regex proxyRegex(R"((https?|socks4|socks5)://(?:([^:]+):([^@]+)@)?([^:/]+)(?::(\d+))?)");
    std::smatch match;
    
    if (!std::regex_match(url, match, proxyRegex)) {
        return false;
    }
    
    std::string protocol = match[1];
    config.username = match[2];
    config.password = match[3];
    config.host = match[4];
    std::string portStr = match[5];
    
    // Set proxy type
    if (protocol == "http") config.type = ProxyType::HTTP;
    else if (protocol == "https") config.type = ProxyType::HTTPS;
    else if (protocol == "socks4") config.type = ProxyType::SOCKS4;
    else if (protocol == "socks5") config.type = ProxyType::SOCKS5;
    else config.type = ProxyType::HTTPS;
    
    // Set port
    config.port = portStr.empty() ? 8080 : std::stoi(portStr);
    
    // Auth
    config.authenticated = !config.username.empty();
    
    return true;
}

// ============================================
// TIMEZONE HELPER IMPLEMENTATION
// ============================================

time_t TimezoneHelper::getTimeInTimezone(const std::string& timezone) {
    // Get current time and apply timezone offset
    time_t now = time(nullptr);
    
    auto tzMap = ProxyConfig::getCountryTimezoneMap();
    int offset = 0;
    
    // Find country from timezone
    for (const auto& pair : tzMap) {
        auto tzDb = ProxyConfig::getTimezoneDatabase();
        auto it = tzDb.find(pair.second);
        if (it != tzDb.end() && pair.second == timezone) {
            offset = it->second.utcOffset * 3600; // Convert to seconds
            break;
        }
    }
    
    return now + offset;
}

std::string TimezoneHelper::getFormattedTime(const std::string& timezone, const std::string& format) {
    time_t t = getTimeInTimezone(timezone);
    char buffer[128];
    strftime(buffer, sizeof(buffer), format.c_str(), localtime(&t));
    return std::string(buffer);
}

int TimezoneHelper::getUTCOffset(const std::string& timezone) {
    auto tzDb = ProxyConfig::getTimezoneDatabase();
    auto it = tzDb.find(timezone);
    if (it != tzDb.end()) {
        return it->second.utcOffset;
    }
    return 0;
}

bool TimezoneHelper::isDST(const std::string& timezone) {
    auto tzDb = ProxyConfig::getTimezoneDatabase();
    auto it = tzDb.find(timezone);
    if (it != tzDb.end()) {
        return it->second.daylightSaving;
    }
    return false;
}

// ============================================
// SYNCHRONIZED IDENTITY IMPLEMENTATION
// ============================================

void ProxyConfig::initializeCountryProfiles() {
    m_countries = {
        // Bangladesh
        {"BD", {"BD", "Bangladesh", "bn-BD", "Asia/Dhaka", "BDT", "bn_BD",
                 20.5, 26.6, 88.0, 92.2,
                 {"Grameenphone", "Robi", "Banglalion", "AirTel", "Teletalk"}}},
        
        // United States
        {"US", {"US", "United States", "en-US", "America/New_York", "USD", "en_US",
                 24.5, 49.5, -125.0, -66.9,
                 {"AT&T", "Verizon", "T-Mobile", "Sprint", "MetroPCS"}}},
        
        // United Kingdom
        {"UK", {"UK", "United Kingdom", "en-GB", "Europe/London", "GBP", "en_GB",
                 49.9, 60.9, -8.6, 1.8,
                 {"EE", "O2", "Vodafone", "Three", "Virgin Media"}}},
        
        // Canada
        {"CA", {"CA", "Canada", "en-CA", "America/Toronto", "CAD", "en_CA",
                 41.7, 83.1, -141.0, -52.6,
                 {"Bell", "Rogers", "Telus", "Fido", "Koodo"}}},
        
        // Australia
        {"AU", {"AU", "Australia", "en-AU", "Australia/Sydney", "AUD", "en_AU",
                 -43.6, -10.6, 112.9, 153.6,
                 {"Telstra", "Optus", "Vodafone", "TPG", "iiNet"}}},
        
        // India
        {"IN", {"IN", "India", "hi-IN", "Asia/Kolkata", "INR", "hi_IN",
                 6.7, 35.5, 68.1, 97.4,
                 {"Jio", "Airtel", "Vi", "BSNL", "MTNL"}}},
        
        // Japan
        {"JP", {"JP", "Japan", "ja-JP", "Asia/Tokyo", "JPY", "ja_JP",
                 24.3, 45.5, 122.9, 153.9,
                 {"NTT Docomo", "SoftBank", "KDDI", "Rakuten", "Y!mobile"}}},
        
        // Germany
        {"DE", {"DE", "Germany", "de-DE", "Europe/Berlin", "EUR", "de_DE",
                 47.3, 55.0, 5.9, 15.0,
                 {"Deutsche Telekom", "Vodafone DE", "O2 DE", "1&1", "E-Plus"}}},
        
        // France
        {"FR", {"FR", "France", "fr-FR", "Europe/Paris", "EUR", "fr_FR",
                 41.3, 51.1, -5.1, 9.6,
                 {"Orange", "SFR", "Bouygues", "Free Mobile", "La Poste"}}},
        
        // Singapore
        {"SG", {"SG", "Singapore", "en-SG", "Asia/Singapore", "SGD", "en_SG",
                 1.2, 1.5, 103.6, 104.0,
                 {"Singtel", "StarHub", "M1", " Circles.Life", "TPG"}}},
        
        // UAE
        {"AE", {"AE", "United Arab Emirates", "ar-AE", "Asia/Dubai", "AED", "ar_AE",
                 22.6, 26.1, 51.5, 56.4,
                 {"Etisalat", "du", "Virgin Mobile", "Switch", "Giffgaff"}}},
        
        // Malaysia
        {"MY", {"MY", "Malaysia", "ms-MY", "Asia/Kuala_Lumpur", "MYR", "ms_MY",
                 0.9, 7.4, 99.6, 119.3,
                 {"Maxis", "Celcom", "DiGi", "U Mobile", "Unifi"}}},
        
        // Indonesia
        {"ID", {"ID", "Indonesia", "id-ID", "Asia/Jakarta", "IDR", "id_ID",
                 -11.0, 6.0, 95.0, 141.0,
                 {"Telkomsel", "Indosat", "XL Axiata", "Three", "Smartfren"}}},
        
        // Thailand
        {"TH", {"TH", "Thailand", "th-TH", "Asia/Bangkok", "THB", "th_TH",
                 5.6, 20.5, 97.3, 105.6,
                 {"AIS", "TrueMove", "dtac", "TOT", "CAT"}}},
        
        // Vietnam
        {"VN", {"VN", "Vietnam", "vi-VN", "Asia/Ho_Chi_Minh", "VND", "vi_VN",
                 8.4, 23.4, 102.1, 109.5,
                 {"Viettel", "Vinaphone", "Mobifone", "Vietnamobile", "Gmobile"}}},
        
        // Philippines
        {"PH", {"PH", "Philippines", "fil-PH", "Asia/Manila", "PHP", "fil_PH",
                 4.4, 21.0, 116.4, 126.6,
                 {"Globe", "Smart", "Sun Cellular", "DITO", "TNT"}}},
        
        // South Korea
        {"KR", {"KR", "South Korea", "ko-KR", "Asia/Seoul", "KRW", "ko_KR",
                 33.0, 38.9, 124.6, 131.0,
                 {"SK Telecom", "KT", "LG U+", "MVNO", "T world"}}},
        
        // China
        {"CN", {"CN", "China", "zh-CN", "Asia/Shanghai", "CNY", "zh_CN",
                 18.2, 53.6, 73.5, 135.0,
                 {"China Mobile", "China Unicom", "China Telecom", "China Broadcasting"}}},
        
        // Brazil
        {"BR", {"BR", "Brazil", "pt-BR", "America/Sao_Paulo", "BRL", "pt_BR",
                 -33.8, 5.3, -73.9, -34.8,
                 {"Vivo", "Claro", "TIM", "Oi", "Nextel"}}},
        
        // Mexico
        {"MX", {"MX", "Mexico", "es-MX", "America/Mexico_City", "MXN", "es_MX",
                 14.5, 32.7, -118.4, -86.7,
                 {"Telcel", "Movistar", "AT&T MX", "Unefon", "Virgin Mobile"}}},
        
        // Nigeria
        {"NG", {"NG", "Nigeria", "en-NG", "Africa/Lagos", "NGN", "en_NG",
                 4.2, 13.9, 2.7, 14.7,
                 {"MTN", "Airtel", "Glo", "9mobile", "NCC"}}},
    };
}

double CountryProfile::generateLatitude() const {
    // Generate random lat within bounds
    double range = latMax - latMin;
    double lat = latMin + (range * Crypto::SecureRandomGenerator().generateDouble());
    return lat;
}

double CountryProfile::generateLongitude() const {
    double range = lonMax - lonMin;
    double lon = lonMin + (range * Crypto::SecureRandomGenerator().generateDouble());
    return lon;
}

std::string ProxyConfig::getLanguageForCountry(const std::string& countryCode) {
    auto it = m_countries.find(countryCode);
    if (it != m_countries.end()) {
        return it->second.primaryLanguage;
    }
    return "en-US"; // Default
}

std::string ProxyConfig::getAndroidLocale(const std::string& countryCode) {
    auto it = m_countries.find(countryCode);
    if (it != m_countries.end()) {
        return it->second.locale;
    }
    return "en_US";
}

void ProxyConfig::setLanguage(const std::string& language) {
    m_currentLanguage = language;
}

std::string ProxyConfig::getLanguage() const {
    return m_currentLanguage.empty() ? "en-US" : m_currentLanguage;
}

DeviceIdentity ProxyConfig::createIdentityWithProxy(const std::string& proxyIP) {
    DeviceIdentity identity;
    identity.ip = proxyIP;
    
    // Detect GeoIP from proxy
    GeoIPInfo geo = detectFromIP(proxyIP);
    
    identity.countryCode = geo.countryCode;
    identity.countryName = geo.countryName;
    identity.city = geo.city;
    identity.isp = geo.isp;
    identity.timezone = geo.timezone;
    identity.language = getLanguageForCountry(geo.countryCode);
    identity.locale = getAndroidLocale(geo.countryCode);
    
    // Generate unique GPS within country bounds (NOT from IP to avoid correlation)
    CountryProfile cp = getCountryProfile(geo.countryCode);
    identity.latitude = cp.generateLatitude();
    identity.longitude = cp.generateLongitude();
    
    // DNS based on country
    if (geo.countryCode == "US") identity.dns = "8.8.8.8,8.8.4.4";
    else if (geo.countryCode == "UK") identity.dns = "8.8.8.8,8.8.4.4";
    else identity.dns = "8.8.8.8,8.8.4.4";
    
    return identity;
}

DeviceIdentity ProxyConfig::createUniqueIdentity(const std::string& countryCode) {
    DeviceIdentity identity;
    
    // Generate unique root IP within country range
    identity.ip = generateUniqueIP(countryCode);
    
    // Get country info
    CountryProfile cp = getCountryProfile(countryCode);
    
    identity.countryCode = cp.countryCode;
    identity.countryName = cp.countryName;
    identity.city = ""; // Will be generated
    identity.isp = cp.mobileCarriers[0];
    identity.timezone = cp.timezone;
    identity.language = cp.primaryLanguage;
    identity.locale = cp.locale;
    
    // Generate UNIQUE GPS within country
    identity.latitude = cp.generateLatitude();
    identity.longitude = cp.generateLongitude();
    
    // DNS
    identity.dns = "8.8.8.8,8.8.4.4";
    
    return identity;
}

std::string ProxyConfig::generateUniqueIP(const std::string& countryCode) {
    std::string key = countryCode + "_ip";
    
    // If already used, generate new
    while (m_usedIPs.count(key)) {
        // Generate mobile carrier IP range
        uint32_t baseIP = 0;
        if (countryCode == "BD") baseIP = 0x67000000; // 103.x.x.x Bangladesh mobile
        else if (countryCode == "US") baseIP = 0xC0A80000; // 192.x.x.x
        else if (countryCode == "UK") baseIP = 0xC0A80000;
        else if (countryCode == "IN") baseIP = 0x66000000; // 102.x.x.x
        else if (countryCode == "JP") baseIP = 0xC0A80000;
        else baseIP = 0xC0A80000;
        
        uint32_t ip = baseIP + (Crypto::SecureRandomGenerator().generateUint32() % 0xFFFF);
        
        std::ostringstream oss;
        oss << ((ip >> 24) & 0xFF) << "."
            << ((ip >> 16) & 0xFF) << "."
            << ((ip >> 8) & 0xFF) << "."
            << (ip & 0xFF);
        
        std::string newIP = oss.str();
        key = countryCode + "_" + newIP;
        
        if (!m_usedIPs.count(newIP)) {
            m_usedIPs.insert(newIP);
            return newIP;
        }
    }
    
    m_usedIPs.insert(key);
    return key;
}

GeoIPInfo ProxyConfig::generateUniqueGPS(const std::string& countryCode) {
    GeoIPInfo geo;
    
    CountryProfile cp = getCountryProfile(countryCode);
    
    geo.latitude = cp.generateLatitude();
    geo.longitude = cp.generateLongitude();
    geo.countryCode = countryCode;
    geo.countryName = cp.countryName;
    geo.timezone = cp.timezone;
    geo.currency = cp.currency;
    geo.language = cp.primaryLanguage;
    
    // Generate unique coordinate key
    std::ostringstream coordKey;
    coordKey << std::fixed << std::setprecision(6) << geo.latitude << "," << geo.longitude;
    
    // Ensure uniqueness
    while (m_usedCoordinates.count(coordKey.str())) {
        geo.latitude = cp.generateLatitude();
        geo.longitude = cp.generateLongitude();
        coordKey.str("");
        coordKey << std::fixed << std::setprecision(6) << geo.latitude << "," << geo.longitude;
    }
    
    m_usedCoordinates.insert(coordKey.str());
    
    return geo;
}

std::vector<CountryProfile> ProxyConfig::getSupportedCountries() {
    std::vector<CountryProfile> countries;
    for (const auto& pair : m_countries) {
        countries.push_back(pair.second);
    }
    return countries;
}

CountryProfile ProxyConfig::getCountryProfile(const std::string& countryCode) {
    auto it = m_countries.find(countryCode);
    if (it != m_countries.end()) {
        return it->second;
    }
    // Default to US
    return m_countries["US"];
}

} // namespace VirtualPhonePro
