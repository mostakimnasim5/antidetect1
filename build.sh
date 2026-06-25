#!/bin/bash

# VirtualPhonePro Build Script
# ============================

set -e

echo "=========================================="
echo "  VirtualPhonePro Build Script"
echo "=========================================="
echo ""

# Check dependencies
echo "[1/4] Checking dependencies..."
MISSING=""

if ! command -v cmake &> /dev/null; then
    MISSING="$MISSING cmake"
fi

if ! command -v g++ &> /dev/null; then
    MISSING="$MISSING g++"
fi

if ! command -v make &> /dev/null; then
    MISSING="$MISSING make"
fi

if [ -n "$MISSING" ]; then
    echo "ERROR: Missing dependencies: $MISSING"
    echo ""
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt install build-essential cmake"
    echo "  Fedora: sudo dnf install gcc-c++ cmake"
    echo "  Arch: sudo pacman -S base-devel cmake"
    exit 1
fi

# Check OpenSSL
if ! pkg-config --exists openssl 2>/dev/null && ! ls /usr/include/openssl/*.h &>/dev/null && ! ls /usr/local/include/openssl/*.h &>/dev/null; then
    echo "WARNING: OpenSSL development headers not found"
    echo "Install with: sudo apt install libssl-dev"
fi

# Check libcurl
if ! pkg-config --exists libcurl 2>/dev/null && ! ls /usr/include/curl/*.h &>/dev/null && ! ls /usr/local/include/curl/*.h &>/dev/null; then
    echo "WARNING: libcurl development headers not found"
    echo "Install with: sudo apt install libcurl4-openssl-dev"
fi

echo "  All required tools found!"
echo ""

# Create build directory
echo "[2/4] Creating build directory..."
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "  Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "  Build directory ready: $BUILD_DIR"
echo ""

# Run CMake
echo "[3/4] Running CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "[4/4] Building..."
BUILD_JOBS=$(nproc 2>/dev/null || echo 4)
cmake --build . -j"$BUILD_JOBS"

# Check result
if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  BUILD SUCCESSFUL!"
    echo "=========================================="
    echo ""
    echo "  Executable: ./VirtualPhonePro_CLI"
    echo ""
    echo "  Run with: ./VirtualPhonePro_CLI"
    echo ""
else
    echo ""
    echo "=========================================="
    echo "  BUILD FAILED!"
    echo "=========================================="
    exit 1
fi
