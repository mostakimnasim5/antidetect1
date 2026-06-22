# AntiDetectPro - Build Guide

## How to Build .exe for Windows

এই গাইড অনুসরণ করে আপনি AntiDetectPro-এর Windows executable (.exe) বানাতে পারবেন।

---

## 📋 Prerequisites (আগে ইনস্টল করুন)

### ১. Qt Framework ইনস্টল করুন

**Option A: Qt Online Installer (সুপারিশকৃত)**
1. https://www.qt.io/download-qt-installer যান
2. Free account তৈরি করুন
3. Qt 6.5 বা 6.6 ডাউনলোড করুন
4. Components select করুন:
   - Qt 6.5.x → MSVC 2022 64-bit
   - Qt Creator
   - Qt Charts (for graphs)
   - Qt Network Auth
   - Qt SVG

**Option B: Qt Creator with MinGW**
1. https://github.com/nitrux/nxqt/releases থেকে portable Qt ডাউনলোড করুন

### ২. CMake ইনস্টল করুন

```bash
# Windows-এ Chocolatey দিয়ে:
choco install cmake -y

# অথবা https://cmake.org/download থেকে
```

### ৩. OpenSSL ইনস্টল করুন

```bash
# Chocolatey দিয়ে:
choco install openssl -y
```

### ৪. ADB (Android Debug Bridge) ইনস্টল করুন

```bash
# Chocolatey দিয়ে:
choco install adb -y

# অথবা Android Studio ইনস্টল করুন
```

### ৫. scrcpy ইনস্টল করুন (Screen Mirror-এর জন্য)

```bash
choco install scrcpy -y
```

---

## 🔨 Build Steps (Windows-এ)

### Step 1: Repository Clone করুন

```bash
git clone https://github.com/mostakimnasim5/antidetect1.git
cd antidetect1
```

### Step 2: Qt Creator-এ Open করুন

```bash
# Qt Creator খুলুন
# File → Open File or Project
# antidetect1/CMakeLists.txt সিলেক্ট করুন
```

### Step 3: Configure করুন

Qt Creator-এ:
1. **Configure Project** ক্লিক করুন
2. **Kits** থেকে MSVC 2022 বা MinGW select করুন
3. **Run CMake** ক্লিক করুন

### Step 4: Build করুন

```bash
# অথবা Terminal-এ:
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Step 5: .exe পাবেন

```
build/bin/AntiDetectPro.exe
```

---

## 🐛 Debug Build (Optional)

Debug version বানতে চাইলে:

```bash
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

Output: `build/bin/Debug/AntiDetectPro.exe`

---

## 📦 Standalone .exe তৈরি করা (অতিরিক্ত)

### Qt DLLs সংগ্রহ করুন

আপনার exe standalone হলে DLLs প্রয়োজন:

```bash
# Qt-এর bin folder থেকে copy করুন:
# C:\Qt\6.x.x\6.x.x\msvc2022_64\bin\ -এ গিয়ে

copy Qt6Core.dll
copy Qt6Gui.dll
copy Qt6Widgets.dll
copy Qt6Network.dll
copy Qt6Svg.dll
copy platforms/qwindows.dll
copy Qt6Graphs.dll (যদি থাকে)
```

### windeployqt ব্যবহার করুন

```bash
# Qt 6-তে:
C:\Qt\6.5.0\6.5.0\msvc2022_64\bin\windeployqt.exe build\bin\AntiDetectPro.exe
```

---

## 🔧 Alternative: Command Line Build

### Visual Studio Developer Command Prompt-এ:

```bash
cd antidetect1
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Output location:
```
build\bin\Release\AntiDetectPro.exe
```

### MinGW দিয়ে:

```bash
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j4
```

---

## ⚠️ সমস্যা সমাধান

### "Qt6 not found" Error:
```
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\6.5.0\msvc2022_64\lib\cmake
```

### "OpenSSL not found" Error:
```
set OPENSSL_ROOT_DIR=C:\OpenSSL-Win64
```

### "MSVC compiler not found":
Qt Creator-এ Tools → Options → Kits → Compilers-এ MSVC add করুন

### GUI Build করতে চাইলে:
CMakeLists.txt-এ:
```cmake
option(BUILD_GUI "Build Qt6 GUI application" ON)
```

তারপর:
```bash
cmake .. -DBUILD_GUI=ON
```

---

## 📁 Final Output

সফলভাবে বিল্ড হলে পাবেন:

```
antidetect1/
└── build/
    └── bin/
        └── AntiDetectPro.exe    ← মূল executable
        └── AntiDetectPro.pdb    ← Debug symbols (optional)
```

---

## 🎯 Quick Build Script (PowerShell)

এই script save করুন `build.ps1` নামে:

```powershell
# AntiDetectPro Build Script
Write-Host "Building AntiDetectPro..." -ForegroundColor Cyan

# Clone if not exists
if (!(Test-Path "antidetect1")) {
    git clone https://github.com/mostakimnasim5/antidetect1.git
}

cd antidetect1

# Create build directory
if (Test-Path "build") { Remove-Item -Recurse -Force build }
mkdir build
cd build

# Configure
Write-Host "Configuring..." -ForegroundColor Yellow
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build
Write-Host "Building..." -ForegroundColor Yellow
cmake --build . --config Release

# Done
Write-Host "`nBuild Complete!" -ForegroundColor Green
Write-Host "Output: $PWD\bin\Release\AntiDetectPro.exe"
Write-Host "`nPress any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
```

Run করুন:
```powershell
powershell -ExecutionPolicy Bypass -File build.ps1
```

---

## 📞 সাহায্য প্রয়োজন?

যদি কোনো সমস্যা হয়, আমাকে জানান আমি সাহায্য করব! 🚀
