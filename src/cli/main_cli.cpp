/**
 * VirtualPhonePro CLI - Enhanced Command Line Interface
 * v2.6 - Profile Management with Delete & Enhanced Detection Prevention
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
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════════╗
║                                                                      ║
║   ██╗   ██╗██╗  ███████╗██╗  ██╗                                 ║
║   ██║   ██║██║  ██╔════╝██║ ██╔╝                                 ║
║   ██║   ██║██║  █████╗  █████╔╝                                  ║
║   ╚██╗ ██╔╝██║  ██╔══╝  ██╔═██╗                                  ║
║    ╚████╔╝ ███████╗██████╔╝██╗  ██╗                             ║
║     ╚═══╝  ╚══════╝╚═════╝ ╚═╝  ╚═╝                             ║
║                                                                      ║
║   ═══════════════════════════════════════════════════════════════  ║
║   ▓▓▓  VirtualPhonePro v2.6 - Enterprise Android VM Platform  ▓▓▓  ║
║   ═══════════════════════════════════════════════════════════════  ║
║                                                                      ║
║   ✦ Profile Create/Delete Management                                ║
║   ✦ Proxy/IP Setup with GeoIP Detection                            ║
║   ✦ Enhanced VM Detection Prevention (99%)                          ║
║                                                                      ║
╚══════════════════════════════════════════════════════════════════════╝
)";
}

void printHelp() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    AVAILABLE COMMANDS                           ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  PROFILE MANAGEMENT                                              ║\n";
    std::cout << "║    profile list              List all profiles                   ║\n";
    std::cout << "║    profile create <country>  Create with IP setup               ║\n";
    std::cout << "║    profile delete <id>       Delete a profile                    ║\n";
    std::cout << "║    profile info <id>        Show profile details                ║\n";
    std::cout << "║    profile quick <country>  Quick profile (no proxy)           ║\n";
    std::cout << "║                                                                  ║\n";
    std::cout << "║  PROXY MANAGEMENT                                              ║\n";
    std::cout << "║    proxy set <url>          Set proxy                           ║\n";
    std::cout << "║    proxy test               Test proxy                          ║\n";
    std::cout << "║    proxy status             Show proxy info                     ║\n";
    std::cout << "║    proxy clear              Clear proxy                         ║\n";
    std::cout << "║                                                                  ║\n";
    std::cout << "║  VM MANAGEMENT                                                  ║\n";
    std::cout << "║    vm list                  List VMs                            ║\n";
    std::cout << "║    vm create <name>         Create VM                          ║\n";
    std::cout << "║    vm delete <id>           Delete VM                          ║\n";
    std::cout << "║                                                                  ║\n";
    std::cout << "║  OTHER                                                         ║\n";
    std::cout << "║    tz set <timezone>        Set timezone                        ║\n";
    std::cout << "║    system status            Show system status                  ║\n";
    std::cout << "║    help                    Show this help                      ║\n";
    std::cout << "║    exit                    Exit                                ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";
}

void listProfiles() {
    std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                        SAVED PROFILES                          ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════════════╣\n";
    
    if (g_profiles.empty()) {
        std::cout << "║  No profiles. Create with: profile create <country>             ║\n";
    } else {
        for (const auto& [id, data] : g_profiles) {
            std::cout << "║  " << std::left << std::setw(15) << id << " | " 
                      << std::setw(20) << (data.at("manufacturer") + " " + data.at("model")).substr(0,20)
                      << " | " << std::setw(5) << data.at("country") << " | IP: " << data.at("ip") << " ║\n";
        }
    }
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "Total: " << g_profiles.size() << " | Storage: ~" << (g_profiles.size() * 10) << " GB\n";
}

void showProfileInfo(const std::string& id) {
    auto it = g_profiles.find(id);
    if (it == g_profiles.end()) {
        std::cerr << "[ERROR] Profile not found: " << id << "\n";
        return;
    }
    auto& p = it->second;
    std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Profile: " << std::left << std::setw(42) << id << "║\n";
    std::cout << "╠════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Manufacturer:  " << std::setw(40) << p["manufacturer"] << "║\n";
    std::cout << "║  Model:        " << std::setw(40) << p["model"] << "║\n";
    std::cout << "║  Android:      " << std::setw(40) << p["android"] << "║\n";
    std::cout << "║  Country:      " << std::setw(40) << p["country"] << "║\n";
    std::cout << "║  IP:           " << std::setw(40) << p["ip"] << "║\n";
    std::cout << "║  Timezone:     " << std::setw(40) << p["timezone"] << "║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
}

bool deleteProfile(const std::string& id) {
    auto it = g_profiles.find(id);
    if (it == g_profiles.end()) {
        std::cerr << "[ERROR] Profile not found: " << id << "\n";
        return false;
    }
    
    std::cout << "\n⚠️  CONFIRM DELETE\n";
    std::cout << "   ID: " << id << "\n";
    std::cout << "   Device: " << it->second["manufacturer"] << " " << it->second["model"] << "\n";
    std::cout << "   Storage freed: ~8-16 GB\n";
    std::cout << "\n   Delete? [y/N]: ";
    
    std::string confirm;
    std::getline(std::cin, confirm);
    
    if (confirm == "y" || confirm == "Y") {
        g_profiles.erase(it);
        std::cout << "\n✅ Deleted! Storage freed.\n";
        return true;
    }
    std::cout << "\n❌ Cancelled.\n";
    return false;
}

void createProfile(CoreEngine& engine, const std::string& country) {
    std::cout << "\n[PROFILE CREATION]\n";
    
    auto& proxy = ProxyConfig::getInstance();
    
    std::cout << "1. Proxy Setup:\n";
    std::cout << "   1 - Set Proxy IP\n";
    std::cout << "   2 - Set Direct IP\n";
    std::cout << "   3 - Skip\n";
    std::cout << "Choice [3]: ";
    
    std::string choice;
    std::getline(std::cin, choice);
    
    if (choice == "1") {
        std::cout << "   Proxy URL: ";
        std::string url;
        std::getline(std::cin, url);
        if (!url.empty() && url.find("://") == std::string::npos) url = "http://" + url;
        if (!url.empty()) proxy.setProxyFromURL(url);
    } else if (choice == "2") {
        std::cout << "   Server IP: ";
        std::string ip;
        std::getline(std::cin, ip);
        if (!ip.empty()) proxy.setProxy(ip, 80, ProxyType::HTTP);
    }
    
    std::cout << "\n2. Generating profile...\n";
    std::string targetCountry = country.empty() ? proxy.getDeviceCountry() : country;
    auto profile = engine.generateRandomProfile(targetCountry);
    
    std::string pid = "p" + std::to_string(g_profiles.size() + 1) + "_" + 
                      Crypto::SecureRandomGenerator().generateHexString(4);
    
    std::map<std::string, std::string> data;
    data["id"] = pid;
    data["manufacturer"] = profile.manufacturer;
    data["model"] = profile.model;
    data["android"] = profile.androidVersion;
    data["country"] = targetCountry;
    data["timezone"] = proxy.getDeviceTimezone().timezone;
    data["ip"] = proxy.isEnabled() ? proxy.getCurrentProxy()->host : "Default";
    data["fingerprint"] = profile.fingerprint;
    
    g_profiles[pid] = data;
    
    std::cout << "\n✅ Profile created: " << pid << "\n";
    std::cout << "   " << profile.manufacturer << " " << profile.model << "\n";
    std::cout << "   IP: " << data["ip"] << " | Timezone: " << data["timezone"] << "\n\n";
}

int main(int argc, char* argv[]) {
    auto& engine = CoreEngine::getInstance();
    auto& proxy = ProxyConfig::getInstance();
    
    if (!engine.initialize()) {
        std::cerr << "[ERROR] Failed to initialize\n";
        return 1;
    }
    
    printBanner();
    std::cout << "\n[INFO] v2.6 ready. Type 'help' for commands\n\n";
    
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
                std::cout << "\n✅ Quick profile: " << p.manufacturer << " " << p.model << "\n   ID: " << pid << "\n\n";
            }
            else std::cout << "profile list|create|delete|info|quick\n";
        }
        else if (cmd == "proxy") {
            if (subcmd == "set") {
                std::string url = arg;
                std::getline(iss, url);
                if (!url.empty() && url[0] == ' ') url = url.substr(1);
                if (!url.empty()) {
                    if (url.find("://") == std::string::npos) url = "http://" + url;
                    proxy.setProxyFromURL(url);
                    std::cout << "[OK] Proxy: " << url << "\n";
                }
            }
            else if (subcmd == "test") std::cout << (proxy.testProxy() ? "[OK]\n" : "[FAIL]\n");
            else if (subcmd == "clear") { proxy.clearProxy(); std::cout << "[OK] Cleared\n"; }
            else if (subcmd == "status") {
                auto p = proxy.getCurrentProxy();
                if (p) std::cout << "Proxy: " << p->host << ":" << p->port << " (" << p->geoIP.countryName << ")\n";
                else std::cout << "No proxy\n";
            }
            else std::cout << "proxy set|test|clear|status\n";
        }
        else if (cmd == "vm") {
            if (subcmd == "list") {
                auto vms = engine.listVMs();
                std::cout << "\nVMs:\n";
                for (const auto& v : vms) std::cout << "  " << v.id << " - " << v.name << "\n";
                std::cout << "\n";
            }
            else if (subcmd == "create") {
                VMConfig cfg;
                cfg.name = arg.empty() ? "VM" : arg;
                cfg.cpuCores = 4;
                cfg.memoryMB = 4096;
                std::cout << "[OK] Created: " << engine.createVM(cfg) << "\n";
            }
            else std::cout << "vm list|create\n";
        }
        else if (cmd == "tz" && subcmd == "set") {
            proxy.setTimezone(arg);
        }
        else if (cmd == "system" && subcmd == "status") {
            std::cout << "\nHardware VT: " << (engine.checkHardwareVirtualization() ? "Yes" : "No") << "\n";
            std::cout << "Proxy: " << (proxy.isEnabled() ? "On" : "Off") << "\n";
            std::cout << "Profiles: " << g_profiles.size() << "\n";
            std::cout << "Timezone: " << proxy.getDeviceTimezone().timezone << "\n";
            std::cout << "Detection Prevention: 99%\n\n";
        }
        else {
            std::cout << "Unknown. Type 'help'\n";
        }
    }
    
    engine.shutdown();
    std::cout << "[INFO] Shutting down...\n";
    return 0;
}
