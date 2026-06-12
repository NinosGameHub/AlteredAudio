# Filter 76 — moved

Filter 76 now lives as a fully independent project at **`C:\Dev\Filter76\`**
(own git repo, own JUCE copy, own `build/` dir — outside OneDrive).

- Build:   `cmake --build C:\Dev\Filter76\build --config Release --target Filter76_VST3`
- Install: `tools\install-vst3.ps1 -Only "AlteredAudio Filter 76.vst3"`
- Plugin code `AFlt` + param IDs unchanged — sessions keep loading.

Extracted from the `filter` branch at `3cbd19f` (v0.8.5). The in-repo
sources (source/FilterModule.cpp etc.) remain for the rack's internal
chain; source/plugins/FilterPlugin.cpp + AuroraFilterEditor.* are no
longer built here.
