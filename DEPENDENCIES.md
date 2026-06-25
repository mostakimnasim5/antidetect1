# VirtualPhonePro - Dependencies

## Required Packages

### Ubuntu/Debian
```bash
sudo apt install build-essential cmake libssl-dev libcurl4-openssl-dev git
```

### Fedora/RHEL/CentOS
```bash
sudo dnf install gcc-c++ cmake openssl-devel libcurl-devel git
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake openssl curl git
```

### Windows (vcpkg)
```powershell
# Install vcpkg first
# Then:
vcpkg install openssl:x64-windows curl:x64-windows
```

### macOS
```bash
brew install cmake openssl curl
```

---

## Package Details

### 1. OpenSSL
- **Purpose**: Cryptography (SHA256, AES, RSA, HMAC)
- **Package**: `libssl-dev` (Ubuntu) / `openssl-devel` (Fedora) / `brew install openssl` (macOS)
- **Version**: 1.1.0 or higher

### 2. libcurl
- **Purpose**: HTTP requests, GeoIP detection, proxy testing
- **Package**: `libcurl4-openssl-dev` (Ubuntu) / `libcurl-devel` (Fedora) / `brew install curl` (macOS)
- **Version**: 7.68.0 or higher

### 3. CMake
- **Purpose**: Build system
- **Package**: `cmake` (all platforms)
- **Version**: 3.20 or higher

### 4. Compiler
- **Purpose**: C++ compilation
- **Ubuntu**: `build-essential` (includes gcc/g++)
- **Windows**: Visual Studio 2019/2022
- **macOS**: Xcode command line tools

---

## Verify Installation

```bash
# Check OpenSSL
openssl version

# Check CMake
cmake --version

# Check libcurl
curl --version

# Check GCC
g++ --version
```

---

## Docker Build (Optional)

```dockerfile
FROM ubuntu:22.04
RUN apt update && apt install -y build-essential cmake libssl-dev libcurl4-openssl-dev git
WORKDIR /app
COPY . .
RUN mkdir -p build && cd build && cmake .. && cmake --build .
CMD ["./VirtualPhonePro_CLI"]
```
