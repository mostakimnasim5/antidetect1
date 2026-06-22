@echo off
chcp 65001 >nul
title AntiDetectPro Build Script

echo.
echo  ███████╗███████╗ ██████╗████████╗ ██████╗ ██████╗ ██╗   ██╗██╗
echo  ██╔════╝██╔════╝██╔════╝╚══██╔══╝██╔═══██╗██╔══██╗██║   ██║██║
echo  █████╗  ███████╗██║        ██║   ██║   ██║███████║██║   ██║██║
echo  ██╔══╝  ╚════██║██║        ██║   ██║   ██║██╔══██║██║   ██║██║
echo  ███████╗███████║╚██████╗   ██║   ╚██████╔╝██████╔╝╚██████╔╝██╗
echo  ╚══════╝╚══════╝ ╚═════╝   ╚═╝    ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝
echo.
echo  Enterprise Anti-Detection System - Build Script
echo  Version: 1.0.0
echo  ==============================================
echo.

:: Check if Qt is installed
echo [1/5] Checking prerequisites...
where qmake >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Qt not found in PATH. Make sure Qt is installed.
    echo Download from: https://www.qt.io/download
    echo.
)

:: Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found. Please install CMake.
    echo Download from: https://cmake.org/download
    pause
    exit /b 1
)

:: Check for MSBuild
where msbuild >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: MSBuild not found. Will try with cmake --build.
)

:: Create build directory
echo.
echo [2/5] Creating build directory...
if exist "build" rmdir /s /q build
mkdir build
cd build

:: Configure with CMake
echo.
echo [3/5] Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON

if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    pause
    exit /b 1
)

:: Build
echo.
echo [4/5] Building AntiDetectPro...
cmake --build . --config Release --parallel

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    echo.
    pause
    exit /b 1
)

:: Done
echo.
echo [5/5] Build complete!
echo.
echo ==============================================
echo.
echo SUCCESS! AntiDetectPro built successfully.
echo.
echo Output location:
if exist "bin\Release\AntiDetectPro.exe" (
    echo   %cd%\bin\Release\AntiDetectPro.exe
    echo.
    echo To run the application:
    echo   bin\Release\AntiDetectPro.exe
) else if exist "bin\AntiDetectPro.exe" (
    echo   %cd%\bin\AntiDetectPro.exe
    echo.
    echo To run the application:
    echo   bin\AntiDetectPro.exe
) else (
    dir /s /b AntiDetectPro.exe
)
echo.
echo.
echo Press any key to exit...
pause >nul
