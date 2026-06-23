# ============================================================
# AntiDetectPro v1.7 - Windows Build Script (PowerShell)
# ============================================================
#
# Modern PowerShell build script for Windows 10/11
# Features:
#   - Auto-detects Visual Studio
#   - Auto-downloads OpenSSL if missing
#   - Supports vcpkg integration
#   - Creates portable .exe
#
# Requirements:
#   - Windows 10/11
#   - Visual Studio 2019/2022
#   - Internet connection (for OpenSSL download)
#
# Usage:
#   1. Open PowerShell as Administrator
#   2. Navigate to project directory
#   3. Run: .\build.ps1
#
# ============================================================

param(
    [switch]$Clean,
    [switch]$UseVcpkg,
    [switch]$Verbose,
    [string]$VcpkgRoot,
    [string]$OpenSSLPath = "C:\OpenSSL-Win64"
)

$ErrorActionPreference = "Continue"
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "  AntiDetectPro v1.7 - Windows Build Script" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Detect Visual Studio
function Get-VisualStudio {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vsWhere) {
        $vs = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath, catalog -format json | ConvertFrom-Json
        
        if ($vs) {
            return @{
                Path = $vs.installationPath
                Version = $vs.catalog.productLineVersion
            }
        }
    }
    
    # Manual detection
    $paths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community",
        "${env:ProgramFiles}\Microsoft Visual Studio\2019\Enterprise",
        "${env:ProgramFiles}\Microsoft Visual Studio\2019\Professional",
        "${env:ProgramFiles}\Microsoft Visual Studio\2019\Community"
    )
    
    foreach ($path in $paths) {
        if (Test-Path $path) {
            return @{
                Path = $path
                Version = "Detected"
            }
        }
    }
    
    return $null
}

# Check for Visual Studio
Write-Host "[1/5] Checking Visual Studio..." -ForegroundColor Yellow
$vs = Get-VisualStudio

if (-not $vs) {
    Write-Host "[ERROR] Visual Studio not found!" -ForegroundColor Red
    Write-Host "Please install Visual Studio 2019 or 2022 with C++ tools" -ForegroundColor Red
    Write-Host "Download: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Cyan
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "  Found Visual Studio at: $($vs.Path)" -ForegroundColor Green
Write-Host "  Version: $($vs.Version)" -ForegroundColor Green

# Check for CMake
Write-Host ""
Write-Host "[2/5] Checking CMake..." -ForegroundColor Yellow

$cmake = Get-Command cmake -ErrorAction SilentlyContinue

if (-not $cmake) {
    Write-Host "  CMake not found. Installing via winget..." -ForegroundColor Yellow
    winget install Kitware.CMake --silent --accept-package-agreements --accept-source-agreements 2>$null
    
    if ($LASTEXITCODE -eq 0) {
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    }
}

if (-not $cmake) {
    Write-Host "[ERROR] CMake installation failed!" -ForegroundColor Red
    Write-Host "Please install CMake manually: https://cmake.org/download/" -ForegroundColor Cyan
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "  Found CMake: $($cmake.Source)" -ForegroundColor Green

# Check/Install OpenSSL
Write-Host ""
Write-Host "[3/5] Checking OpenSSL..." -ForegroundColor Yellow

$opensslInclude = Join-Path $OpenSSLPath "include\openssl\ssl.h"
$opensslLib = Join-Path $OpenSSLPath "lib\VC\libcrypto.lib"

$opensslFound = (Test-Path $opensslInclude) -and (Test-Path $opensslLib)

if (-not $opensslFound) {
    Write-Host "  OpenSSL not found at: $OpenSSLPath" -ForegroundColor Yellow
    Write-Host "  Downloading OpenSSL..." -ForegroundColor Yellow
    
    $opensslUrl = "https://slproweb.com/download/Win64OpenSSL-3_0_15.exe"
    $opensslInstaller = Join-Path $env:TEMP "Win64OpenSSL-3_0_15.exe"
    
    try {
        Write-Host "  Downloading OpenSSL 3.0.15..." -ForegroundColor Cyan
        Invoke-WebRequest -Uri $opensslUrl -OutFile $opensslInstaller -UseBasicParsing
        
        Write-Host "  Installing OpenSSL (silent)..." -ForegroundColor Cyan
        Start-Process -FilePath $opensslInstaller -ArgumentList "/silent", "/verysilent", "/norestart" -Wait
        
        # Refresh paths
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        
        $opensslFound = $true
        Write-Host "  OpenSSL installed successfully!" -ForegroundColor Green
    }
    catch {
        Write-Host "  [WARNING] OpenSSL download failed. Build may fail." -ForegroundColor Yellow
    }
}

if ($opensslFound) {
    Write-Host "  OpenSSL found: $OpenSSLPath" -ForegroundColor Green
} else {
    Write-Host "  [WARNING] OpenSSL not found. Build may fail." -ForegroundColor Yellow
}

# Clean build directory
Write-Host ""
Write-Host "[4/5] Preparing build directory..." -ForegroundColor Yellow

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "  Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
}

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Set-Location $BuildDir

# Configure CMake
Write-Host ""
Write-Host "[5/5] Configuring and building..." -ForegroundColor Yellow

$cmakeArgs = @(
    "..",
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_BUILD_TYPE=Release"
)

if ($UseVcpkg -and $VcpkgRoot) {
    $vcpkgToolchain = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
    if (Test-Path $vcpkgToolchain) {
        $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$vcpkgToolchain"
    }
}

if ($opensslFound) {
    $cmakeArgs += "-DOPENSSL_ROOT_DIR=$OpenSSLPath"
}

Write-Host "  CMake Arguments:" -ForegroundColor Cyan
$cmakeArgs | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }

# Configure
Write-Host ""
Write-Host "  Running CMake configure..." -ForegroundColor Yellow
cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[ERROR] CMake configuration failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Build
Write-Host ""
Write-Host "  Building AntiDetectPro..." -ForegroundColor Yellow
$jobs = [Environment]::ProcessorCount
cmake --build . --config Release -j $jobs

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[ERROR] Build failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Find the executable
$exePath = Get-ChildItem -Path . -Filter "AntiDetectPro.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1

Write-Host ""
Write-Host "============================================================" -ForegroundColor Green
Write-Host "  [SUCCESS] Build completed!" -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Green
Write-Host ""

if ($exePath) {
    Write-Host "Executable: $($exePath.FullName)" -ForegroundColor Cyan
    Write-Host "Size: $([math]::Round($exePath.Length / 1MB, 2)) MB" -ForegroundColor Cyan
    
    # Copy to bin directory
    $binDir = Join-Path $ProjectRoot "bin"
    if (-not (Test-Path $binDir)) {
        New-Item -ItemType Directory -Path $binDir | Out-Null
    }
    
    $destPath = Join-Path $binDir "AntiDetectPro.exe"
    Copy-Item -Path $exePath.FullName -Destination $destPath -Force
    
    Write-Host ""
    Write-Host "Copied to: $destPath" -ForegroundColor Green
}

Write-Host ""
Write-Host "Usage:" -ForegroundColor Cyan
Write-Host "  .\bin\AntiDetectPro.exe --help" -ForegroundColor White
Write-Host "  .\bin\AntiDetectPro.exe list-devices" -ForegroundColor White
Write-Host "  .\bin\AntiDetectPro.exe enable-sensor-spoofing" -ForegroundColor White
Write-Host ""
Write-Host "============================================================" -ForegroundColor Cyan

if (-not $Verbose) {
    Read-Host "Press Enter to exit"
}
