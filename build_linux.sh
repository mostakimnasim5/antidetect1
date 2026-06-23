#!/bin/bash
# ============================================================
# AntiDetectPro v1.7 - Linux Build Script
# ============================================================
#
# Build script for Linux/macOS
# Requirements:
#   - GCC 10+ or Clang 12+
#   - CMake 3.20+
#   - OpenSSL development files
#
# Usage:
#   ./build_linux.sh          # Standard build
#   ./build_linux.sh clean    # Clean and rebuild
#   ./build_linux.sh debug    # Debug build
#
# ============================================================

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}============================================================${NC}"
echo -e "${CYAN}  AntiDetectPro v1.7 - Linux Build Script${NC}"
echo -e "${CYAN}============================================================${NC}"
echo ""

# Parse arguments
CLEAN=false
BUILD_TYPE="Release"
VERBOSE=false

for arg in "$@"; do
    case $arg in
        clean)
            CLEAN=true
            ;;
        debug)
            BUILD_TYPE="Debug"
            ;;
        verbose)
            VERBOSE=true
            ;;
        help|-h|--help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  clean    - Clean previous build"
            echo "  debug    - Build with Debug configuration"
            echo "  verbose  - Show detailed output"
            echo "  help     - Show this help message"
            exit 0
            ;;
    esac
done

# Check for required tools
echo -e "${YELLOW}[1/5] Checking dependencies...${NC}"

check_command() {
    if command -v "$1" &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} $1: $(command -v $1)"
    else
        echo -e "  ${RED}✗${NC} $1: NOT FOUND"
        return 1
    fi
}

MISSING_DEPS=0

check_command cmake || MISSING_DEPS=1
check_command g++ || check_command clang++ || MISSING_DEPS=1
check_command pkg-config || echo -e "  ${YELLOW}!${NC} pkg-config: optional"

# Check OpenSSL
if pkg-config --exists openssl 2>/dev/null; then
    echo -e "  ${GREEN}✓${NC} OpenSSL: $(pkg-config --modversion openssl)"
elif [ -f /usr/include/openssl/ssl.h ]; then
    echo -e "  ${GREEN}✓${NC} OpenSSL: system headers found"
else
    echo -e "  ${RED}✗${NC} OpenSSL: development files NOT FOUND"
    MISSING_DEPS=1
fi

if [ $MISSING_DEPS -eq 1 ]; then
    echo ""
    echo -e "${RED}[ERROR] Missing dependencies!${NC}"
    echo ""
    echo "Install dependencies:"
    echo ""
    echo "  Ubuntu/Debian:"
    echo "    sudo apt-get install build-essential cmake libssl-dev"
    echo ""
    echo "  Fedora/RHEL:"
    echo "    sudo dnf groupinstall 'Development Tools'"
    echo "    sudo dnf install cmake openssl-devel"
    echo ""
    echo "  Arch Linux:"
    echo "    sudo pacman -S base-devel cmake openssl"
    echo ""
    echo "  macOS:"
    echo "    brew install cmake openssl"
    echo ""
    exit 1
fi

# Clean previous build
if [ "$CLEAN" = true ] && [ -d "$BUILD_DIR" ]; then
    echo ""
    echo -e "${YELLOW}[2/5] Cleaning previous build...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure CMake
echo ""
echo -e "${YELLOW}[3/5] Configuring CMake...${NC}"

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

# Add verbose flag if requested
if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

cmake $CMAKE_ARGS ..

if [ $? -ne 0 ]; then
    echo ""
    echo -e "${RED}[ERROR] CMake configuration failed!${NC}"
    exit 1
fi

# Build
echo ""
echo -e "${YELLOW}[4/5] Building AntiDetectPro...${NC}"
echo ""

# Get number of CPU cores for parallel build
if command -v nproc &> /dev/null; then
    JOBS=$(nproc)
elif command -v sysctl &> /dev/null; then
    JOBS=$(sysctl -n hw.ncpu)
else
    JOBS=4
fi

echo "  Using $JOBS parallel jobs"

cmake --build . --config $BUILD_TYPE -j $JOBS

if [ $? -ne 0 ]; then
    echo ""
    echo -e "${RED}[ERROR] Build failed!${NC}"
    exit 1
fi

# Find executable
echo ""
echo -e "${YELLOW}[5/5] Locating executable...${NC}"

EXE_PATH=$(find . -name "AntiDetectPro" -type f -executable 2>/dev/null | head -1)

if [ -n "$EXE_PATH" ]; then
    EXE_SIZE=$(stat -f%z "$EXE_PATH" 2>/dev/null || stat -c%s "$EXE_PATH" 2>/dev/null || echo "unknown")
    if [ "$EXE_SIZE" != "unknown" ]; then
        EXE_SIZE_MB=$(echo "scale=2; $EXE_SIZE / 1024 / 1024" | bc 2>/dev/null || echo "~")
    else
        EXE_SIZE_MB="~"
    fi
    
    echo ""
    echo -e "${GREEN}============================================================${NC}"
    echo -e "${GREEN}  [SUCCESS] AntiDetectPro v1.7 Build Completed!${NC}"
    echo -e "${GREEN}============================================================${NC}"
    echo ""
    echo -e "  ${CYAN}Executable:${NC} $EXE_PATH"
    echo -e "  ${CYAN}Size:${NC} ${EXE_SIZE_MB} MB"
    echo ""
    echo -e "  ${CYAN}Usage:${NC}"
    echo -e "    $EXE_PATH --help"
    echo -e "    $EXE_PATH list-devices"
    echo -e "    $EXE_PATH enable-sensor-spoofing"
    echo ""
    echo -e "${GREEN}============================================================${NC}"
    
    # Copy to bin directory
    mkdir -p "$PROJECT_DIR/bin"
    cp "$EXE_PATH" "$PROJECT_DIR/bin/"
    echo -e "  ${CYAN}Copied to:${NC} $PROJECT_DIR/bin/AntiDetectPro"
else
    echo ""
    echo -e "${YELLOW}[WARNING] Executable not found in standard location${NC}"
    echo "Searching in build directory..."
    find . -type f -name "AntiDetectPro*" 2>/dev/null | head -5
fi

echo ""
echo "Build log saved to: $BUILD_DIR/CMakeFiles/CMakeOutput.log"
echo ""
