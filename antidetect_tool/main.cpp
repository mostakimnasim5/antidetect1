#include "antidetect.h"
#include <iostream>

void printBanner() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════╗\n";
    std::cout << "║     ANTIDETECT BROWSER TESTING TOOL       ║\n";
    std::cout << "║     C++ Implementation - For Testing      ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void printMenu() {
    std::cout << "Options:\n";
    std::cout << "  1. Generate New Fingerprint\n";
    std::cout << "  2. Display Current Fingerprint\n";
    std::cout << "  3. Save Profile to File\n";
    std::cout << "  4. Load Profile from File\n";
    std::cout << "  5. Generate Multiple Fingerprints\n";
    std::cout << "  6. Exit\n";
    std::cout << "\n> ";
}

int main(int argc, char* argv[]) {
    printBanner();
    
    Antidetect::AntidetectEngine engine;
    
    // If arguments provided, run in batch mode
    if (argc > 1) {
        std::string arg = argv[1];
        
        if (arg == "--generate" || arg == "-g") {
            engine.printAllFingerprints();
            return 0;
        }
        else if (arg == "--save" && argc > 2) {
            engine.saveProfile(argv[2]);
            return 0;
        }
        else if (arg == "--load" && argc > 2) {
            if (engine.loadProfile(argv[2])) {
                engine.printAllFingerprints();
            }
            return 0;
        }
        else if (arg == "--multi" && argc > 2) {
            int count = std::stoi(argv[2]);
            for (int i = 0; i < count; ++i) {
                std::cout << "\n[Profile " << (i + 1) << " of " << count << "]\n";
                engine.printAllFingerprints();
            }
            return 0;
        }
        else {
            std::cout << "Usage:\n";
            std::cout << "  " << argv[0] << " --generate       Generate and display fingerprint\n";
            std::cout << "  " << argv[0] << " --save <file>    Save current profile\n";
            std::cout << "  " << argv[0] << " --load <file>    Load profile from file\n";
            std::cout << "  " << argv[0] << " --multi <n>      Generate n fingerprints\n";
            return 1;
        }
    }
    
    // Interactive mode
    int choice;
    std::string filename;
    
    while (true) {
        printMenu();
        
        if (!(std::cin >> choice)) {
            std::cerr << "Invalid input!\n";
            break;
        }
        
        switch (choice) {
            case 1:
                std::cout << "\n[Generating new fingerprint...]\n";
                engine.generateRandomUserAgent();
                engine.generateRandomPlatform();
                engine.generateRandomLanguage();
                engine.generateRandomScreen();
                engine.generateRandomTimezone();
                engine.generateCanvasFingerprint();
                engine.generateWebGLFingerprint();
                engine.generateSystemInfo();
                std::cout << "New fingerprint generated!\n";
                break;
                
            case 2:
                engine.printAllFingerprints();
                break;
                
            case 3:
                std::cout << "Enter filename to save: ";
                std::cin >> filename;
                engine.saveProfile(filename);
                break;
                
            case 4:
                std::cout << "Enter filename to load: ";
                std::cin >> filename;
                engine.loadProfile(filename);
                break;
                
            case 5:
                std::cout << "How many fingerprints to generate? ";
                int count;
                std::cin >> count;
                for (int i = 0; i < count; ++i) {
                    std::cout << "\n[Profile " << (i + 1) << " of " << count << "]\n";
                    engine.printAllFingerprints();
                }
                break;
                
            case 6:
                std::cout << "Goodbye!\n";
                return 0;
                
            default:
                std::cout << "Invalid option!\n";
        }
    }
    
    return 0;
}