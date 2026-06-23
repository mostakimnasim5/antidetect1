# AntiDetectPro v1.7 - Windows Build Guide

## Quick Start (5 Minutes)

### Option 1: CMake Build (Recommended)

```batch
# 1. Open Command Prompt
# 2. Navigate to project directory
cd AntiDetectPro

# 3. Run CMake build
build_cmake.bat

# 4. Find exe at: build/Release/AntiDetectPro.exe
```

### Option 2: MSBuild

```batch
# 1. Open Developer Command Prompt for VS2022
# 2. Navigate to project
cd AntiDetectPro

# 3. Run build
build_windows.bat

# 4. Find exe at: build/AntiDetectPro.exe
```

### Option 3: PowerShell

```powershell
# 1. Open PowerShell as Administrator
# 2. Navigate to project
cd AntiDetectPro

# 3. Run build
.\build.ps1

# 4. Find exe at: build/AntiDetectPro.exe
```

---

## Requirements

### Minimum
- Windows 10/11 (64-bit)
- Visual Studio 2019 or 2022
- CMake 3.20+
- 4GB RAM
- 2GB Disk Space

### Optional
- ADB (Android Debug Bridge) - for device connection
- Git - for version control

---

## Build Output

```
AntiDetectPro/
├── build/
│   ├── AntiDetectPro.exe          # Main executable
│   ├── AntiDetectPro.pdb          # Debug symbols
│   └── (dependencies)/
├── profiles/                       # Profile storage
├── logs/                          # Log files
└── config/                        # Configuration
```

---

## Features

### What's Included

| Module | Description |
|--------|-------------|
| RealisticProfileGenerator | Generate 100% unique device profiles |
| HardwareFingerprintSpoofer | Spoof CPU, GPU, DMI info |
| NetworkStackSpoofer | Spoof TTL, MAC, DNS |
| SafetyNetAdvancedBypass | Bypass SafetyNet/Play Integrity |
| SensorSpoofer | Natural sensor noise simulation |
| HypervisorBypass | Hide VM/emulator detection |
| TimingAttackPrevention | CPU timing protection |

### Profile Statistics

| Metric | Value |
|--------|-------|
| Max Profiles | **∞ (Unlimited)** |
| Device Database | 34 real devices |
| Regions | 8 geographic regions |
| Carriers | 37+ operators |
| Uniqueness | 100% |
| Realism | 95% |

---

## Troubleshooting

### "Visual Studio not found"

1. Install Visual Studio 2022 from: https://visualstudio.microsoft.com/
2. Select "Desktop development with C++" workload
3. Restart command prompt

### "CMake not found"

1. Download CMake from: https://cmake.org/download/
2. Add to PATH during installation
3. Restart command prompt

### Build fails with errors

```batch
# Clean and rebuild
cd build
del /f /s /q *
cd ..
build_cmake.bat
```

---

## Usage

### Generate a Profile

```batch
# Connect Android device via ADB first
adb devices

# Run application
build\AntiDetectPro.exe

# Or in interactive mode
build\AntiDetectPro.exe --interactive
```

### Available Commands

```batch
# Generate Samsung profile
generate samsung

# Generate Google profile  
generate google

# Generate unique profile
generate unique

# Apply profile to device
apply <profile_id>

# Export as JSON
export json

# Show system status
status
```

---

## Support

For issues: https://github.com/mostakimnasim5/antidetect1/issues

---

**AntiDetectPro v1.7** - Enterprise Anti-Detection System
