@echo off
echo ================================================
echo    OS-Level Anti-Detect System - Windows Build
echo ================================================
echo.

REM Check if g++ is installed
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: g++ not found!
    echo Please install MinGW-w64 or MSYS2
    echo Download: https://www.msys2.org/
    echo.
    echo After installing, run: pacman -S mingw-w64-gcc
    pause
    exit /b 1
)

echo [*] Compiling Anti-Detect System...
echo.

REM Compile with static linking
g++ -std=c++17 -O2 -static ^
    main.cpp ^
    system_fingerprint.cpp ^
    advanced_fingerprint.cpp ^
    android_spoof.cpp ^
    -o antidetect.exe ^
    -lws2_32 -lwinpthread -lm

if %errorlevel% equ 0 (
    echo.
    echo ================================================
    echo    BUILD SUCCESSFUL!
    echo ================================================
    echo.
    echo antidetect.exe has been created!
    echo.
    echo Usage:
    echo   antidetect.exe --info        : Show system info
    echo   antidetect.exe --list       : List presets
    echo   antidetect.exe --generate    : Generate profile
    echo   antidetect.exe --help        : Show help
    echo.
) else (
    echo.
    echo ================================================
    echo    BUILD FAILED!
    echo ================================================
    pause
    exit /b 1
)

pause
