/*
 * VirtualPhonePro CLI v2.6
 */

#include "core/CoreEngine.hpp"
#include "network/ProxyConfig.hpp"
#include "core/CryptoUtils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace VirtualPhonePro;

static std::map<std::string, std::map<std::string, std::string>> g_profiles;

void printBanner() {
    std::cout << "VirtualPhonePro v2.6" << std::endl;
    std::cout << "[+] Profile Management" << std::endl;
    std::cout << "[+] Proxy/IP Setup" << std::endl;
    std::cout << "[+] VM Detection Prevention (99%)" << std::endl << std::endl;
}

void printHelp() {
    std::cout << "Commands: profile list|create|delete|info|quick" << std::endl;
    std::cout << "          proxy set|test|clear|status" << std::endl;
    std::cout << "          vm list|create" << std::endl;
    std::cout << "          tz set <timezone>" << std::endl;
    std::cout << "          system status" << std::endl;
    std::cout << "          help, exit" << std::endl << std::endl;
}

void listProfiles() {
    if (g_profiles.empty()) {
        std::cout << "No profiles. Use: profile create <country>" << std::endl << std::endl;
        return;
    }
    for (const auto& [id, data] : g_profiles) {
        std::cout << id << " | " << data.at("manufacturer") << " " << data.at("model") << " | " << data.at("country") << " | IP: " << data.at("ip") << std::endl;
    }
    std::cout << "Total: " << g_profiles.size() << " profiles" << std::endl << std::endl;
}

void showProfileInfo(const std::string& id) {
    auto it = g_profiles.find(id);
    if (it == g_profiles.end()) {
        std::cerr << "Profile not found: " << id << std::endl;
        return;
    }
    auto& p = it->second;
    std::cout << "ID: " << p["id"] << std::endl;
    std::cout << "Device: " << p["manufacturer"] << " " << p["model"] << std::endl;
    std::cout << "Android: " << p["android"] << std::endl;
    std::cout << "Country: " << p["country"] << std::endl;
    std::cout << "IP: " << p["ip"] << std::endl;
    std::cout << "Timezone: " << p["timezone"] << std::endl << std::endl;
}

bool deleteProfile(const std::string& id) {
    auto it = g_profiles.find(id);
    if (it == g_profiles.end()) {
        std::cerr << "Profile not found: " << id << std::endl;
        return false;
    }
    std::cout << "Delete " << id << "? [y/N]: ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (confirm == "y" || confirm == "Y") {
        g_profiles.erase(it);
        std::cout << "[OK] Deleted. Storage freed." << std::endl << std::endl;
        return true;
    }
    std::cout << "[CANCELLED]" << std::endl << std::endl;
    return false;
}

void createProfile(CoreEngine& engine, const std::string& country) {
    std::cout << "Profile Creation" << std::endl;
    auto& proxy = ProxyConfig::getInstance();
    std::cout << "1-Proxy, 2-Direct IP, 3-Skip [3]: ";
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "1") {
        std::cout << "URL: ";
        std::string url;
        std::getline(std::cin, url);
        if (!url.empty() && url.find("://") == std::string::npos) url = "http://" + url;
        if (!url.empty()) proxy.setProxyFromURL(url);
    } else if (choice == "2") {
        std::cout << "IP: ";
        std::string ip;
        std::getline(std::cin, ip);
        if (!ip.empty()) proxy.setProxy(ip, 80, ProxyType::HTTP);
    }
    std::string targetCountry = country.empty() ? proxy.getDeviceCountry() : country;
    auto profile = engine.generateRandomProfile(targetCountry);
    std::string pid = "p" + std::to_string(g_profiles.size() + 1);
    g_profiles[pid] = {{"id", pid}, {"manufacturer", profile.manufacturer}, {"model", profile.model}, {"android", profile.androidVersion}, {"country", targetCountry}, {"ip", proxy.isEnabled() ? proxy.getCurrentProxy()->host : "Default"}, {"timezone", proxy.getDeviceTimezone().timezone}, {"fingerprint", profile.fingerprint}};
    std::cout << "[OK] Profile: " << pid << " - " << profile.manufacturer << " " << profile.model << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    auto& engine = CoreEngine::getInstance();
    auto& proxy = ProxyConfig::getInstance();
    if (!engine.initialize()) {
        std::cerr << "[ERROR] Init failed" << std::endl;
        return 1;
    }
    printBanner();
    std::string input;
    while (true) {
        std::cout << "VPP> ";
        std::getline(std::cin, input);
        if (input.empty()) continue;
        if (input == "exit" || input == "quit") break;
        if (input == "help") { printHelp(); continue; }
        std::istringstream iss(input);
        std::string cmd, subcmd, arg;
        iss >> cmd >> subcmd >> arg;
        if (cmd == "profile") {
            if (subcmd == "list") listProfiles();
            else if (subcmd == "create") createProfile(engine, arg);
            else if (subcmd == "delete") deleteProfile(arg);
            else if (subcmd == "info") showProfileInfo(arg);
            else if (subcmd == "quick") {
                auto p = engine.generateRandomProfile(arg.empty() ? "US" : arg);
                std::string pid = "q" + std::to_string(g_profiles.size() + 1);
                g_profiles[pid] = {{"id", pid}, {"manufacturer", p.manufacturer}, {"model", p.model}, {"android", p.androidVersion}, {"country", arg.empty() ? "US" : arg}, {"ip", "Default"}, {"timezone", "UTC"}, {"fingerprint", p.fingerprint}};
                std::cout << "[OK] Quick: " << p.manufacturer << " " << p.model << " (ID: " << pid << ")" << std::endl << std::endl;
            }
        }
        else if (cmd == "proxy") {
            if (subcmd == "set") {
                std::string url = arg;
                std::getline(iss, url);
                if (!url.empty() && url[0] == ' ') url = url.substr(1);
                if (!url.empty()) {
                    if (url.find("://") == std::string::npos) url = "http://" + url;
                    proxy.setProxyFromURL(url);
                    std::cout << "[OK] Proxy: " << url << std::endl;
                }
            }
            else if (subcmd == "test") std::cout << (proxy.testProxy() ? "[OK]" : "[FAIL]") << std::endl;
            else if (subcmd == "clear") { proxy.clearProxy(); std::cout << "[OK] Cleared" << std::endl; }
            else if (subcmd == "status") {
                auto p = proxy.getCurrentProxy();
                if (p) std::cout << "Proxy: " << p->host << ":" << p->port << std::endl;
                else std::cout << "No proxy" << std::endl;
            }
        }
        else if (cmd == "vm") {
            if (subcmd == "list") {
                auto vms = engine.listVMs();
                std::cout << "VMs:" << std::endl;
                for (const auto& v : vms) std::cout << "  " << v.id << " - " << v.name << std::endl;
                std::cout << std::endl;
            }
            else if (subcmd == "create") {
                VMConfig cfg;
                cfg.name = arg.empty() ? "VM" : arg;
                cfg.cpuCores = 4;
                cfg.memoryMB = 4096;
                std::cout << "[OK] " << engine.createVM(cfg) << std::endl;
            }
        }
        else if (cmd == "tz" && subcmd == "set") proxy.setTimezone(arg);
        else if (cmd == "system" && subcmd == "status") {
            std::cout << "VT: " << (engine.checkHardwareVirtualization() ? "Yes" : "No") << std::endl;
            std::cout << "Proxy: " << (proxy.isEnabled() ? "On" : "Off") << std::endl;
            std::cout << "Profiles: " << g_profiles.size() << std::endl;
            std::cout << std::endl;
        }
        else std::cout << "Unknown. Type 'help'" << std::endl;
    }
    engine.shutdown();
    return 0;
}
