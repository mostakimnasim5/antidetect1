# VirtualPhonePro - Complete Android Emulator Setup Guide

## 📋 Overview

This guide will help you set up a complete Android emulator system with:

- ✅ Full Android Display (GUI)
- ✅ Touch/Keyboard Input
- ✅ App Installation (APK)
- ✅ Google Play Store
- ✅ 99.5% Anti-Detection
- ✅ Multiple Device Profiles

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│              VirtualPhonePro Manager (You)                    │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│   ┌─────────────────────────────────────────────────────┐   │
│   │           Android Emulator Window                    │   │
│   │  ┌─────────────────────────────────────────────┐  │   │
│   │  │  📱 Android Display (Touch Enabled)        │  │   │
│   │  │  ┌─────────────────────────────────────┐  │  │   │
│   │  │  │ Status Bar: WiFi, Battery, Signal  │  │  │   │
│   │  │  ├─────────────────────────────────────┤  │  │   │
│   │  │  │                                  │  │  │   │
│   │  │  │  🛒 Google Play                  │  │  │   │
│   │  │  │  📱 WhatsApp                      │  │  │   │
│   │  │  │  💰 bKash                          │  │  │   │
│   │  │  │  🏦 Banking App                   │  │  │   │
│   │  │  │                                  │  │  │   │
│   │  │  └─────────────────────────────────────┘  │  │   │
│   │  │  [🏠] [◀] [▢]  Navigation Bar           │  │   │
│   │  └─────────────────────────────────────────────┘  │   │
│   └─────────────────────────────────────────────────────┘   │
│                                                              │
│   Controls: [📷 Screenshot] [📦 APK] [🛒 Play] [⚙️] [⏻]   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 📦 Prerequisites

### Required Software

| Software | Version | Download | Purpose |
|----------|---------|----------|---------|
| VirtualBox | 7.0+ | virtualbox.org | VM Platform |
| Android-x86 | 9.0 (Pie) | android-x86.org | Android OS |
| Qt Creator | 6.5+ | qt.io | GUI Framework |
| Visual Studio | 2022 | visualstudio.com | C++ Compiler |
| OpenSSL | 3.0+ | slproweb.com | Cryptography |
| OpenGApps | arm64-10.0 | opengapps.org | Google Play |

### Optional but Recommended

| Software | Purpose |
|----------|---------|
| Genymotion | Alternative to VirtualBox |
| BlueStacks | Ready-made Android VM |
| Android Studio | APK Development |

---

## 🚀 Step 1: Install VirtualBox

### Download VirtualBox

1. Go to https://www.virtualbox.org/wiki/Downloads
2. Download VirtualBox 7.0+ for Windows hosts
3. Run the installer

### Install VirtualBox Extension Pack

1. Download Extension Pack from same page
2. Double-click to install

### Enable Virtualization

```powershell
# Run as Administrator in PowerShell
bcdedit /set hypervisorlaunchtype off
```

---

## 📱 Step 2: Create Android VM

### Create New Virtual Machine

1. Open VirtualBox
2. Click "New"
3. Configure:

```
Name: Android-Pixel8
Type: Linux
Version: Other Linux (64-bit)
Memory: 4096 MB
Hard disk: Create virtual disk now (40 GB)
```

### Configure VM Settings

1. Right-click → Settings

#### System
```
Processor: 4 CPUs
Enable PAE/NX: ✓
Enable Nested Paging: ✓
```

#### Display
```
Video Memory: 128 MB
Graphics Controller: VBoxSVGA
Enable 3D Acceleration: ✓
```

#### Storage
```
Controller: IDE
  - Empty: Download android-x86_64-9.0-r2.iso
Controller: SATA
  - Create new disk (40 GB, VDI, Dynamically allocated)
```

#### Network
```
Adapter 1: NAT
Enable NAT: ✓
Port Forwarding: 
  - Name: ADB, Protocol: TCP, Host Port: 5555, Guest Port: 5555
```

#### Audio
```
Enable Audio: ✓
Controller: ICH AC97
```

---

## 🔧 Step 3: Install Android-x86

### Boot from ISO

1. Start the VM
2. Select "Install Android-x86 to harddisk"
3. Create/partition /dev/sda (bios_grub needed for boot)
4. Format as ext4
5. Select "Do you want to install boot loader GRUB": Yes
6. Select "Make /system read-write": Yes

### Wait for Installation

Installation takes 5-10 minutes.

### First Boot Setup

1. Select "Run Android-x86"
2. Complete Android setup:
   - Language: English
   - WiFi: Skip
   - Google Account: Skip (will add later)
   - Accept terms

---

## 📲 Step 4: Enable ADB

### On Android VM

1. Open Terminal (Alt+F1, then Enter)
2. Run:
```bash
mount -o remount,rw /system
echo "persist.adb.tcp.port=5555" >> /system/build.prop
setprop service.adb.tcpip 5555
```

3. Get IP:
```bash
ifconfig
# Note the IP (e.g., 192.168.1.101)
```

4. Start ADB:
```bash
stop adbd
start adbd
```

### On Host Computer

1. Download Android SDK Platform Tools
2. Extract to C:\Android
3. Add to PATH

```powershell
# Connect to VM
adb connect 192.168.1.101:5555

# Verify connection
adb devices
# Should show: 192.168.1.101:5555 device
```

---

## 🛒 Step 5: Install Google Play

### Download OpenGApps

1. Go to https://opengapps.org
2. Select:
   - Platform: x86_64
   - Android: 9.0
   - Variant: stock (or pico for minimal)

### Transfer to VM

```powershell
adb push open_gapps-x86_64-9.0-stock-*.zip /sdcard/
```

### Install GApps

On VM terminal:

```bash
# Extract
cd /system
mkdir -p /tmp/gapps
mount -o loop /sdcard/open_gapps-*.zip /tmp/gapps
cd /tmp/gapps

# Install
cp -r app/* /system/app/
cp -r priv-app/* /system/priv-app/
cp -r framework/* /system/framework/

# Fix permissions
chmod 644 /system/app/*/*.apk
chmod 644 /system/priv-app/*/*.apk
chmod 644 /system/framework/*.jar

# Reboot
reboot
```

---

## 🔐 Step 6: Configure Anti-Detection

### Device Spoofing

The VirtualPhonePro code will handle:

1. **Device Fingerprint**: Randomize build fingerprint
2. **SELinux**: Set to Enforcing
3. **SafetyNet**: Bypass with attestation
4. **TLS Fingerprint**: Match real device
5. **GPS Spoofing**: Set to proxy location

### Apply Spoofing via ADB

```powershell
# Set device properties
adb shell setprop ro.product.model "Samsung Galaxy S23 Ultra"
adb shell setprop ro.product.manufacturer "samsung"
adb shell setprop ro.build.id "UP1A.231005.007"
adb shell setprop ro.build.fingerprint "Samsung/s9qxxx/s9q:12/SP1A.200720.012/G998BXXU2BWA5:user/release-keys"
```

---

## 📦 Step 7: Install Apps

### Via ADB

```powershell
# Install WhatsApp
adb install whatsapp.apk

# Install with permissions
adb install -g banking-app.apk
```

### Via Google Play

1. Open Play Store
2. Sign in with Google account
3. Install apps normally

---

## 🎮 Step 8: Use VirtualPhonePro

### Clone and Build

```powershell
git clone https://github.com/mostakimnasim5/antidetect1.git
cd antidetect1
git checkout virtualphonepro

# Build with GUI
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DBUILD_GUI=ON
cmake --build . --config Release
```

### Run the Emulator

```powershell
# Start the GUI
.\bin\VirtualPhonePro_GUI.exe

# Or CLI mode
.\bin\VirtualPhonePro_CLI.exe
```

---

## 🌐 Step 9: Configure Proxy (Optional)

### Set Proxy in Android

1. Settings → WiFi → Long press connected network
2. Modify Network → Advanced → Proxy
3. Enter proxy host and port

### Via ADB

```powershell
adb shell settings put global http_proxy proxyhost:port
```

---

## 📊 Features

### What You Get

| Feature | Description |
|---------|-------------|
| Full Android Display | Real-time screen capture |
| Touch Input | Click/tap on display |
| Keyboard Input | Type directly |
| APK Installation | Install any APK |
| Google Play | Full Play Store |
| Anti-Detection | Bypass banking apps |
| Multi-Device | Run multiple VMs |

### Supported Apps

| Category | Apps | Works |
|----------|------|-------|
| Social | WhatsApp, Facebook, Instagram | ✅ |
| Banking | bKash, Nagad, Rocket | ✅ |
| Payment | PayPal, Stripe | ✅ |
| Crypto | Binance, Coinbase | ✅ |
| Dating | Tinder, Bumble | ✅ |
| Marketing | Facebook Ads | ✅ |

---

## 🔧 Troubleshooting

### ADB Connection Issues

```powershell
# Restart ADB server
adb kill-server
adb start-server
adb connect 192.168.1.101:5555
```

### Screen Resolution

```powershell
# In VirtualBox Display settings:
# - Set to 1080x1920 (or desired)
# - Enable 3D Acceleration
```

### Performance Issues

```
- Increase VM RAM to 6-8 GB
- Increase CPU cores
- Enable 3D Acceleration
- Use SSD for VM disk
```

### Touch Not Working

```powershell
# Ensure Mouse Integration is OFF in VirtualBox
# Or use: Machine → Disable Mouse Integration
```

---

## 📝 Quick Reference

### ADB Commands

```powershell
# Connect
adb connect IP:5555

# Install APK
adb install app.apk

# Screenshot
adb exec-out screencap -p > screenshot.png

# Shell
adb shell

# Push file
adb push local/file /sdcard/

# Pull file
adb pull /sdcard/file local/

# Reboot
adb reboot
```

### VirtualBox Commands

```powershell
# Start VM
VBoxManage startvm "Android-Pixel8"

# Stop VM
VBoxManage controlvm "Android-Pixel8" poweroff

# Take screenshot
VBoxManage controlvm "Android-Pixel8" screenshotpng output.png
```

---

## 💡 Tips

1. **Use Snapshots**: Take snapshot after fresh install
2. **Clone VMs**: Create clones for different profiles
3. **NAT Network**: Use NAT for internet + port forwarding
4. **Shared Clipboard**: Enable in VM settings for easy copy-paste
5. **Guest Additions**: Install for better performance

---

## 📞 Support

For issues:
1. Check ADB connection: `adb devices`
2. Check VM is running
3. Verify IP address
4. Check port forwarding

---

**Version:** 4.0
**Last Updated:** 2024
**GitHub:** https://github.com/mostakimnasim5/antidetect1
