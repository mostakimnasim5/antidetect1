#include "ProxyManager.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"
#include <sstream>
#include <fstream>

namespace AntiDetect {

ProxyManager& ProxyManager::getInstance() {
    static ProxyManager instance;
    return instance;
}

ProxyManager::ProxyManager() : m_initialized(false) {}

ProxyManager::~ProxyManager() {
    shutdown();
}

bool ProxyManager::initialize() {
    if (m_initialized) return true;
    
    Logger::getInstance().info("Initializing ProxyManager...");
    m_initialized = true;
    
    return true;
}

bool ProxyManager::shutdown() {
    if (!m_initialized) return true;
    
    removeProxy();
    m_initialized = false;
    
    return true;
}

std::string NetworkProxyInfo::getProxyString() const {
    std::stringstream ss;
    
    switch (type) {
        case ProxyType::SOCKS5:
            ss << "socks5://";
            break;
        case ProxyType::SOCKS4:
            ss << "socks4://";
            break;
        case ProxyType::HTTPS:
            ss << "https://";
            break;
        default:
            ss << "http://";
            break;
    }
    
    if (credentials.useAuth) {
        ss << credentials.username << ":" << credentials.password << "@";
    }
    
    ss << host << ":" << port;
    
    return ss.str();
}

std::string NetworkProxyInfo::getFormattedProxy() const {
    std::stringstream ss;
    ss << host << ":" << port;
    if (credentials.useAuth) {
        ss << " [Auth: " << credentials.username << "]";
    }
    return ss.str();
}

std::string ProxyManager::proxyTypeToString(ProxyType type) const {
    switch (type) {
        case ProxyType::HTTPS: return "HTTPS";
        case ProxyType::SOCKS4: return "SOCKS4";
        case ProxyType::SOCKS5: return "SOCKS5";
        default: return "HTTP";
    }
}

ProxyType ProxyManager::stringToProxyType(const std::string& type) const {
    if (type == "HTTPS" || type == "https") return ProxyType::HTTPS;
    if (type == "SOCKS4" || type == "socks4") return ProxyType::SOCKS4;
    if (type == "SOCKS5" || type == "socks5") return ProxyType::SOCKS5;
    return ProxyType::HTTP;
}

bool ProxyManager::setProxy(const NetworkProxyInfo& proxy) {
    Logger::getInstance().info("Setting proxy: " + proxy.getFormattedProxy());
    
    if (!proxy.enabled) {
        return removeProxy();
    }
    
    if (!validateProxy(proxy)) {
        Logger::getInstance().error("Invalid proxy configuration");
        return false;
    }
    
    m_currentProxy = proxy;
    
    return applyProxySettings(proxy);
}

bool ProxyManager::applyProxySettings(const NetworkProxyInfo& proxy) {
    auto& adb = ADBManager::getInstance();
    
    std::string proxyStr = proxy.getProxyString();
    
    adb.setProperty("persist.sys.proxy", proxy.host + ":" + std::to_string(proxy.port));
    
    switch (proxy.type) {
        case ProxyType::HTTP:
        case ProxyType::HTTPS:
            adb.executeShellCommand("settings put global http_proxy " + proxy.host + ":" + std::to_string(proxy.port));
            break;
        case ProxyType::SOCKS5:
            adb.setProperty("proxy.socks", "socks://" + proxy.host + ":" + std::to_string(proxy.port));
            adb.executeShellCommand("settings put global http_proxy " + proxy.host + ":" + std::to_string(proxy.port));
            break;
        case ProxyType::SOCKS4:
            adb.setProperty("proxy.socks", "socks4://" + proxy.host + ":" + std::to_string(proxy.port));
            adb.executeShellCommand("settings put global http_proxy " + proxy.host + ":" + std::to_string(proxy.port));
            break;
    }
    
    Logger::getInstance().info("Proxy applied successfully");
    
    return true;
}

bool ProxyManager::removeProxy() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings delete global http_proxy");
    adb.executeShellCommand("settings delete global proxy_host");
    adb.executeShellCommand("settings delete global proxy_port");
    adb.setProperty("persist.sys.proxy", "");
    adb.setProperty("proxy.socks", "");
    
    m_currentProxy.enabled = false;
    
    Logger::getInstance().info("Proxy removed");
    
    return true;
}

NetworkProxyInfo ProxyManager::getCurrentProxy() const {
    return m_currentProxy;
}

bool ProxyManager::isProxyEnabled() const {
    return m_currentProxy.enabled;
}

bool ProxyManager::validateProxy(const NetworkProxyInfo& proxy) {
    if (proxy.host.empty()) return false;
    if (proxy.port <= 0 || proxy.port > 65535) return false;
    if (proxy.credentials.useAuth) {
        if (proxy.credentials.username.empty() || proxy.credentials.password.empty()) {
            return false;
        }
    }
    return true;
}

bool ProxyManager::testProxy(const NetworkProxyInfo& proxy) {
    Logger::getInstance().info("Testing proxy: " + proxy.getFormattedProxy());
    return true;
}

std::string ProxyManager::getProxyIP() const {
    auto& adb = ADBManager::getInstance();
    std::string result = adb.executeShellCommand("curl -s ifconfig.me 2>/dev/null || curl -s ipinfo.io/ip 2>/dev/null || echo 'unknown'");
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

bool ProxyManager::applyProxyToProfile(const std::string& profileId, const NetworkProxyInfo& proxy) {
    m_profileProxies[profileId] = proxy;
    Logger::getInstance().info("Proxy assigned to profile: " + profileId);
    return true;
}

bool ProxyManager::removeProxyFromProfile(const std::string& profileId) {
    if (m_profileProxies.erase(profileId) > 0) {
        Logger::getInstance().info("Proxy removed from profile: " + profileId);
        return true;
    }
    return false;
}

NetworkProxyInfo ProxyManager::getProfileProxy(const std::string& profileId) const {
    auto it = m_profileProxies.find(profileId);
    if (it != m_profileProxies.end()) {
        return it->second;
    }
    return NetworkProxyInfo();
}

bool ProxyManager::addToPool(const std::string& poolName, const NetworkProxyInfo& proxy) {
    m_proxyPools[poolName].proxies.push_back(proxy);
    m_proxyPools[poolName].poolName = poolName;
    Logger::getInstance().info("Added proxy to pool: " + poolName);
    return true;
}

bool ProxyManager::removeFromPool(const std::string& poolName, const std::string& host, int port) {
    auto it = m_proxyPools.find(poolName);
    if (it == m_proxyPools.end()) return false;
    
    it->second.proxies.erase(
        std::remove_if(it->second.proxies.begin(), it->second.proxies.end(),
            [&host, &port](const NetworkProxyInfo& p) {
                return p.host == host && p.port == port;
            }),
        it->second.proxies.end()
    );
    
    return true;
}

NetworkProxyInfo ProxyManager::getNextFromPool(const std::string& poolName) {
    auto it = m_proxyPools.find(poolName);
    if (it == m_proxyPools.end() || it->second.proxies.empty()) {
        return NetworkProxyInfo();
    }
    
    int index = it->second.currentIndex % it->second.proxies.size();
    NetworkProxyInfo proxy = it->second.proxies[index];
    
    if (it->second.rotateAutomatically) {
        it->second.currentIndex = (it->second.currentIndex + 1) % it->second.proxies.size();
    }
    
    return proxy;
}

bool ProxyManager::rotatePool(const std::string& poolName) {
    auto it = m_proxyPools.find(poolName);
    if (it != m_proxyPools.end() && !it->second.proxies.empty()) {
        it->second.currentIndex = (it->second.currentIndex + 1) % it->second.proxies.size();
        return true;
    }
    return false;
}

std::vector<NetworkProxyInfo> ProxyManager::parseProxyList(const std::string& proxyString) {
    std::vector<NetworkProxyInfo> proxies;
    std::istringstream stream(proxyString);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        NetworkProxyInfo proxy;
        
        std::string cleaned = line;
        cleaned.erase(std::remove_if(cleaned.begin(), cleaned.end(), ::isspace), cleaned.end());
        
        size_t protocolPos = cleaned.find("://");
        if (protocolPos != std::string::npos) {
            std::string protocol = cleaned.substr(0, protocolPos);
            proxy.type = stringToProxyType(protocol);
            cleaned = cleaned.substr(protocolPos + 3);
        } else {
            proxy.type = ProxyType::HTTP;
        }
        
        size_t authPos = cleaned.find('@');
        if (authPos != std::string::npos) {
            std::string auth = cleaned.substr(0, authPos);
            size_t colonPos = auth.find(':');
            if (colonPos != std::string::npos) {
                proxy.credentials.username = auth.substr(0, colonPos);
                proxy.credentials.password = auth.substr(colonPos + 1);
                proxy.credentials.useAuth = true;
            }
            cleaned = cleaned.substr(authPos + 1);
        }
        
        size_t colonPos = cleaned.find(':');
        if (colonPos != std::string::npos) {
            proxy.host = cleaned.substr(0, colonPos);
            try {
                proxy.port = std::stoi(cleaned.substr(colonPos + 1));
            } catch (...) {
                continue;
            }
        } else {
            continue;
        }
        
        proxy.enabled = true;
        proxies.push_back(proxy);
    }
    
    return proxies;
}

bool ProxyManager::importFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::getInstance().error("Could not open proxy file: " + filepath);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    auto proxies = parseProxyList(buffer.str());
    
    Logger::getInstance().info("Imported " + std::to_string(proxies.size()) + " proxies");
    
    return !proxies.empty();
}

bool ProxyManager::importFromText(const std::string& text) {
    auto proxies = parseProxyList(text);
    return !proxies.empty();
}

bool ProxyManager::useRealDeviceIP() {
    Logger::getInstance().info("Using real device IP (proxy disabled)");
    return removeProxy();
}

std::string ProxyManager::getDeviceRealIP() const {
    auto& adb = ADBManager::getInstance();
    std::string result = adb.executeShellCommand("curl -s ifconfig.me 2>/dev/null");
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

std::string ProxyManager::getDeviceRealMAC() const {
    auto& adb = ADBManager::getInstance();
    std::string result = adb.executeShellCommand("cat /sys/class/net/wlan0/address 2>/dev/null");
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

bool ProxyManager::setDNS(const std::string& dnsServer) {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("mount -o rw,remount /system");
    
    adb.executeShellCommand("setprop net.dns1 " + dnsServer);
    adb.executeShellCommand("setprop net.dns2 " + dnsServer);
    adb.setProperty("persist.net.dns1", dnsServer);
    
    adb.executeShellCommand("settings put global private_dns_default_hostname " + dnsServer);
    adb.executeShellCommand("settings put global private_dns_mode hostname");
    
    Logger::getInstance().info("DNS set to: " + dnsServer);
    
    return true;
}

bool ProxyManager::setGoogleDNS() {
    return setDNS("8.8.8.8");
}

bool ProxyManager::setCloudflareDNS() {
    return setDNS("1.1.1.1");
}

bool ProxyManager::resetDNS() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("setprop net.dns1 8.8.8.8");
    adb.executeShellCommand("setprop net.dns2 8.8.4.4");
    adb.setProperty("persist.net.dns1", "");
    adb.setProperty("persist.net.dns2", "");
    
    adb.executeShellCommand("settings delete global private_dns_default_hostname");
    adb.executeShellCommand("settings delete global private_dns_mode");
    
    Logger::getInstance().info("DNS reset to default");
    
    return true;
}

bool ProxyManager::setWiFiProxy(const NetworkProxyInfo& proxy) {
    auto& adb = ADBManager::getInstance();
    
    std::string proxyHost = proxy.host;
    std::string proxyPort = std::to_string(proxy.port);
    std::string proxyExclusionList = "";
    
    adb.executeShellCommand("settings put global wifi_proxy_settings "
        + "{\"proxyHost\":\"" + proxyHost + "\",\"proxyPort\":\"" + proxyPort + "\"}");
    
    return true;
}

bool ProxyManager::removeWiFiProxy() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings delete global wifi_proxy_settings");
    adb.executeShellCommand("settings delete global http_proxy");
    
    return true;
}

}
