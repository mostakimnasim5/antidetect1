@echo off
REM ============================================================
REM AntiDetectPro v1.8 - Windows Build Script (COMPLETE)
REM ============================================================
REM
REM Full Windows Build Script with Dependency Check
REM Requirements:
REM   - Visual Studio 2019/2022 with C++ tools
REM   - CMake 3.20+
REM   - OpenSSL 1.1.x or 3.x
REM
REM Usage:
REM   1. Open "Developer Command Prompt for VS 2022"
REM   2. Navigate to project directory
REM   3. Run: build_windows.bat
REM
REM ============================================================

setlocal enabledelayedexpansion

echo.
echo ============================================================ 
echo    AntiDetectPro v1.8 - Windows Build Script
echo ============================================================
echo.

REM =======================
REM CHECK VISUAL STUDIO
REM =======================
echo [1/6] Checking Visual Studio...

if not exist "%VSINSTALLDIR%" (
    if not exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        echo [ERROR] Visual Studio not found!
        echo Please install Visual Studio 2019 or 2022 with C++ tools
        echo Download: https://visualstudio.microsoft.com/downloads/
        pause
        exit /b 1
    )
    
    for /f "tokens=*" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
    if defined VS_PATH (
        set VSINSTALLDIR=!VS_PATH!
        echo [OK] Found Visual Studio at: !VSINSTALLDIR!
    ) else (
        echo [ERROR] Visual Studio not configured properly!
        pause
        exit /b 1
    )
) else (
    echo [OK] Visual Studio found: %VSINSTALLDIR%
)

REM =======================
REM CHECK CMAKE
REM =======================
echo.
echo [2/6] Checking CMake...

where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake not found in PATH!
    echo Please install CMake and add to PATH
    echo Download: https://cmake.org/download/
    pause
    exit /b 1
)

for /f "tokens=*" %%i in ('cmake --version 2^>nul ^| findstr /C:"cmake version"') do echo [OK] %%i

REM =======================
REM CHECK OPENSSL
REM =======================
echo.
echo [3/6] Checking OpenSSL...

set "OPENSSL_FOUND=0"

REM Check common OpenSSL locations
for %%p in ("C:\OpenSSL" "C:\Program Files\OpenSSL" "C:\Program Files (x86)\OpenSSL" "%VCPKG_INSTALLATION_ROOT%\installed\x64-windows" "C:\vcpkg\installed\x64-windows") do (
    if exist "%%~sp\include\openssl\ssl.h" (
        set "OPENSSL_ROOT=%%~sp"
        set "OPENSSL_FOUND=1"
        echo [OK] OpenSSL found: !OPENSSL_ROOT!
        goto :openssl_done
    )
)

:openssl_done

if %OPENSSL_FOUND% equ 0 (
    echo [WARNING] OpenSSL not found in standard locations.
    echo Install OpenSSL via one of these methods:
    echo   1. vcpkg: vcpkg install openssl:x64-windows
    echo   2. Download: https://slproweb.com/products/Win32OpenSSL.html
    echo Will try CMake find_package^(OpenSSL^)...
)

REM =======================
REM CLEAN PREVIOUS BUILD
REM =======================
echo.
echo [4/6] Cleaning previous build...

if exist "build" (
    echo Removing old build directory...
    rd /s /q "build" 2>nul
)

echo Creating new build directory...
mkdir build
cd build

REM =======================
REM CMAKE CONFIGURATION
REM =======================
echo.
echo [5/6] Running CMake configuration...

set "CMAKE_ARGS=-G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release"

if defined OPENSSL_ROOT (
    set "CMAKE_ARGS=!CMAKE_ARGS! -DOPENSSL_ROOT_DIR=!OPENSSL_ROOT!"
)

cmake !CMAKE_ARGS! ..

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] CMake configuration failed!
    echo.
    echo Common issues:
    echo   - OpenSSL not found - install via: vcpkg install openssl:x64-windows
    echo   - CMake cache corruption - delete build folder and retry
    pause
    exit /b 1
)

echo.
echo [OK] CMake configuration successful!

REM =======================
REM BUILD
REM =======================
echo.
echo [6/6] Building AntiDetectPro...

cmake --build . --config Release --parallel

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

REM =======================
REM LOCATE OUTPUT
REM =======================
echo.
echo ============================================================
echo    [SUCCESS] Build Completed!
echo ============================================================
echo.

REM Find the executable
set "EXE_PATH="
for /r "%CD%" %%f in (AntiDetectPro.exe) do (
    if exist "%%~ff" (
        set "EXE_PATH=%%~ff"
        goto :found_exe
    )
)

:found_exe

if defined EXE_PATH (
    echo [OK] Executable: %EXE_PATH%
    echo.
    echo ============================================================
    echo    How to Run:
    echo ============================================================
    echo.
    echo   1. Direct run:
    echo      "%EXE_PATH%" --help
    echo.
    echo   2. List connected devices:
    echo      "%EXE_PATH%" list-devices
    echo.
    echo   3. Interactive mode:
    echo      "%EXE_PATH%"
    echo.
) else (
    echo [WARNING] Executable not found in expected location
    echo Check build directory manually.
)

echo ============================================================
echo.
echo For GUI version, run: cmake .. -DBUILD_GUI=ON
echo.

pause
