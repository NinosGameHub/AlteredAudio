# Gain 76 — moved

Gain 76 now lives as a fully independent project at **`C:\Dev\Gain76\`**
(own git repo, own JUCE copy, own `build/` dir — outside OneDrive so
builds aren't throttled by sync).

- Build:   `cmake --build C:\Dev\Gain76\build --config Release --target Gain76_VST3`
- Install: `tools\install-vst3.ps1 -Only "AlteredAudio Gain 76.vst3"` (scans `C:\Dev\Gain76\build`)
- Plugin code `AGn1` + param IDs unchanged — sessions keep loading.

Extracted at commit `9641536` of this repo (gain branch).

## Changelog

### 2026-06-13 — v1.1.0: In-plugin drag editor
- **Right-click** on the faceplate opens a context menu: "Edit Layout" / "Save Layout" / "Exit Edit Mode"
- In edit mode: cyan/orange bordered drag handles per component; drag body to move, drag bottom-right corner to resize
- Live x/y/w/h readout shown above each component while editing
- "Save Layout" writes positions to `%AppData%\AlteredAudio\Gain76\layout.json`; positions restored on next startup (`LayoutStore`)
- `EditOverlay` component covers the content area and intercepts mouse events only over tracked components
- Fixed knob scale labels to follow the knob's dragged position (no longer hardcoded)


### 2026-06-13 — Design test (TEMPORARY — revert via `design-test` branch in C:\Dev\Gain76)
- Added `AlteredAudioLookAndFeel.h` + `PluginEditor.h` to `Source/`
- New editor: vintage cream panel, bakelite rotary knob, amber VU meters, stereo bar
- Processor exposes `getInputLevel()`, `getOutputLevel()`, `getAPVTS()` for the new editor
- **To revert**: in C:\Dev\Gain76, `git checkout master` then rebuild + reinstall

### 2026-06-13 — Visual corrections (session 2)
- **Meters**: segment width +10% (22 → 24 px); meters repositioned to panel edges (x=36 / x=885)
- **Footer PEAK**: removed dark readout box; value now plain black text (`textPrimary`)
- **Footer LUFS**: same — removed dark box, plain black text

### 2026-06-13 — v1.0.2: Design system implementation (gain76-design/ projectId 86f4c874)
- **Knob**: added static tick ring outside rim (48 fine ticks, longer majors, amber zero mark at 12 o'clock); bold amber tracking tick follows value; gain value + "GAIN · DB" label printed on face
- **Meters**: segment width 24→22 px to match design spec; dark readout box replaced with plain text readout below segments
- **Meter caps**: INPUT/OUTPUT labels now heading 11px bold + "dB" unit below in inkFaint
- **Footer**: label font 9→10 px mono centred; value font 12→15 px mono centred; ComboBoxes repositioned to align with value row

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
