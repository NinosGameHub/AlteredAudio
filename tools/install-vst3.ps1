# Installs built AlteredAudio VST3s into C:\Program Files\Common Files\VST3\AlteredAudio\
#
# Usage:  powershell -ExecutionPolicy Bypass -File tools\install-vst3.ps1 [-Only <bundle name>]
#         -Only "AlteredAudio Gain 76.vst3"   installs just that bundle, touches nothing else
# Self-elevates if not running as administrator (forwards -Only through the relaunch).
# Uses rename-then-replace so installs succeed while a DAW has plugins loaded.
# Scans both the main build dir (build\) and standalone module dirs (build-gain\).

param(
    [string]$Only = ''
)

$ErrorActionPreference = 'Stop'

$repoRoot  = Split-Path -Parent $PSScriptRoot
$buildDirs = @((Join-Path $repoRoot 'build'), 'C:\Dev\Gain76\build', 'C:\Dev\Filter76\build', 'C:\Dev\Comp76\build')
$destRoot  = 'C:\Program Files\Common Files\VST3\AlteredAudio'
$oldRack   = 'C:\Program Files\Common Files\VST3\AlteredAudio.vst3'   # pre-0.4 location

# ---- Self-elevate ----
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
           ).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    $fwd = "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    if ($Only -ne '') { $fwd += " -Only `"$Only`"" }
    Start-Process powershell -Verb RunAs -Wait -ArgumentList $fwd
    exit
}

# ---- Collect built bundles ----
$bundles = @()
foreach ($buildDir in $buildDirs) {
    if (-not (Test-Path $buildDir)) { continue }
    $bundles += Get-ChildItem -Path $buildDir -Directory -Filter '*_artefacts' |
        ForEach-Object { Get-ChildItem -Path (Join-Path $_.FullName 'Release\VST3') -Directory -Filter '*.vst3' -ErrorAction SilentlyContinue }
}

if ($Only -ne '') {
    $bundles = $bundles | Where-Object { $_.Name -eq $Only -or $_.BaseName -eq $Only }
}

if (-not $bundles) {
    Write-Host 'No matching VST3 bundles found - run a Release build first.' -ForegroundColor Red
    pause; exit 1
}

# Same product can exist in several build dirs - keep the newest binary
$bundles = $bundles | Sort-Object {
        $bin = Join-Path $_.FullName ('Contents\x86_64-win\' + $_.Name)
        if (Test-Path $bin) { (Get-Item $bin).LastWriteTime } else { [datetime]::MinValue }
    } -Descending |
    Group-Object Name | ForEach-Object { $_.Group[0] }

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
    Get-ChildItem $dest -Recurse -Filter '*.old' -ErrorAction SilentlyContinue |
        ForEach-Object { try { Remove-Item $_.FullName -Force -ErrorAction Stop } catch {} }

    Write-Host ("Installed  " + $bundle.Name)
}

# ---- One-time cleanups (skipped when -Only is used) ----
if ($Only -eq '') {
    if (Test-Path $oldRack) {
        try {
            Remove-Item $oldRack -Recurse -Force -ErrorAction Stop
            Write-Host 'Removed old root-level AlteredAudio.vst3'
        } catch {
            Write-Host 'Old root-level AlteredAudio.vst3 is locked - close your DAW and delete it manually.' -ForegroundColor Yellow
        }
    }

    $renamed = @('AlteredAudio Filter.vst3',   # pre-0.7.1 name of Filter 76
                 'AlteredAudio Gain.vst3')     # pre-0.8.6 name of Gain 76
    foreach ($name in $renamed) {
        $stale = Join-Path $destRoot $name
        if (Test-Path $stale) {
            try {
                Remove-Item $stale -Recurse -Force -ErrorAction Stop
                Write-Host "Removed renamed plugin $name"
            } catch {
                Write-Host "$name is locked - close your DAW and delete it manually." -ForegroundColor Yellow
            }
        }
    }
}

Write-Host "`nDone." -ForegroundColor Green
