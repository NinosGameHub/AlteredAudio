# Gain 76 — moved

Gain 76 now lives as a fully independent project at **`C:\Dev\Gain76\`**
(own git repo, own JUCE copy, own `build/` dir — outside OneDrive so
builds aren't throttled by sync).

- Build:   `cmake --build C:\Dev\Gain76\build --config Release --target Gain76_VST3`
- Install: `tools\install-vst3.ps1 -Only "AlteredAudio Gain 76.vst3"` (scans `C:\Dev\Gain76\build`)
- Plugin code `AGn1` + param IDs unchanged — sessions keep loading.

Extracted at commit `9641536` of this repo (gain branch).

## Changelog

### 2026-06-13 — Design implementation (UI overhaul)
- Window: 820 × 820 → **980 × 980** (matches design spec in `gain76-design/`)
- Added **title bar**: "Down In The Dark / AlteredAudio Gain 76" + × decoration
- **Meters**: removed dark-well background; segments are now embossed amber/grey
  gradients mounted directly on the faceplate surface (matches `PeakMeter.jsx`)
- Meter dB range: −60…+6 → **−24…+24** with 52 segments, warn orange ≥ 0 dB
- **Footer PEAK / LUFS**: plain text → dark readout boxes (graphBg + amber digits,
  matching `Readout.jsx` dark tone)
- Removed horizontal L/R peak-hold strip (not in `Design_2.png` reference)
- Design system: `gain76-design/` (projectId `86f4c874`) — local mirror committed
  to gain branch on same date
