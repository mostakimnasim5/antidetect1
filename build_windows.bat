@echo off
REM ============================================================
REM AntiDetectPro v1.7 - Windows Build Script
REM ============================================================
REM 
REM This script compiles AntiDetectPro for Windows
REM Requirements:
REM   - Visual Studio 2019/2022 with C++ tools
REM   - OpenSSL (can be installed via vcpkg or downloaded)
REM
REM Usage:
REM   1. Open "Developer Command Prompt for VS 2022"
REM   2. Navigate to project directory
REM   3. Run: build_windows.bat
REM
REM ============================================================

echo ============================================================
echo AntiDetectPro v1.7 - Windows Build
echo ============================================================
echo.

REM Check for Visual Studio
if not exist "%VSINSTALLDIR%" (
    echo [ERROR] Visual Studio not found!
    echo Please install Visual Studio 2019 or 2022 with C++ tools
    echo Download: https://visualstudio.microsoft.com/downloads/
    pause
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo [INFO] Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

REM Build
echo.
echo [INFO] Building AntiDetectPro...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo ============================================================
echo [SUCCESS] Build completed!
echo.
echo Output location: build\bin\Release\AntiDetectPro.exe
echo.
echo To run:
echo   .\build\bin\Release\AntiDetectPro.exe --help
echo ============================================================

pause
