# Filter 76 v0.8.2 — one-click installer
# Right-click this file and choose "Run with PowerShell"

$pluginName = "AlteredAudio Filter 76.vst3"
$installDir = "C:\Program Files\Common Files\VST3"
$src = Join-Path $PSScriptRoot $pluginName
$dest = Join-Path $installDir $pluginName

# Re-launch elevated if not already admin
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltinRole]::Administrator)) {
    Start-Process powershell -Verb RunAs -Wait -ArgumentList `
        "-ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}

Write-Host ""
Write-Host "  ALTERED AUDIO  -  Filter 76  v0.8.2  Installer" -ForegroundColor Yellow
Write-Host ""

if (-not (Test-Path $src)) {
    Write-Host "  ERROR: plugin folder not found next to this script." -ForegroundColor Red
    Write-Host "  Make sure '$pluginName' is in the same folder as install.ps1."
    Read-Host "  Press Enter to exit"
    exit 1
}

# Remove old copy if present
if (Test-Path $dest) {
    Write-Host "  Removing previous install..." -ForegroundColor Gray
    Remove-Item $dest -Recurse -Force
}

Write-Host "  Installing to: $installDir" -ForegroundColor Cyan
Copy-Item $src $dest -Recurse -Force

if (Test-Path "$dest\Contents\x86_64-win\$pluginName") {
    Write-Host ""
    Write-Host "  Installation successful!" -ForegroundColor Green
    Write-Host "  Rescan plugins in your DAW to use Filter 76."
} else {
    Write-Host ""
    Write-Host "  Something went wrong. File not found after copy." -ForegroundColor Red
}

Write-Host ""
Read-Host "  Press Enter to exit"
