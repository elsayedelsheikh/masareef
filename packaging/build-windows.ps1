<#
.SYNOPSIS
    Builds a portable Windows zip of Masareef.

.DESCRIPTION
    Run from Windows PowerShell (not from inside WSL). On first run it
    downloads Qt (with the Charts module) and the matching MinGW toolchain
    into %LOCALAPPDATA%\masareef-build\qt via aqtinstall, then builds with
    CMake, gathers the Qt DLLs with windeployqt and zips a portable
    folder - no installer.

.EXAMPLE
    PS> cd packaging
    PS> .\build-windows.ps1
#>
param(
    [string]$QtVersion = "6.4.2",
    [string]$AppVersion = "1.0.0"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
# Qt must live on a Windows-local drive: windeployqt cannot handle a Qt
# installation on a \\wsl$ UNC share (and downloads/builds are faster).
$qtRoot = Join-Path $env:LOCALAPPDATA "masareef-build\qt"
$qtPath = Join-Path $qtRoot "$QtVersion\mingw_64"
$mingwBin = Join-Path $qtRoot "Tools\mingw1120_64\bin"

function Require-Command($name, $hint) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
        throw "$name was not found. $hint"
    }
}

Require-Command python "Install Python 3 from https://python.org or the Microsoft Store."
Require-Command cmake "Install CMake, e.g.: winget install Kitware.CMake"

# --- 1. Qt + MinGW toolchain via aqtinstall (first run only) ---------------
Write-Host "==> Ensuring aqtinstall" -ForegroundColor Cyan
python -m pip install --quiet --upgrade aqtinstall

if (-not (Test-Path $qtPath)) {
    Write-Host "==> Downloading Qt $QtVersion (base + charts) into $qtRoot" -ForegroundColor Cyan
    python -m aqt install-qt windows desktop $QtVersion win64_mingw -m qtcharts -O $qtRoot
}
if (-not (Test-Path $mingwBin)) {
    Write-Host "==> Downloading the MinGW 11.2 toolchain" -ForegroundColor Cyan
    python -m aqt install-tool windows desktop tools_mingw90 -O $qtRoot
}

$env:PATH = "$mingwBin;$(Join-Path $qtPath 'bin');$env:PATH"

# --- 2. Build ---------------------------------------------------------------
$buildDir = Join-Path $repoRoot "build-win"
Write-Host "==> Configuring ($buildDir)" -ForegroundColor Cyan
cmake -B $buildDir -S $repoRoot -G "MinGW Makefiles" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_PREFIX_PATH="$qtPath" `
    -DCMAKE_MAKE_PROGRAM="$(Join-Path $mingwBin 'mingw32-make.exe')" `
    -DMASAREEF_BUILD_TESTS=OFF
Write-Host "==> Building" -ForegroundColor Cyan
cmake --build $buildDir --parallel

# --- 3. Deploy Qt runtime ----------------------------------------------------
# Staged in a Windows-local directory: windeployqt cannot resolve its
# plugins for a target exe on a \\wsl$ UNC path.
$stageRoot = Join-Path $env:TEMP "masareef-dist"
$stageDir = Join-Path $stageRoot "Masareef"
Write-Host "==> Deploying to $stageDir" -ForegroundColor Cyan
if (Test-Path $stageRoot) { Remove-Item -Recurse -Force $stageRoot }
New-Item -ItemType Directory -Force -Path $stageDir | Out-Null
Copy-Item (Join-Path $buildDir "masareef.exe") $stageDir

# Gathers Qt DLLs, the sqlite driver, charts and the MinGW runtime
& (Join-Path $qtPath "bin\windeployqt.exe") --release --compiler-runtime `
    --no-translations (Join-Path $stageDir "masareef.exe")
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed (exit code $LASTEXITCODE)" }
if (-not (Test-Path (Join-Path $stageDir "platforms\qwindows.dll"))) {
    throw "windeployqt did not deploy the Windows platform plugin"
}

# --- 4. Zip ------------------------------------------------------------------
$zipLocal = Join-Path $stageRoot "Masareef-$AppVersion-win64.zip"
Write-Host "==> Zipping" -ForegroundColor Cyan
Compress-Archive -Path $stageDir -DestinationPath $zipLocal -Force

$zipPath = Join-Path $PSScriptRoot "Masareef-$AppVersion-win64.zip"
if (Test-Path $zipPath) { Remove-Item -Force $zipPath }
Move-Item $zipLocal $zipPath

# Keep an unzipped copy next to the script for quick inspection
$distDir = Join-Path $PSScriptRoot "dist"
if (Test-Path $distDir) { Remove-Item -Recurse -Force $distDir }
New-Item -ItemType Directory -Force -Path $distDir | Out-Null
Copy-Item -Recurse $stageDir (Join-Path $distDir "Masareef")

Write-Host "Done. Portable build: $zipPath" -ForegroundColor Green
Write-Host "Unzip anywhere and run Masareef\masareef.exe - data is stored under %APPDATA%\Masareef."
