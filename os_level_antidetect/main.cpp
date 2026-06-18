/**
 * OS-Level Anti-Detect System
 * Professional Command Line Interface
 */

#include "core/system_fingerprint.h"
#include "core/advanced_fingerprint.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <csignal>
#include <getopt.h>

namespace AntiDetect {

void printBanner() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════════════════════════╗
║                                                                               ║
║     █████╗ ██╗     ███████╗██╗  ██╗    ██╗  ██╗██╗   ██╗███╗   ██╗████████╗   ║
║    ██╔══██╗██║     ██╔════╝╚██╗██╔╝    ██║  ██║██║   ██║████╗  ██║╚══██╔══╝   ║
║    ███████║██║     █████╗   ╚███╔╝     ███████║██║   ██║██╔██╗ ██║   ██║      ║
║    ██╔══██║██║     ██╔══╝   ██╔██╗     ██╔══██║██║   ██║██║╚██╗██║   ██║      ║
║    ██║  ██║███████╗███████╗██╔╝ ██╗    ██║  ██║╚██████╔╝██║ ╚████║   ██║      ║
║    ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝    ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝      ║
║                                                                               ║
║                    OS-LEVEL ANTI-DETECT SYSTEM v2.0                          ║
║                    Professional Fingerprint Manipulation                      ║
║                                                                               ║
╚═══════════════════════════════════════════════════════════════════════════════╝
)";
}

void printUsage(const char* program_name) {
    std::cout << "\nUsage: " << program_name << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -m, --mode MODE              Operation mode (interactive, generate, apply, reset)\n";
    std::cout << "  -p, --preset NAME            Apply preset (windows_dell_xps, macos_macbook_pro, etc.)\n";
    std::cout << "  -i, --info                   Show current system fingerprint\n";
    std::cout << "  -s, --spoof                  Spoof system identifiers\n";
    std::cout << "  -r, --reset                  Reset to real system fingerprint\n";
    std::cout << "  -l, --list                   List available presets\n";
    std::cout << "  -g, --generate               Generate new fingerprint profile\n";
    std::cout << "  -o, --output FILE            Save profile to file\n";
    std::cout << "  -c, --check                  Check system fingerprint status\n";
    std::cout << "  -h, --help                   Show this help message\n";
    std::cout << "  -v, --version                Show version information\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " --mode interactive\n";
    std::cout << "  " << program_name << " --preset windows_dell_xps\n";
    std::cout << "  " << program_name << " --generate --output my_profile.txt\n";
    std::cout << "  " << program_name << " --info\n";
    std::cout << "\n";
}

void printSystemInfo() {
    auto& fp_mgr = SystemFingerprintManager::getInstance();
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    SYSTEM FINGERPRINT                       ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    
    auto hw_ids = fp_mgr.getHardwareIdentifiers();
    std::cout << "║ HARDWARE IDENTIFIERS:                                       ║\n";
    std::cout << "║   Machine ID:     " << std::setw(43) << hw_ids.machine_id << " ║\n";
    std::cout << "║   Product UUID:   " << std::setw(43) << hw_ids.product_uuid << " ║\n";
    std::cout << "║   Board Serial:   " << std::setw(43) << hw_ids.board_serial << " ║\n";
    std::cout << "║   BIOS Version:   " << std::setw(43) << hw_ids.bios_version << " ║\n";
    
    auto net_ids = fp_mgr.getNetworkIdentifiers();
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ NETWORK IDENTIFIERS:                                        ║\n";
    std::cout << "║   Hostname:       " << std::setw(43) << net_ids.hostname << " ║\n";
    std::cout << "║   MAC Address:    " << std::setw(43) << net_ids.mac_address << " ║\n";
    
    auto sys_info = fp_mgr.getSystemInfo();
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ SYSTEM INFORMATION:                                         ║\n";
    std::cout << "║   OS Version:     " << std::setw(43) << sys_info.os_version << " ║\n";
    std::cout << "║   Kernel:         " << std::setw(43) << sys_info.kernel_version << " ║\n";
    std::cout << "║   Architecture:   " << std::setw(43) << sys_info.architecture << " ║\n";
    std::cout << "║   CPU Model:      " << std::setw(43) << sys_info.cpu_model << " ║\n";
    std::cout << "║   CPU Cores:      " << std::setw(43) << sys_info.cpu_cores << " ║\n";
    std::cout << "║   Memory:         " << std::setw(43) << sys_info.memory_total << " ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
}

void printPresetList() {
    auto& preset_mgr = PresetManager::getInstance();
    auto presets = preset_mgr.listPresets();
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    AVAILABLE PRESETS                        ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    
    for (const auto& name : presets) {
        auto preset = preset_mgr.getPreset(name);
        std::cout << "║   " << std::setw(60) << name << "  ║\n";
        std::cout << "║     └─ " << std::setw(55) << preset.description << "  ║\n";
        std::cout << "║        Region: " << std::setw(48) << preset.region << "  ║\n";
    }
    
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
}

void interactiveMode() {
    int choice;
    
    while (true) {
        std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║                    MAIN MENU                                ║
╠══════════════════════════════════════════════════════════════╣
║  [1] Show Current System Fingerprint                        ║
║  [2] Apply Fingerprint Preset                              ║
║  [3] Generate New Fingerprint Profile                      ║
║  [4] List Available Presets                                ║
║  [5] Check Fingerprint Status                              ║
║  [6] Reset to Real System                                  ║
║  [7] Create Custom Profile                                 ║
║  [8] Save Profile to File                                  ║
║  [0] Exit                                                  ║
╚══════════════════════════════════════════════════════════════╝
)";
        std::cout << "\nEnter choice: ";
        
        if (!(std::cin >> choice)) {
            std::cerr << "Invalid input!\n";
            break;
        }
        
        switch (choice) {
            case 1:
                printSystemInfo();
                break;
                
            case 2: {
                printPresetList();
                std::cout << "Enter preset name: ";
                std::string preset_name;
                std::cin >> preset_name;
                
                auto& fp_mgr = SystemFingerprintManager::getInstance();
                if (fp_mgr.applyFingerprintPreset(preset_name)) {
                    std::cout << "✓ Preset applied successfully!\n";
                } else {
                    std::cout << "✗ Failed to apply preset (root required)\n";
                }
                break;
            }
            
            case 3: {
                auto& adv_fp = AdvancedFingerprintGenerator::getInstance();
                auto profile = adv_fp.generateProfile("Windows");
                
                std::cout << "\n[Generated Profile]\n";
                std::cout << "ID:              " << profile.profile_id << "\n";
                std::cout << "User Agent:      " << profile.user_agent << "\n";
                std::cout << "Platform:        " << profile.platform << "\n";
                std::cout << "Screen:          " << profile.screen_resolution << "\n";
                std::cout << "WebGL Vendor:    " << profile.webgl_vendor << "\n";
                std::cout << "WebGL Renderer:  " << profile.webgl_renderer << "\n";
                break;
            }
            
            case 4:
                printPresetList();
                break;
                
            case 5: {
                auto& fp_mgr = SystemFingerprintManager::getInstance();
                auto hw_ids = fp_mgr.getHardwareIdentifiers();
                std::cout << "\n[Fingerprint Status]\n";
                std::cout << "Machine ID:      " << hw_ids.machine_id.substr(0, 16) << "...\n";
                std::cout << "Product UUID:    " << hw_ids.product_uuid << "\n";
                break;
            }
            
            case 6: {
                auto& fp_mgr = SystemFingerprintManager::getInstance();
                if (fp_mgr.resetToRealSystem()) {
                    std::cout << "✓ System reset to real fingerprint\n";
                } else {
                    std::cout << "✗ Reset failed (root required)\n";
                }
                break;
            }
            
            case 7: {
                BrowserProfile profile;
                std::cout << "\n[Create Custom Profile]\n";
                std::cout << "User Agent: ";
                std::cin >> profile.user_agent;
                std::cout << "Platform: ";
                std::cin >> profile.platform;
                std::cout << "Screen Resolution: ";
                std::cin >> profile.screen_resolution;
                std::cout << "Timezone: ";
                std::cin >> profile.timezone;
                
                auto& db_mgr = ProfileDatabaseManager::getInstance();
                db_mgr.addProfile(profile);
                std::cout << "✓ Profile created and saved\n";
                break;
            }
            
            case 8: {
                auto& db_mgr = ProfileDatabaseManager::getInstance();
                if (db_mgr.saveDatabase("profiles.db")) {
                    std::cout << "✓ Profiles saved to profiles.db\n";
                } else {
                    std::cout << "✗ Failed to save profiles\n";
                }
                break;
            }
            
            case 0:
                std::cout << "Goodbye!\n";
                return;
                
            default:
                std::cout << "Invalid option!\n";
        }
    }
}

} // namespace AntiDetect

int main(int argc, char* argv[]) {
    using namespace AntiDetect;
    
    // Parse command line arguments
    static struct option long_options[] = {
        {"mode", required_argument, 0, 'm'},
        {"preset", required_argument, 0, 'p'},
        {"info", no_argument, 0, 'i'},
        {"spoof", no_argument, 0, 's'},
        {"reset", no_argument, 0, 'r'},
        {"list", no_argument, 0, 'l'},
        {"generate", no_argument, 0, 'g'},
        {"output", required_argument, 0, 'o'},
        {"check", no_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    std::string mode, preset_name, output_file;
    bool show_info = false, list_presets = false, generate = false, check_status = false;
    
    while ((c = getopt_long(argc, argv, "m:p:isrlgo:chv", long_options, &option_index)) != -1) {
        switch (c) {
            case 'm':
                mode = optarg;
                break;
            case 'p':
                preset_name = optarg;
                break;
            case 'i':
                show_info = true;
                break;
            case 's':
                // Spoof mode - will be handled after preset
                break;
            case 'r':
                SystemFingerprintManager::getInstance().resetToRealSystem();
                std::cout << "✓ System reset to real fingerprint\n";
                return 0;
            case 'l':
                list_presets = true;
                break;
            case 'g':
                generate = true;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'c':
                check_status = true;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            case 'v':
                std::cout << "OS-Level Anti-Detect System v2.0\n";
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }
    
    // Print banner
    printBanner();
    
    // Handle operations
    if (show_info) {
        printSystemInfo();
    }
    
    if (list_presets) {
        printPresetList();
    }
    
    if (check_status) {
        auto& fp_mgr = SystemFingerprintManager::getInstance();
        auto hw_ids = fp_mgr.getHardwareIdentifiers();
        std::cout << "\n[Current Fingerprint Status]\n";
        std::cout << "Machine ID: " << hw_ids.machine_id << "\n";
        std::cout << "Product UUID: " << hw_ids.product_uuid << "\n";
        std::cout << "Board Serial: " << hw_ids.board_serial << "\n";
    }
    
    if (!preset_name.empty()) {
        std::cout << "\n[Applying Preset: " << preset_name << "]\n";
        auto& fp_mgr = SystemFingerprintManager::getInstance();
        if (fp_mgr.applyFingerprintPreset(preset_name)) {
            std::cout << "✓ Preset applied successfully!\n";
            printSystemInfo();
        } else {
            std::cout << "✗ Failed to apply preset\n";
            std::cout << "  Note: Root privileges may be required\n";
        }
    }
    
    if (generate) {
        std::cout << "\n[Generating Fingerprint Profile]\n";
        auto& adv_fp = AdvancedFingerprintGenerator::getInstance();
        auto profile = adv_fp.generateProfile("windows");
        
        std::cout << "\nGenerated Profile:\n";
        std::cout << "────────────────────────────────────────\n";
        std::cout << "Profile ID:    " << profile.profile_id << "\n";
        std::cout << "User Agent:    " << profile.user_agent << "\n";
        std::cout << "Platform:      " << profile.platform << "\n";
        std::cout << "Screen:        " << profile.screen_resolution << "\n";
        std::cout << "Color Depth:   " << profile.color_depth << "\n";
        std::cout << "Timezone:      " << profile.timezone << "\n";
        std::cout << "Language:      " << profile.accept_language << "\n";
        std::cout << "WebGL Vendor:  " << profile.webgl_vendor << "\n";
        std::cout << "WebGL Render:  " << profile.webgl_renderer << "\n";
        std::cout << "Canvas Hash:   " << profile.canvas_fingerprint << "\n";
        std::cout << "────────────────────────────────────────\n";
        
        if (!output_file.empty()) {
            std::ofstream file(output_file);
            if (file.is_open()) {
                file << "[BrowserProfile]\n";
                file << "profile_id=" << profile.profile_id << "\n";
                file << "user_agent=" << profile.user_agent << "\n";
                file << "platform=" << profile.platform << "\n";
                file << "screen_resolution=" << profile.screen_resolution << "\n";
                file << "timezone=" << profile.timezone << "\n";
                file << "webgl_vendor=" << profile.webgl_vendor << "\n";
                file << "webgl_renderer=" << profile.webgl_renderer << "\n";
                file << "canvas_fingerprint=" << profile.canvas_fingerprint << "\n";
                file.close();
                std::cout << "✓ Profile saved to: " << output_file << "\n";
            }
        }
    }
    
    if (mode == "interactive" || (argc == 1 && !show_info && !list_presets && preset_name.empty())) {
        interactiveMode();
    }
    
    return 0;
}