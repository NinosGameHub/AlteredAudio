# Altered Audio — Filter 76  v0.8.2
### Windows 64-bit VST3

---

## What gets installed and where

| Item | Location |
|------|----------|
| Plugin (VST3) | `C:\Program Files\Common Files\VST3\AlteredAudio Filter 76.vst3\` |
| User presets | `Documents\AlteredAudio\Filter 76\Presets\` *(created on first save)* |

The installer does **not** touch the registry or any other system folders.  
To uninstall, delete the `AlteredAudio Filter 76.vst3` folder from the VST3 location above.

---

## Installation

### Option A — Automatic (recommended)

1. Right-click **install.ps1**
2. Choose **"Run with PowerShell"**
3. Accept the UAC admin prompt
4. When you see *"Installation successful!"* press Enter
5. Rescan plugins in your DAW

### Option B — Manual copy

Copy the `AlteredAudio Filter 76.vst3` folder into either:

- **System-wide (all users):** `C:\Program Files\Common Files\VST3\`
- **Current user only:** `C:\Users\YourName\AppData\Roaming\VST3\`

Then rescan plugins in your DAW.

---

## Requirements

- Windows 10 64-bit or later
- VST3-compatible DAW (Ableton Live, Bitwig, FL Studio, Reaper, Studio One, etc.)
- Any modern x86-64 CPU

---

## Quick start

| Action | What it does |
|--------|-------------|
| Drag the node on the spectrum display | Set frequency / resonance (Q) |
| Scroll on the display | Adjust drive |
| Double-click the display | Reset frequency + Q |
| Right-click RATE / ATTACK / RELEASE | Tempo-sync options |
| Click the preset number | Browse all presets |
| Click **SAVE** | Save current state as a new preset |

---

## Factory presets

| # | Name | Description |
|---|------|-------------|
| 001 | INIT | Default state |
| 002 | ANALOG LOWPASS | Warm resonant low-pass |
| 003 | ACID SQUELCH | Envelope-modulated squelch |
| 004 | SLOW SWEEP | LFO low-pass sweep |
| 005 | TELEPHONE | Bandpass telephone effect |
| 006 | NOTCH WOBBLE | Notch with LFO wobble |
