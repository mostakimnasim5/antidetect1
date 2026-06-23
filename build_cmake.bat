@echo off
REM ============================================================
REM AntiDetectPro v1.5 - Windows CMake Build (Alternative)
REM ============================================================
REM 
REM This script uses CMake directly for building
REM Requirements:
REM   - CMake 3.20+
REM   - Visual Studio 2019/2022
REM   - OpenSSL development files
REM
REM Quick Setup (without vcpkg):
REM   1. Download OpenSSL from: https://slproweb.com/products/Win32OpenSSL.html
REM   2. Install to C:\OpenSSL-Win64
REM   3. Run this script
REM
REM With vcpkg:
REM   1. Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git
REM   2. vcpkg integrate install
REM   3. vcpkg install openssl:x64-windows
REM   4. Set VCPKG_ROOT environment variable
REM   5. Run this script with -DUSE_VCPKG=ON
REM
REM ============================================================

setlocal

REM Parse arguments
set USE_VCPKG=OFF
set VCPKG_ROOT=%VCPKG_ROOT%

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="-DUSE_VCPKG" (
    set USE_VCPKG=ON
)
if /i "%~1"=="ON" (
    set USE_VCPKG=ON
)
shift
goto :parse_args

:args_done

echo ============================================================
echo AntiDetectPro v1.5 - Windows Build (CMake)
echo ============================================================
echo.

REM Detect Visual Studio version
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community" (
    set VS_VERSION=17 2022
    set VS_GEN=Visual Studio 17 2022
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional" (
    set VS_VERSION=17 2022
    set VS_GEN=Visual Studio 17 2022
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise" (
    set VS_VERSION=17 2022
    set VS_GEN=Visual Studio 17 2022
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2019\Community" (
    set VS_VERSION=16 2019
    set VS_GEN=Visual Studio 16 2019
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2019\Professional" (
    set VS_VERSION=16 2019
    set VS_GEN=Visual Studio 16 2019
) else (
    echo [WARNING] Could not detect Visual Studio version
    set VS_GEN=Visual Studio 17 2022
)

echo [INFO] Detected Visual Studio: %VS_GEN%

REM Create build directory
if not exist "build" mkdir build
cd build

REM Clean previous build
if exist CMakeCache.txt (
    echo [INFO] Cleaning previous build...
    del /f /q CMakeCache.txt 2>nul
    for /d /r . %%d in (CMakeFiles) do (
        if exist "%%d" rmdir /s /q "%%d"
    )
)

REM Configure CMake
set CMAKE_ARGS=-G "%VS_GEN%" -A x64 -DCMAKE_BUILD_TYPE=Release

REM Add OpenSSL path if not using vcpkg
if "%USE_VCPKG%"=="OFF" (
    if exist "C:\OpenSSL-Win64\lib\VC_STATIC_LIB" (
        set CMAKE_ARGS=%CMAKE_ARGS% -DOPENSSL_ROOT_DIR=C:\OpenSSL-Win64 -DOPENSSL_CRYPTO_LIBRARY=C:\OpenSSL-Win64\lib\VC\libcrypto.lib -DOPENSSL_SSL_LIBRARY=C:\OpenSSL-Win64\lib\VC\libssl.lib
    )
)

REM With vcpkg
if "%USE_VCPKG%"=="ON" (
    if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
        set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
    )
)

echo [INFO] Running CMake configuration...
echo [INFO] Arguments: %CMAKE_ARGS%

cmake .. %CMAKE_ARGS%

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] CMake configuration failed!
    echo.
    echo Troubleshooting:
    echo   1. Install OpenSSL: winget install OpenSSL
    echo   2. Or download from: https://slproweb.com/products/Win32OpenSSL.html
    echo   3. Or use vcpkg: set VCPKG_ROOT=C:\path\to\vcpkg
    echo.
    pause
    exit /b 1
)

REM Build
echo.
echo [INFO] Building AntiDetectPro...
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo ============================================================
echo [SUCCESS] AntiDetectPro v1.5 Build Completed!
echo ============================================================
echo.
echo Output executable:
if exist "bin\Release\AntiDetectPro.exe" (
    echo   bin\Release\AntiDetectPro.exe
) else if exist "AntiDetectPro\Release\AntiDetectPro.exe" (
    echo   AntiDetectPro\Release\AntiDetectPro.exe
) else (
    dir /s /b *.exe 2>nul
)
echo.
echo Usage:
echo   AntiDetectPro.exe --help
echo   AntiDetectPro.exe list-devices
echo   AntiDetectPro.exe enable-sensor-spoofing
echo ============================================================

endlocal
pause
