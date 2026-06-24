# AntiDetectPro v1.8 - System Dependencies & Build Guide

## 📋 Dependency Summary

| Package | Version | Required | Purpose | Package Name |
|---------|---------|----------|---------|--------------|
| **CMake** | 3.20+ | ✅ Yes | Build system | `cmake` |
| **C++ Compiler** | C++20 | ✅ Yes | Compilation | MSVC 2019/2022, GCC 10+, Clang 12+ |
| **OpenSSL** | 1.1.x / 3.x | ✅ Yes | SSL/TLS, Crypto | `libssl-dev` (Linux), `openssl` (Windows) |
| **Threads** | POSIX/Win32 | ✅ Yes | Threading | Built-in |
| **ADB** | 1.0.x+ | ⚠️ Optional | Device connection | `adb` |
| **Qt5/Qt6** | 5.15+ / 6.x | ❌ Optional | GUI (disabled by default) | `qt5`, `qt6` |

---

## 🪟 Windows (Visual Studio)

### Required Installation

1. **Visual Studio 2022** (Recommended)
   - Download: https://visualstudio.microsoft.com/downloads/
   - Select workload: **"Desktop development with C++"**
   - Individual components needed:
     - MSVC v143 - VS 2022 C++ x64/x86 build tools
     - Windows 11 SDK (or Windows 10 SDK)
     - C++ ATL for latest builds

2. **CMake 3.20+**
   - Download: https://cmake.org/download/
   - During install: ✅ Add CMake to system PATH

3. **OpenSSL**
   - **Option A (Recommended)**: vcpkg
     ```powershell
     # Install vcpkg first
     git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
     cd C:\vcpkg
     .\bootstrap-vcpkg.bat
     
     # Install OpenSSL
     vcpkg install openssl:x64-windows
     ```
   
   - **Option B**: Pre-built binaries
     - Download: https://slproweb.com/products/Win32OpenSSL.html
     - Install to: `C:\OpenSSL`

### Build Commands

```powershell
# Open "Developer Command Prompt for VS 2022"
cd C:\path\to\AntiDetectPro

# Run build script
build_windows.bat

# OR manual build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel

# Find executable at: build\AntiDetectPro.exe
```

---

## 🐧 Linux (Ubuntu/Debian)

### Required Packages

```bash
# Core build tools
sudo apt-get update
sudo apt-get install -y build-essential cmake

# OpenSSL development
sudo apt-get install -y libssl-dev

# Optional: Qt5 for GUI
sudo apt-get install -y qtbase5-dev qtbase5-dev-tools

# Optional: ADB
sudo apt-get install -y adb
```

### Build Commands

```bash
# Clone/Fetch project
cd AntiDetectPro

# Run build script
chmod +x build_linux.sh
./build_linux.sh

# OR manual build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Find executable at: build/AntiDetectPro
```

---

## 🍎 macOS

### Required Packages

```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake openssl

# Optional: Qt6
brew install qt6
```

### Build Commands

```bash
# Set OpenSSL path
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Find executable at: build/AntiDetectPro
```

---

## ⚙️ CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type: Debug/Release |
| `BUILD_GUI` | OFF | Build Qt GUI (requires Qt5/Qt6) |
| `OPENSSL_ROOT_DIR` | Auto-detect | OpenSSL installation path |

### Example: Build with GUI

```bash
# Linux
cmake .. -DBUILD_GUI=ON -DCMAKE_BUILD_TYPE=Release

# Windows
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_GUI=ON -DCMAKE_BUILD_TYPE=Release
```

---

## 🔍 Troubleshooting

### "Visual Studio not found"

1. Install Visual Studio 2022 with C++ workload
2. Restart terminal after installation
3. Verify: `where cl` in Developer Command Prompt

### "CMake not found"

1. Download CMake from https://cmake.org/download/
2. Run installer, select "Add to system PATH"
3. Restart terminal

### "OpenSSL not found"

**Windows:**
```powershell
# Install via vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
vcpkg install openssl:x64-windows

# Then build
cmake .. -DOPENSSL_ROOT_DIR=C:\vcpkg\installed\x64-windows
```

**Linux:**
```bash
sudo apt-get install libssl-dev
```

### "Build fails with errors"

```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
cmake --build . --parallel 2>&1 | head -100
```

---

## 📁 Project Structure

```
AntiDetectPro/
├── src/                    # C++ source files (30 files)
├── include/                # Header files (34 files)
├── gui/                    # Qt GUI files (optional)
├── build/                  # Build output
│   └── AntiDetectPro       # Executable
├── CMakeLists.txt          # CMake configuration
├── build_windows.bat       # Windows build script
├── build_linux.sh          # Linux build script
└── DEPENDENCIES.md         # This file
```

---

## ✅ Build Verification

After successful build, you should see:

```
============================================================
[SUCCESS] AntiDetectPro v1.8 Build Completed!
============================================================

Executable: build/AntiDetectPro.exe (Windows)
            build/AntiDetectPro (Linux/macOS)

To run:
  ./AntiDetectPro --help
  ./AntiDetectPro list-devices
```

---

## 📦 Package Names by Distribution

| Distribution | CMake | OpenSSL | Qt5 | ADB |
|-------------|-------|---------|-----|-----|
| Ubuntu/Debian | `cmake` | `libssl-dev` | `qtbase5-dev` | `adb` |
| Fedora/RHEL | `cmake` | `openssl-devel` | `qt5-qtbase` | `android-tools` |
| Arch Linux | `cmake` | `openssl` | `qt5-base` | `android-tools` |
| openSUSE | `cmake` | `libopenssl-devel` | `libqt5-qtbase` | `android-tools` |

---

**AntiDetectPro v1.8** - Enterprise Anti-Detection System
