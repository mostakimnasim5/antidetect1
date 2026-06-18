# Antidetect Browser Testing Tool

A C++ implementation for testing browser fingerprint spoofing techniques.

## Features

- **Browser Fingerprint Generation**
  - User Agent spoofing
  - Platform detection
  - Language spoofing
  - Screen resolution spoofing
  - Timezone spoofing
  - Canvas fingerprint generation
  - WebGL fingerprint spoofing

- **System Information**
  - Hostname spoofing
  - MAC address generation
  - OS information detection

- **Profile Management**
  - Save fingerprints to file
  - Load fingerprints from file
  - Generate multiple unique fingerprints

## Build

```bash
g++ -std=c++17 -O2 -Wall -o antidetect main.cpp antidetect.cpp
```

## Usage

### Command Line Mode

```bash
# Generate a random fingerprint
./antidetect --generate

# Save profile to file
./antidetect --save profile.txt

# Load profile from file
./antidetect --load profile.txt

# Generate multiple fingerprints
./antidetect --multi 5
```

### Interactive Mode

```bash
./antidetect
```

Options:
1. Generate New Fingerprint
2. Display Current Fingerprint
3. Save Profile to File
4. Load Profile from File
5. Generate Multiple Fingerprints
6. Exit

## Files

- `antidetect.h` - Header file with class definitions
- `antidetect.cpp` - Implementation of fingerprint generation
- `main.cpp` - Main program with CLI interface
- `CMakeLists.txt` - CMake build configuration

## Purpose

This tool is for **educational and testing purposes only**. It demonstrates how browser fingerprinting works and how to generate randomized fingerprints for testing anti-detection systems.

**Disclaimer**: This software should not be used for malicious purposes, fraud, or any illegal activities.