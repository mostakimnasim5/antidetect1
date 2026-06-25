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
    : m_enabled(false) {
    
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

} // namespace VirtualPhonePro
