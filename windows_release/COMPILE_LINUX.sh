#!/bin/bash

echo "================================================"
echo "   OS-Level Anti-Detect System - Linux Build"
echo "================================================"
echo ""

# Check if g++ is installed
if ! command -v g++ &> /dev/null; then
    echo "ERROR: g++ not found!"
    echo "Please install: sudo apt install build-essential"
    exit 1
fi

echo "[*] Compiling Anti-Detect System..."
echo ""

# Compile
g++ -std=c++17 -O2 -pthread \
    main.cpp \
    system_fingerprint.cpp \
    advanced_fingerprint.cpp \
    android_spoof.cpp \
    -o antidetect

if [ $? -eq 0 ]; then
    echo ""
    echo "================================================"
    echo "   BUILD SUCCESSFUL!"
    echo "================================================"
    echo ""
    echo "antidetect has been created!"
    echo ""
    echo "Usage:"
    echo "  ./antidetect --info        : Show system info"
    echo "  ./antidetect --list        : List presets"
    echo "  ./antidetect --generate    : Generate profile"
    echo "  ./antidetect --help        : Show help"
    echo ""
    echo "Root commands (require sudo):"
    echo "  sudo ./antidetect --preset samsung_s24_ultra"
    echo "  sudo ./antidetect --reset"
    echo ""
    chmod +x antidetect
else
    echo ""
    echo "================================================"
    echo "   BUILD FAILED!"
    echo "================================================"
    exit 1
fi
