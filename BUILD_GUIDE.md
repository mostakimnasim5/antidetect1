# VirtualPhonePro - Build Guide

## System Requirements

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    libssl-dev \
    libcurl4-openssl-dev \
    git

# Optional: For GUI (Qt)
# sudo apt install -y qtbase5-dev qt5-qmake cmake
```

### Windows
```
1. Install Visual Studio 2019/2022 with C++ tools
2. Install CMake (cmake.org)
3. Install OpenSSL (vcpkg or prebuilt)
4. Install libcurl (vcpkg or prebuilt)
```

### macOS
```bash
# Install Homebrew first
brew install cmake openssl curl
```

---

## Build Instructions

### Linux/macOS

```bash
# 1. Go to project directory
cd VirtualPhonePro

# 2. Create build directory
mkdir -p build && cd build

# 3. Run CMake
cmake ..

# 4. Build
cmake --build . -j$(nproc)

# 5. Run
./VirtualPhonePro_CLI
```

### Windows (Visual Studio)

```powershell
# 1. Open Developer Command Prompt
# 2. Go to project directory
cd VirtualPhonePro

# 3. Create build directory
mkdir build && cd build

# 4. Run CMake
cmake .. -G "Visual Studio 17 2022"

# 5. Build
cmake --build . --config Release

# 6. Run
./Release/VirtualPhonePro_CLI.exe
```

---

## Dependencies

| Package | Version | Required | Purpose |
|---------|---------|----------|---------|
| OpenSSL | 1.1+ | ✅ Yes | Cryptography |
| libcurl | 7.68+ | ✅ Yes | HTTP requests, GeoIP |
| Threads | - | ✅ Yes | POSIX threads |
| Qt5/Qt6 | 5.15+ | ❌ No | GUI (optional) |

---

## Troubleshooting

### Error: "OpenSSL not found"
```bash
# Ubuntu/Debian
sudo apt install libssl-dev

# macOS
brew install openssl
```

### Error: "libcurl not found"
```bash
# Ubuntu/Debian
sudo apt install libcurl4-openssl-dev

# macOS
brew install curl
```

### Error: "CMake version too old"
```bash
# Ubuntu/Debian
sudo apt install cmake
# or download from cmake.org
```

---

## Quick Build (Single Command)

### Linux:
```bash
mkdir -p build && cd build && cmake .. && cmake --build . -j$(nproc) && ./VirtualPhonePro_CLI
```

### Windows:
```powershell
mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Release
```
