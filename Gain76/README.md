# Gain 76 — moved

Gain 76 now lives as a fully independent project at **`C:\Dev\Gain76\`**
(own git repo, own JUCE copy, own `build/` dir — outside OneDrive so
builds aren't throttled by sync).

- Build:   `cmake --build C:\Dev\Gain76\build --config Release --target Gain76_VST3`
- Install: `tools\install-vst3.ps1 -Only "AlteredAudio Gain 76.vst3"` (scans `C:\Dev\Gain76\build`)
- Plugin code `AGn1` + param IDs unchanged — sessions keep loading.

Extracted at commit `9641536` of this repo (gain branch).
