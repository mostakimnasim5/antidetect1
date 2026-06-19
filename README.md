# AntiDetectPro

**Enterprise-Grade Android Anti-Detection System**

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-Commercial-red)
![C++](https://img.shields.io/badge/C%2B%2B-20-green)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-orange)

---

## Overview

AntiDetectPro is a **commercial-grade, enterprise-level** Android anti-detection system designed for professional application testing and browser fingerprint testing. Built entirely in **C++** without JavaScript or third-party APIs, it operates at the **Android OS level** to modify device fingerprints and tracking parameters with precision.

## Key Features

### 🔒 Device Fingerprint Control
- **Manufacturer Spoofing**: Change device manufacturer (Samsung, Google, Xiaomi, etc.)
- **Model Spoofing**: Mimic any Android device model
- **Build Fingerprint Modification**: Complete build fingerprint customization
- **Android Version Control**: Spoof Android version from 8.0 to 14.0
- **Screen Resolution & Density**: Modify display metrics for canvas fingerprinting
- **Hardware Information**: Spoof GPU, bootloader, radio version

### 📡 Network Identity Management
- **MAC Address Spoofing**: WiFi, Bluetooth, Ethernet MAC addresses
- **Carrier Spoofing**: Simulate any carrier/network operator
- **Location Spoofing**: GPS coordinate manipulation
- **Network Type Simulation**: 5G, 4G/LTE, 3G, 2G

### ⚙️ System Control
- **SELinux Management**: Enable/disable SELinux enforcement
- **Debug Mode Control**: Toggle debug flags
- **Timezone & Locale**: Full localization control
- **Battery Status Simulation**: Mock battery levels and charging states
- **Root Detection Bypass**: Hide root status from detection apps

### 👤 Profile Management
- **Multiple Profiles**: Create and manage unlimited device profiles
- **Quick Profile Switching**: One-command profile application
- **Import/Export**: Share and backup profiles
- **Pre-built Profiles**: Samsung, Google Pixel, Xiaomi, OnePlus included

## Architecture

```
AntiDetectPro/
├── include/                    # Header files
│   ├── AntiDetectCore.hpp     # Main API interface
│   ├── ADBManager.hpp          # ADB communication layer
│   ├── DeviceFingerprint.hpp   # Device fingerprint engine
│   ├── NetworkSpoofer.hpp      # Network spoofing module
│   ├── SystemManager.hpp       # System control module
│   ├── ProfileManager.hpp      # Profile management
│   ├── Logger.hpp              # Logging system
│   └── Config.hpp              # Configuration management
├── src/                        # Source files
│   ├── main.cpp                # CLI application
│   ├── AntiDetectCore.cpp      # Core implementation
│   ├── ADBManager.cpp          # ADB operations
│   ├── DeviceFingerprint.cpp   # Fingerprint manipulation
│   ├── NetworkSpoofer.cpp      # Network spoofing
│   ├── SystemManager.cpp       # System control
│   ├── ProfileManager.cpp      # Profile handling
│   ├── Logger.cpp              # Logging
│   └── Config.cpp              # Configuration
├── CMakeLists.txt             # Build configuration
└── README.md                   # This file
```

## Requirements

### Build Requirements
- **C++20 compatible compiler**
  - GCC 10+ (Linux/macOS)
  - MSVC 2019+ (Windows)
  - Clang 12+ (macOS)
- **CMake 3.20+**
- **OpenSSL** (for cryptographic functions)
- **Threads** (POSIX threads)

### Runtime Requirements
- **ADB (Android Debug Bridge)** installed and in PATH
- **Android device** with USB debugging enabled or network ADB
- **Root access** (optional, for some advanced features)

## Installation

### Build from Source

```bash
# Clone the repository
git clone https://github.com/mostakimnasim5/antidetect1.git
cd antidetect1

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Install (optional)
cmake --install .
```

### Windows

```powershell
# Using Visual Studio Developer Command Prompt
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential cmake libssl-dev

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS

```bash
# Install dependencies
brew install cmake openssl

# Build
mkdir build && cd build
cmake .. -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)
make -j$(sysctl -n hw.ncpu)
```

## Usage

### Command Line Interface

```bash
# Show help
./AntiDetectPro --help

# List connected devices
./AntiDetectPro list-devices

# Connect to device via network
./AntiDetectPro connect 192.168.1.100:5555

# Spoof device manufacturer
./AntiDetectPro spoof-manufacturer Samsung

# Spoof device model
./AntiDetectPro spoof-model "Galaxy S21 Ultra"

# Spoof MAC address
./AntiDetectPro spoof-mac AA:BB:CC:DD:EE:FF

# Spoof carrier
./AntiDetectPro spoof-carrier Verizon

# Spoof location (latitude, longitude)
./AntiDetectPro spoof-location 40.7128 -74.0060

# Set timezone
./AntiDetectPro set-timezone "America/New_York"

# List available profiles
./AntiDetectPro list-profiles

# Apply a profile
./AntiDetectPro apply-profile samsung_galaxy_s21

# Reset device to original state
./AntiDetectPro reset
```

### Interactive Mode

```bash
./AntiDetectPro

# In interactive mode:
AntiDetectPro> list-devices
AntiDetectPro> connect 192.168.1.100:5555
AntiDetectPro> spoof-model "Pixel 7 Pro"
AntiDetectPro> spoof-mac 00:11:22:33:44:55
AntiDetectPro> apply-profile google_pixel
AntiDetectPro> reset
AntiDetectPro> exit
```

### C++ API Usage

```cpp
#include "AntiDetectCore.hpp"

int main() {
    auto& core = AntiDetectCore::getInstance();
    
    // Initialize
    core.initialize();
    
    // Connect to device
    core.connect("192.168.1.100:5555");
    
    // Spoof device fingerprint
    core.spoofManufacturer("Samsung");
    core.spoofModel("SM-G998B");
    core.spoofAndroidVersion("13");
    core.spoofScreenResolution(1440, 3200);
    core.spoofScreenDensity(640);
    
    // Network spoofing
    core.spoofMACAddress("AA:BB:CC:DD:EE:FF");
    core.spoofCarrier("Verizon");
    core.spoofLocation(40.7128, -74.0060);
    core.enableMockLocation();
    
    // System control
    core.setTimezone("America/New_York");
    core.setLocale("en_US");
    core.setBatteryStatus(85, "Charging");
    
    // Apply profile
    core.applyProfile("profile_id_here");
    
    // Reset when done
    core.resetDevice();
    
    // Shutdown
    core.shutdown();
    
    return 0;
}
```

## Pre-built Profiles

AntiDetectPro includes several pre-configured profiles:

| Profile | Manufacturer | Model | Description |
|---------|-------------|-------|-------------|
| `Samsung` | Samsung | Galaxy S21 Ultra | Samsung flagship configuration |
| `Google` | Google | Pixel 7 Pro | Google flagship configuration |
| `Xiaomi` | Xiaomi | Mi 13 Pro | Xiaomi flagship configuration |
| `OnePlus` | OnePlus | OnePlus 11 | OnePlus flagship configuration |
| `Generic` | Generic | Android Device | Generic Android configuration |

## Advanced Features

### Profile Creation

```bash
# Create custom profile via CLI
./AntiDetectPro create-profile "My Custom Device"

# Or via API
auto profile = profileManager.createDefaultProfile("Custom Device", "Custom");
profile.device.manufacturer = "Sony";
profile.device.model = "Xperia 1 IV";
profile.device.androidVersion = "13";
profileManager.createProfile(profile);
```

### Batch Operations

```cpp
// Apply multiple changes at once
std::map<std::string, std::string> customProfile = {
    {"ro.product.manufacturer", "Samsung"},
    {"ro.product.model", "SM-G998B"},
    {"ro.build.version.release", "13"},
    {"ro.sf.lcd_density", "640"},
    {"ro.carrier", "Verizon"}
};

core.applyDeviceProfile(customProfile);
```

### Export/Import

```bash
# Export all profiles
./AntiDetectPro export-config ./my-profiles

# Import profile
./AntiDetectPro import-config ./my-profile.json
```

## Security Considerations

⚠️ **Important Disclaimers:**

1. **Legal Use Only**: This software is intended for legitimate purposes such as:
   - Application testing
   - Browser automation testing
   - Security research
   - Personal privacy protection

2. **No Illegal Activities**: Do not use this software for:
   - Fraud
   - Account manipulation that violates terms of service
   - Any illegal activities

3. **Device Responsibility**: Users are solely responsible for:
   - Backing up device data before use
   - Understanding the risks of modifying device properties
   - Ensuring compliance with applicable laws and regulations

## Troubleshooting

### Common Issues

**"ADB not found"**
```bash
# Install ADB
# Ubuntu/Debian
sudo apt-get install adb

# macOS
brew install android-platform-tools

# Windows - Download from Android SDK Manager
```

**"Device not found"**
```bash
# Enable USB debugging on your Android device
# Settings > Developer Options > USB Debugging

# Check device connection
adb devices

# Restart ADB server
adb kill-server
adb start-server
```

**"Permission denied"**
```bash
# Some features require root access
# Enable root in developer options
# Settings > Developer Options > Root Access > Apps and ADB
```

### Debug Mode

```bash
# Enable debug logging
./AntiDetectPro --debug --log ./debug.log

# Or in interactive mode
AntiDetectPro> debug on
```

## Development

### Adding New Spoofing Capabilities

```cpp
// In DeviceFingerprint.hpp
FingerprintResult spoofNewProperty(const std::string& value);

// In DeviceFingerprint.cpp
FingerprintResult DeviceFingerprint::spoofNewProperty(const std::string& value) {
    FingerprintResult result = {false, "new.property", "", value, ""};
    result.originalValue = getPropertyValue("new.property");
    result.success = applyPropertyChange("new.property", value);
    return result;
}
```

### Building Documentation

```bash
# Generate API documentation (requires Doxygen)
doxygen Doxyfile
```

## License

**Commercial License - Elite Level**

This software is proprietary and commercial. All rights reserved.

- Redistribution of source code is not permitted
- Redistribution of compiled binaries requires written permission
- Commercial use requires a valid license

## Support

For support, documentation, and licensing inquiries:
- GitHub Issues: [Report Issues](https://github.com/mostakimnasim5/antidetect1/issues)
- Email: [Contact Author](mailto:support@example.com)

## Changelog

### v1.0.0 (2024)
- Initial release
- Device fingerprint spoofing
- Network identity spoofing
- System control features
- Profile management
- CLI interface
- C++ API

## Acknowledgments

- Android Open Source Project
- ADB Protocol Documentation
- OpenSSL Project

---

**AntiDetectPro** - Enterprise Anti-Detection System
Copyright (c) 2024 - All Rights Reserved