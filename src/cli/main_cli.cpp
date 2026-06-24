/**
 * VirtualPhonePro CLI - Command Line Interface
 */

#include "core/CoreEngine.hpp"
#include <iostream>

using namespace VirtualPhonePro;

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
║   ▓▓▓  VirtualPhonePro v1.0 - Enterprise Android VM Platform  ▓▓▓  ║
║   ═══════════════════════════════════════════════════════════════  ║
║                                                                      ║
║   Unlimited Android VMs with Unique Fingerprints                    ║
║   Zero VM Detection - Real Device Appearance                         ║
║                                                                      ║
╚══════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printHelp() {
    std::cout << "\nAvailable Commands:\n";
    std::cout << "  vm create <name>           - Create new VM\n";
    std::cout << "  vm start <id>              - Start VM\n";
    std::cout << "  vm stop <id>               - Stop VM\n";
    std::cout << "  vm list                    - List all VMs\n";
    std::cout << "  vm delete <id>             - Delete VM\n";
    std::cout << "  profile generate           - Generate random profile\n";
    std::cout << "  system status              - Show system status\n";
    std::cout << "  help                       - Show this help\n";
    std::cout << "  exit                       - Exit\n\n";
}

int main(int argc, char* argv[]) {
    auto& engine = CoreEngine::getInstance();
    
    if (!engine.initialize()) {
        std::cerr << "[ERROR] Failed to initialize VirtualPhonePro\n";
        return 1;
    }
    
    printBanner();
    std::cout << "\n[INFO] VirtualPhonePro initialized successfully\n";
    std::cout << "[INFO] Type 'help' for available commands\n\n";
    
    std::string input;
    while (true) {
        std::cout << "VirtualPhonePro> ";
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        if (input == "exit" || input == "quit") break;
        if (input == "help") { printHelp(); continue; }
        
        std::istringstream iss(input);
        std::string cmd, subcmd;
        iss >> cmd >> subcmd;
        
        if (cmd == "vm") {
            if (subcmd == "list") {
                auto vms = engine.listVMs();
                std::cout << "\nVirtual Machines:\n";
                for (const auto& vm : vms) {
                    std::cout << "  " << vm.id << " - " << vm.name << " (" << vm.state << ")\n";
                }
                std::cout << "\n";
            }
            else if (subcmd == "create") {
                VMConfig config;
                config.name = "New VM";
                config.cpuCores = 4;
                config.memoryMB = 4096;
                config.storageMB = 16384;
                std::string vmId = engine.createVM(config);
                std::cout << "Created VM: " << vmId << "\n";
            }
        }
        else if (cmd == "profile") {
            if (subcmd == "generate") {
                auto profile = engine.generateRandomProfile("US");
                std::cout << "\nGenerated Profile:\n";
                std::cout << "  Manufacturer: " << profile.manufacturer << "\n";
                std::cout << "  Model: " << profile.model << "\n";
                std::cout << "  Android: " << profile.androidVersion << "\n";
                std::cout << "  Fingerprint: " << profile.fingerprint << "\n\n";
            }
        }
        else if (cmd == "system") {
            if (subcmd == "status") {
                std::cout << "\nSystem Status:\n";
                std::cout << "  Hardware VT: " << (engine.checkHardwareVirtualization() ? "Available" : "Not Available") << "\n";
            }
        }
        else {
            std::cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }
    
    engine.shutdown();
    std::cout << "\n[INFO] VirtualPhonePro shutting down...\n";
    return 0;
}
