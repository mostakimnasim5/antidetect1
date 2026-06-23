#include "AntiDetectCore.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#define CLEAR_SCREEN system("cls")
#else
#include <unistd.h>
#define CLEAR_SCREEN system("clear")
#endif

using namespace AntiDetect;

void printBanner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════════════════╗
║                                                                              ║
║   ██████╗ ███████╗███████╗ ██████╗██╗   ██╗███████╗                          ║
║   ██╔══██╗██╔════╝██╔════╝██╔════╝██║   ██║██╔════╝                          ║
║   ██████╔╝█████╗  ███████╗██║     ██║   ██║█████╗                            ║
║   ██╔══██╗██╔══╝  ╚════██║██║     ██║   ██║██╔══╝                            ║
║   ██║  ██║███████╗███████║╚██████╗╚██████╔╝███████╗                          ║
║   ╚═╝  ╚═╝╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝                          ║
║                                                                              ║
║   ███████╗███████╗ ██████╗ █████╗ ██████╗ ███████╗                          ║
║   ██╔════╝██╔════╝██╔════╝██╔══██╗██╔══██╗██╔════╝                          ║
║   █████╗  ███████╗██║     ███████║██████╔╝█████╗                            ║
║   ██╔══╝  ╚════██║██║     ██╔══██║██╔═══╝ ██╔══╝                            ║
║   ███████╗███████║╚██████╗██║  ██║██║     ███████╗                          ║
║   ╚══════╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝     ╚══════╝                          ║
║                                                                              ║
║   ═══════════════════════════════════════════════════════════════════════    ║
║   ▓▓▓  Enterprise Anti-Detect System v1.0.0  ▓▓▓                              ║
║   ═══════════════════════════════════════════════════════════════════════    ║
║                                                                              ║
║   [Professional Device Fingerprinting & Spoofing Suite]                      ║
║   [Commercial License - Elite Level]                                          ║
║                                                                              ║
╚══════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printUsage(const std::string& programName) {
    std::cout << "\nUsage: " << programName << " [OPTIONS] [COMMAND]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  -h, --help          Show this help message\n";
    std::cout << "  -v, --version       Show version information\n";
    std::cout << "  -d, --debug         Enable debug logging\n";
    std::cout << "  -l, --log FILE      Set log file path\n\n";
    
    std::cout << "COMMANDS:\n\n";
    
    std::cout << "  DEVICE MANAGEMENT:\n";
    std::cout << "    list-devices           List connected devices\n";
    std::cout << "    connect <address>      Connect to device (IP:PORT or USB)\n";
    std::cout << "    disconnect             Disconnect from device\n";
    std::cout << "    select <serial>       Select specific device\n";
    std::cout << "    device-info           Show connected device info\n\n";
    
    std::cout << "  DEVICE FINGERPRINT:\n";
    std::cout << "    spoof-manufacturer <name>    Spoof device manufacturer\n";
    std::cout << "    spoof-model <model>          Spoof device model\n";
    std::cout << "    spoof-brand <brand>          Spoof device brand\n";
    std::cout << "    spoof-android <version>      Spoof Android version\n";
    std::cout << "    spoof-build <fingerprint>     Spoof build fingerprint\n";
    std::cout << "    spoof-resolution <W>x<H>    Spoof screen resolution\n";
    std::cout << "    spoof-density <dpi>          Spoof screen density\n";
    std::cout << "    spoof-gpu <vendor> <render>  Spoof GPU info\n\n";
    
    std::cout << "  NETWORK SPOOFING:\n";
    std::cout << "    spoof-mac <address>          Spoof MAC address\n";
    std::cout << "    spoof-carrier <name>         Spoof carrier name\n";
    std::cout << "    spoof-country <code>         Spoof country code\n";
    std::cout << "    spoof-location <lat> <lon>  Spoof GPS location\n";
    std::cout << "    enable-gps-spoofing         Enable GPS spoofing\n";
    std::cout << "    enable-mock-location        Enable mock location\n\n";
    
    std::cout << "  SYSTEM CONTROL:\n";
    std::cout << "    set-timezone <tz>           Set timezone\n";
    std::cout << "    set-locale <locale>         Set locale (e.g., en_US)\n";
    std::cout << "    set-language <lang>         Set language\n";
    std::cout << "    set-battery <level> <status> Set battery status\n";
    std::cout << "    enable-selinux              Enable SELinux\n";
    std::cout << "    disable-selinux             Disable SELinux\n";
    std::cout << "    enable-debug                Enable debug mode\n";
    std::cout << "    disable-debug               Disable debug mode\n\n";
    
    std::cout << "  PROFILE MANAGEMENT:\n";
    std::cout << "    list-profiles               List all profiles\n";
    std::cout << "    apply-profile <id>          Apply a profile\n";
    std::cout << "    create-profile <name>       Create new profile\n";
    std::cout << "    delete-profile <id>         Delete a profile\n";
    std::cout << "    export-config <path>        Export configuration\n";
    std::cout << "    import-config <path>        Import configuration\n\n";
    
    std::cout << "  SYSTEM:\n";
    std::cout << "    reset                       Reset device to original state\n";
    std::cout << "    current-fingerprint         Show current fingerprint\n";
    std::cout << "    system-status               Show system status\n";
    std::cout << "    backup-state                Backup current state\n";
    std::cout << "    restore-state               Restore backed up state\n\n";
}

void printResult(const AntiDetectResult& result) {
    if (result.success) {
        std::cout << "[SUCCESS] " << result.message << "\n";
    } else {
        std::cout << "[ERROR] " << result.error << "\n";
    }
}

void printSeparator() {
    std::cout << "\n────────────────────────────────────────────────────────────\n";
}

int main(int argc, char* argv[]) {
    bool debugMode = false;
    std::string logFile;
    std::vector<std::string> commands;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printBanner();
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-v" || arg == "--version") {
            std::cout << "AntiDetectPro v1.0.0\n";
            std::cout << "Enterprise Anti-Detect System\n";
            std::cout << "Copyright (c) 2024 - Commercial License\n";
            return 0;
        }
        else if (arg == "-d" || arg == "--debug") {
            debugMode = true;
        }
        else if (arg == "-l" || arg == "--log") {
            if (i + 1 < argc) {
                logFile = argv[++i];
            }
        }
        else {
            commands.push_back(arg);
        }
    }
    
    auto& logger = Logger::getInstance();
    if (debugMode) {
        logger.setLogLevel(LogLevel::DEBUG);
    }
    if (!logFile.empty()) {
        logger.setLogFile(logFile);
    }
    
    printBanner();
    
    auto& core = AntiDetectCore::getInstance();
    
    if (!core.initialize()) {
        std::cerr << "[FATAL] Failed to initialize AntiDetectPro\n";
        return 1;
    }
    
    printSeparator();
    std::cout << "System Status: " << (core.getStatus() == AntiDetectStatus::READY ? "READY" : "INITIALIZING") << "\n";
    std::cout << "ADB Connected Devices: " << core.listDevices().size() << "\n";
    printSeparator();
    
    if (commands.empty()) {
        std::cout << "\nInteractive Mode - Type 'help' for commands or 'exit' to quit\n\n";
        
        std::string input;
        while (true) {
            std::cout << "AntiDetectPro> ";
            std::getline(std::cin, input);
            
            if (input.empty()) continue;
            
            if (input == "exit" || input == "quit") {
                std::cout << "Shutting down...\n";
                break;
            }
            
            if (input == "help") {
                printUsage(argv[0]);
                continue;
            }
            
            std::istringstream iss(input);
            std::string cmd;
            iss >> cmd;
            
            if (cmd == "list-devices" || cmd == "devices") {
                auto devices = core.listDevices();
                if (devices.empty()) {
                    std::cout << "No devices connected\n";
                } else {
                    std::cout << "Connected Devices:\n";
                    for (const auto& device : devices) {
                        std::cout << "  - " << device << "\n";
                    }
                }
            }
            else if (cmd == "connect" && commands.size() > 1) {
                std::string address;
                iss >> address;
                printResult(core.connect(address));
            }
            else if (cmd == "disconnect") {
                printResult(core.disconnect());
            }
            else if (cmd == "device-info") {
                auto info = core.getDeviceInfo();
                std::cout << "Device Information:\n";
                for (const auto& [key, value] : info) {
                    std::cout << "  " << key << ": " << value << "\n";
                }
            }
            else if (cmd == "spoof-manufacturer") {
                std::string manufacturer;
                iss >> manufacturer;
                printResult(core.spoofManufacturer(manufacturer));
            }
            else if (cmd == "spoof-model") {
                std::string model;
                iss >> model;
                printResult(core.spoofModel(model));
            }
            else if (cmd == "spoof-brand") {
                std::string brand;
                iss >> brand;
                printResult(core.spoofBrand(brand));
            }
            else if (cmd == "spoof-android") {
                std::string version;
                iss >> version;
                printResult(core.spoofAndroidVersion(version));
            }
            else if (cmd == "spoof-mac") {
                std::string mac;
                iss >> mac;
                printResult(core.spoofMACAddress(mac));
            }
            else if (cmd == "spoof-carrier") {
                std::string carrier;
                iss >> carrier;
                printResult(core.spoofCarrier(carrier));
            }
            else if (cmd == "spoof-location") {
                double lat, lon;
                iss >> lat >> lon;
                printResult(core.spoofLocation(lat, lon));
            }
            else if (cmd == "enable-gps-spoofing") {
                printResult(core.enableGPSSpoofing());
            }
            else if (cmd == "enable-mock-location") {
                printResult(core.enableMockLocation());
            }
            else if (cmd == "set-timezone") {
                std::string tz;
                iss >> tz;
                printResult(core.setTimezone(tz));
            }
            else if (cmd == "set-locale") {
                std::string locale;
                iss >> locale;
                printResult(core.setLocale(locale));
            }
            else if (cmd == "set-language") {
                std::string lang;
                iss >> lang;
                printResult(core.setLanguage(lang));
            }
            else if (cmd == "enable-selinux") {
                printResult(core.enableSELinux());
            }
            else if (cmd == "disable-selinux") {
                printResult(core.disableSELinux());
            }
            else if (cmd == "enable-debug") {
                printResult(core.enableDebugMode());
            }
            else if (cmd == "disable-debug") {
                printResult(core.disableDebugMode());
            }
            else if (cmd == "list-profiles" || cmd == "profiles") {
                auto profiles = core.listProfiles();
                if (profiles.empty()) {
                    std::cout << "No profiles found\n";
                } else {
                    std::cout << "Available Profiles:\n";
                    for (const auto& profile : profiles) {
                        std::cout << "  [" << profile.at("id") << "] " << profile.at("name") 
                                  << " - " << profile.at("category") << "\n";
                    }
                }
            }
            else if (cmd == "apply-profile") {
                std::string profileId;
                iss >> profileId;
                printResult(core.applyProfile(profileId));
            }
            else if (cmd == "reset") {
                printResult(core.resetDevice());
            }
            else if (cmd == "current-fingerprint" || cmd == "fingerprint") {
                auto fp = core.getCurrentFingerprint();
                std::cout << "Current Device Fingerprint:\n";
                for (const auto& [key, value] : fp) {
                    std::cout << "  " << key << ": " << value << "\n";
                }
            }
            else if (cmd == "system-status" || cmd == "status") {
                auto status = core.getSystemStatus();
                std::cout << "System Status:\n";
                for (const auto& [key, value] : status) {
                    std::cout << "  " << key << ": " << value << "\n";
                }
            }
            else if (cmd == "clear" || cmd == "cls") {
                CLEAR_SCREEN;
                printBanner();
            }
            else {
                std::cout << "Unknown command: " << cmd << ". Type 'help' for available commands.\n";
            }
        }
    }
    else {
        std::string cmd = commands[0];
        
        if (cmd == "list-devices") {
            auto devices = core.listDevices();
            for (const auto& device : devices) {
                std::cout << device << "\n";
            }
        }
        else if (cmd == "connect" && commands.size() >= 2) {
            printResult(core.connect(commands[1]));
        }
        else if (cmd == "disconnect") {
            printResult(core.disconnect());
        }
        else if (cmd == "spoof-manufacturer" && commands.size() >= 2) {
            printResult(core.spoofManufacturer(commands[1]));
        }
        else if (cmd == "spoof-model" && commands.size() >= 2) {
            printResult(core.spoofModel(commands[1]));
        }
        else if (cmd == "spoof-brand" && commands.size() >= 2) {
            printResult(core.spoofBrand(commands[1]));
        }
        else if (cmd == "spoof-android" && commands.size() >= 2) {
            printResult(core.spoofAndroidVersion(commands[1]));
        }
        else if (cmd == "spoof-mac" && commands.size() >= 2) {
            printResult(core.spoofMACAddress(commands[1]));
        }
        else if (cmd == "spoof-carrier" && commands.size() >= 2) {
            printResult(core.spoofCarrier(commands[1]));
        }
        else if (cmd == "spoof-location" && commands.size() >= 3) {
            try {
                double lat = std::stod(commands[1]);
                double lon = std::stod(commands[2]);
                printResult(core.spoofLocation(lat, lon));
            } catch (const std::exception& e) {
                std::cout << "[ERROR] Invalid coordinates: " << e.what() << "\n";
            }
        }
        else if (cmd == "list-profiles") {
            auto profiles = core.listProfiles();
            for (const auto& profile : profiles) {
                std::cout << "[" << profile.at("id") << "] " << profile.at("name") 
                          << " - " << profile.at("category") << "\n";
            }
        }
        else if (cmd == "apply-profile" && commands.size() >= 2) {
            printResult(core.applyProfile(commands[1]));
        }
        else if (cmd == "reset") {
            printResult(core.resetDevice());
        }
        else if (cmd == "current-fingerprint") {
            auto fp = core.getCurrentFingerprint();
            for (const auto& [key, value] : fp) {
                std::cout << key << ": " << value << "\n";
            }
        }
        else {
            std::cerr << "Unknown command or missing arguments: " << cmd << "\n";
            printUsage(argv[0]);
        }
    }
    
    core.shutdown();
    
    return 0;
}
