# OS-Level Anti-Detect System

Professional-grade anti-detection system that operates at the OS level using pure C++. No JavaScript or third-party APIs required.

## Features

### 🖥️ OS-Level System Manipulation
- **Machine ID Spoofing** - Modify `/etc/machine-id`
- **Product UUID Manipulation** - Spoof DMI product UUID
- **MAC Address Spoofing** - Change network interface MAC addresses
- **Hostname Spoofing** - Modify system hostname in real-time
- **DMI Information Manipulation** - Modify BIOS, board, and chassis information

### 🌐 Network-Level Protection
- Custom DNS configuration
- IPv6 disable option
- Proxy support
- Network identity masking

### 🎭 Browser Fingerprint Management
- **User Agent Generation** - 50+ real browser profiles
- **Canvas Fingerprint** - Random hash generation
- **WebGL Fingerprint** - Vendor/renderer spoofing
- **Audio Fingerprint** - Audio context fingerprinting
- **Font Fingerprint** - System font enumeration and hashing
- **Screen Resolution** - Multiple display configurations
- **Timezone Spoofing** - Global timezone support
- **Language Settings** - Multi-language support

### 📊 Pre-built Presets
| Preset | Description | Region |
|--------|-------------|--------|
| `windows_dell_xps` | Windows 10 on Dell XPS 15 | US |
| `macos_macbook_pro` | macOS Sonoma on MacBook Pro M3 | US |
| `ubuntu_desktop` | Ubuntu 22.04 LTS | UK |
| `windows_hp_envy` | Windows 11 on HP Envy | DE |
| `android_samsung` | Android 14 on Samsung Galaxy S24 | JP |

## Architecture

```
os_level_antidetect/
├── core/
│   ├── system_fingerprint.h      # Core system fingerprint API
│   ├── system_fingerprint.cpp    # OS-level implementation
│   ├── advanced_fingerprint.h    # Advanced fingerprint features
│   └── advanced_fingerprint.cpp  # Browser fingerprint generation
├── main.cpp                      # CLI interface
├── antidetect                    # Compiled binary
└── README.md
```

## Build

```bash
# Direct compilation
g++ -std=c++17 -O2 -pthread -o antidetect main.cpp core/*.cpp

# Or use CMake
mkdir build && cd build
cmake ..
make
```

## Usage

### Show System Fingerprint
```bash
./antidetect --info
```

### List Available Presets
```bash
./antidetect --list
```

### Apply a Preset
```bash
sudo ./antidetect --preset windows_dell_xps
```

### Generate New Profile
```bash
./antidetect --generate -o my_profile.txt
```

### Interactive Mode
```bash
./antidetect --mode interactive
```

### Reset to Real System
```bash
sudo ./antidetect --reset
```

## Technical Details

### System-Level Manipulation
- Reads from `/proc`, `/sys/class/dmi/id`, `/etc`
- Uses `uname()` for kernel information
- Direct file system manipulation
- Network interface control via `ioctl()`

### Fingerprint Generation
- Cryptographically secure random number generation
- djb2 and MurmurHash for fingerprinting
- Real browser profile database
- Canvas rendering simulation

### Security Features
- Timing attack prevention
- Random timing delays
- No external dependencies
- Thread-safe implementation

## Requirements

- C++17 compatible compiler (g++ 9+ or clang 10+)
- Linux kernel 4.x+
- Root privileges for system modifications

## Disclaimer

This software is for **educational and security research purposes only**. Unauthorized use of this software for fraudulent activities, account takeover, or any illegal purposes is strictly prohibited. The developers are not responsible for any misuse of this software.

## License

MIT License - See LICENSE file for details