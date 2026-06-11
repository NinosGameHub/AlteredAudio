# Installs every built AlteredAudio VST3 (rack + all single-module plugins)
# into C:\Program Files\Common Files\VST3\AlteredAudio\
#
# Usage:  powershell -ExecutionPolicy Bypass -File tools\install-vst3.ps1
# Self-elevates if not running as administrator.
# Uses rename-then-replace so installs succeed while a DAW has plugins loaded.

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build'
$destRoot = 'C:\Program Files\Common Files\VST3\AlteredAudio'
$oldRack  = 'C:\Program Files\Common Files\VST3\AlteredAudio.vst3'   # pre-0.4 location

# ---- Self-elevate ----
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
           ).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Start-Process powershell -Verb RunAs -Wait -ArgumentList `
        "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}

# ---- Collect built bundles ----
$bundles = Get-ChildItem -Path $buildDir -Directory -Filter '*_artefacts' |
    ForEach-Object { Get-ChildItem -Path (Join-Path $_.FullName 'Release\VST3') -Directory -Filter '*.vst3' -ErrorAction SilentlyContinue }

if (-not $bundles) {
    Write-Host 'No built VST3 bundles found — run a Release build first.' -ForegroundColor Red
    pause; exit 1
}

New-Item -ItemType Directory -Force -Path $destRoot | Out-Null

# ---- Install each bundle (rename-replace for locked binaries) ----
foreach ($bundle in $bundles) {
    $dest   = Join-Path $destRoot $bundle.Name
    $binDir = Join-Path $dest 'Contents\x86_64-win'

    if (Test-Path $binDir) {
        Get-ChildItem $binDir -Filter '*.vst3' | ForEach-Object {
            try { Rename-Item $_.FullName ($_.Name + '.old') -ErrorAction Stop } catch {}
        }
    }

    Copy-Item $bundle.FullName $destRoot -Recurse -Force
    Get-ChildItem $destRoot -Recurse -Filter '*.old' -ErrorAction SilentlyContinue |
        ForEach-Object { try { Remove-Item $_.FullName -Force -ErrorAction Stop } catch {} }

    Write-Host ("Installed  " + $bundle.Name)
}

# ---- Remove the old root-level rack install (now lives in the subfolder) ----
if (Test-Path $oldRack) {
    try {
        Remove-Item $oldRack -Recurse -Force -ErrorAction Stop
        Write-Host 'Removed old root-level AlteredAudio.vst3'
    } catch {
        Write-Host 'Old root-level AlteredAudio.vst3 is locked — close your DAW and delete it manually.' -ForegroundColor Yellow
    }
}

Write-Host "`nDone. All plugins are in $destRoot" -ForegroundColor Green
