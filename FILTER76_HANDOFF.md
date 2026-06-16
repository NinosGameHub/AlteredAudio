# Filter 76 — Complete Handoff (Light + Prime)

_Last updated 2026-06-16. Read **§ Session handoff** first, then the rest._

---

## ⭐ Session handoff — 2026-06-16 (READ FIRST)

**Where Prime is:** the JUCE **Filter 76 Prime** editor (`Source/Prime76Editor.h/.cpp`, on the
`filter-prime` branch) now has the **full UI**, all sprite/Blender-driven:
- **Faceplate** sprite background (`Resources/prime_faceplate.png`, 8-bit), window sized to the
  plate **1405×900** (drawn 1:1; `createEditor` returns `Prime76Editor`).
- **13 knobs** (FREQ/RES/DRIVE/MIX/GAIN[hero]/OUT[small] + MOD AMOUNT + LFO RATE/DEPTH/PHASE +
  ENV ATK/REL/SENS) — `prime_knob.png` + rotating `prime_indicator.png`.
- **Readouts** (12): under each hero knob + the 7 bottom knobs — show function name idle / value
  while turning (`ReadoutScreen`, Michroma glyph font).
- **Buttons** (`OptionLeds`, fixed LED + movable text label): filter type LP/HP/BP/NOTCH/PEAK/SHELF
  (SHELF cycles Low↔High shelf), SLOPE 12/24/48 (dimmed unless LP/HP), MODE ANALOG/CLEAN.
- **Mod source/dest** (left panels): LFO A/LFO B/EF + FREQ/RESO/DRIVE (`OptionLeds`, source toggles
  OFF, dest gated by source).
- **LFO**: live `LfoScope` (amplitude tracks DEPTH, waveform shifts with PHASE) + **4 wave-select
  screens** (`WaveSelect`, sine/tri/square/random symbols) below it; RATE readout click = tempo
  SYNC (shows division increments while turning). LFO A/B selection rebinds knobs/readouts/scope/
  wave via `rebindLfo`.
- **Live screens**: `ResponseDisplay` (filter curve + spectrum, **±24 dB** scale, "24" corner,
  "20k"), `EnvScope`.
- **Header** (top strip): preset `<`/name/`>`/SAVE (`PresetManager` + INIT), A/B compare, PWR
  (bypass), OS cycle, MIX readout — glyph font via `GlyphLnF`.
- **Footer** info line + **peak readout** above meters (hold-until-silence).
- **In-app Layout Editor**: right-click → "Layout edit mode" → click text → arrow-nudge (Shift=10);
  positions persist to `Documents/AlteredAudio/Filter 76/prime_layout.json` (baked defaults in
  `applyDefaultPositions`).

**Build:** `cmake --build build --config Release --target Filter76_Standalone Filter76_VST3`
(Stop-Process `AlteredAudio Filter 76` first — exe locks). **Verify:** launch the Standalone via
PowerShell `Start-Process`, screenshot by `MainWindowHandle` (SetWindowPos topmost 0x0041 +
CopyFromScreen), crop with System.Drawing. Desktop shortcut "AlteredAudio Filter 76 Prime".

**Blender export:** `blender --background "C:/Dev/Blender/Filter 76 Prime/Filter76_Prime.blend"
--python C:/Users/ninov/filter76_export.py -- faceplate` (or `knob`/`indicator`). 64 samples,
~5-6 min, **maxes all CPU cores = the "Blender 90% CPU"**. ⚠️ **GOTCHA: the user edits in the live
Blender GUI but often doesn't SAVE** — the headless render reads the disk file, so before
re-rendering, `bpy.ops.wm.save_mainfile()` via the socket (or have them Ctrl+S). Several requested
re-renders (slope plate, freq container) showed **no change** because the edits weren't in the file.

**Remaining / next (task #8):** live **output meters** — the `Meter_L/R`/`MeterStrip_L/R` are baked
static in the faceplate; need a `VerticalMeter`-style overlay driven by `analysis.peakL/R` (reuse
Gain 76's `VerticalMeter` + a `meter_strip` sprite). Also pending on the user saving Blender edits:
re-render freq-display container if resized. Glyph font lacks `%` `/` lowercase (kept values
unit-light) — render those glyphs if needed.

**Working style (user):** iterates fast, many small visual tweaks; **act on their visual judgment,
don't argue pixel measurements** — adjust or give them a control (see
`[[feedback-visual-not-measurements]]`). Flag design decisions, they revert freely.

---

## 0. TL;DR

Filter 76 is the AlteredAudio standalone **filter plugin** (JUCE 8 + CMake, C++17),
at `C:\dev\filter76` (own JUCE copy in `JUCE/`, own `build/`, both git‑ignored).
There are two UI versions, kept as git branches:

- **`filter-light`** — the existing, **working procedural UI** (`AuroraFilterEditor`).
  Ship‑ready. Leave it as the "Light Version."
- **`filter-prime`** — the new **Blender‑rendered hardware UI** (like Gain 76),
  built from "The VST MOD" asset library. **UI design in progress in Blender;
  no Prime‑specific C++ yet** (it currently shares the Light source).

The DSP is identical for both; only the editor changes.

---

## 1. Repo / branches

- **Local repo:** `C:\dev\filter76` (no remote of its own). Branches: `filter`,
  `master`, **`filter-light`**, **`filter-prime`** (current).
- **Remote:** pushed to the **AlteredAudio** GitHub repo
  (`github.com/NinosGameHub/AlteredAudio`) as branches **`filter-light`** and
  **`filter-prime`**. ⚠️ The remote URL has a **GitHub PAT in plaintext** — rotate it.
- **Prime design files** live outside the code repo:
  - Blender scene: `C:\Dev\Blender\Filter 76 Prime\Filter76_Prime.blend`
    (mirrored into the repo at `Prime/Filter76_Prime.blend`).
  - Asset library: `…\OneDrive\Documents\AlteredAudio\The VST MOD` (shared).

---

## 2. Build

```
cmake -B build -S .
cmake --build build --config Release --target Filter76_VST3 Filter76_Standalone
```
(Targets per `CMakeLists.txt`.) Stop the standalone before rebuilding (exe lock).

---

## 3. DSP / Parameters (shared by Light + Prime — `ParameterIDs.h`)

**Filter core:** `filter_type` (LP/HP/BP/Notch/Peak/Shelf), `filter_freq`,
`filter_q`, `filter_gain`, `filter_slope` (12/24/48 dB/oct), `filter_mode`
(Analog tanh‑drive / Clean), `filter_drive` (0–24 dB input sat), `filter_mix`
(dry/wet), `filter_output` (trim), `filter_oversampling` (1×/4×/8×),
`filter_bypass`. Global `channel_mode`.

**Aurora modulation engine:** `flt_mod_source` (OFF/LFO A/LFO B/ENV),
`flt_mod_dest` (FREQ/RES/DRIVE), `flt_mod_amount` (−1..+1); **LFO A & B**
(wave / rate / depth / phase / tempo‑sync + division); **envelope follower**
(attack / release / sens, each with optional tempo‑sync). DSP in
`FilterModule` / `BiquadFilter` / `SingleModuleProcessor`; analysis bridge
`FilterAnalysisSource` (FFT ring buffer for the spectrum, peaks, env, LFO phase).

---

## 4. Light UI — `AuroraFilterEditor` (procedural, shipping)

Fixed design space **1400 × 900**, scaled to fit. `AuroraLookAndFeel` = matte‑cream
puck knobs + floating tick ring + amber dot, LED option rows, dark amber readout
wells. Layout (`resized()`):

- **Header (y 6–64):** preset `< name >` + SAVE, A/B, oversampling combo, MIX
  drag‑readout, power key.
- **Display (16,78, ~1190×340):** `ResponseDisplay` — log‑freq spectrum + exact
  filter response curve + draggable node. **Meter column** (stereo peaks) at the right.
- **Filter row (y 432–632):** FILTER TYPE option list (6) · 5 hero knobs
  FREQ/RES/DRIVE/MIX/OUT (150 px) + GAIN (72 px) · SLOPE + MODE option lists.
- **Bottom row (y 646–832):** MODULATION (source/dest dropdowns + AMOUNT knob) ·
  LFO ENGINE (4 wave buttons + `LfoScope` + RATE/DEPTH/PHASE) · ENVELOPE FOLLOWER
  (`EnvScope` + ATK/REL/SENS).

Presets in `~/Documents/AlteredAudio/Filter 76/Presets/*.f76preset`.

---

## 5. Prime UI — Blender‑rendered (in progress)

Goal: replace the procedural look with Blender‑rendered brass/amber hardware, reusing
**"The VST MOD"** asset library (see its own reference). Same controls as §4, but as
rendered sprites driven by JUCE (the Gain 76 pattern).

### Blender scene — `C:\Dev\Blender\Filter 76 Prime\Filter76_Prime.blend`
- Created from the VST MOD **Template.blend**. **Landscape** to match the 1400×900
  design: render **2800×1800** (@2×), ortho camera, **182.3 px/world‑unit** (same as
  Gain), so JUCE overlay math is identical.
- **Faceplate** (`F76_Background`) sized to the **exact frame aspect** (15.36 × 9.87
  world) and the **camera fits it 1:1** (ortho_scale 15.36, centred) → the render is
  the whole faceplate, edge‑to‑edge, no margin/cut. Everything to be rendered must sit
  on the plate.
- Blocked out from VST MOD assets at the procedural control positions: 6 filter knobs
  (hero row) + 7 mod/LFO/env knobs (scaled to ~0.42/0.24, then ‑20%), stereo meter(s),
  and screen plates. A **reference image** of the Light UI (`Empty.005`, image.png) is
  overlaid at 50 % as a tracing guide (non‑selectable; hide with H).
- **Render settings:** Cycles, **128 samples + denoise**, 100 %, film‑transparent.

### ⚠️ Render rule (learned the hard way)
**Do the final full‑res (2800×1800) render with F12 *inside Blender*.** Driving a
full‑res render through the blender‑mcp socket **crashed Blender** — automated/preview
renders must stay low‑res/low‑sample.

### Build / helper scripts (in `C:\Users\ninov`, run via `blender_cmd.py`)
- `build_filter76_prime.py` — rebuilds the blockout from Template + VST MOD assets.
- `add_f76_background.py` — adds the faceplate plate.
- `tray_screens.py` / `make_screens_real.py` / `separate_and_fill.py` — screen parts
  as separate objects + camera fill.
- `arrange_f76.py`, `restore_f76.py` — layout/position helpers.

---

## 6. The VST MOD library notes (used by Prime)
- Registered library "The VST MOD" (`…\AlteredAudio\The VST MOD`), **Import Method =
  Append** (so drags come in local/editable).
- Dragging a **collection** asset → press **F9 → uncheck "Instance"** for real objects;
  or after drop, **Object ▸ Apply ▸ Make Instances Real**, then **U ▸ Object & Data**.
- Linked (read‑only) data → **L ▸ Make Local** (or it's already local with Append).
- The **meter** is now a single joined object asset ("Meter") for easy single‑object
  import (housing+strip merged — re‑separate if the plugin needs to light segments).

---

## 7. Current state & next steps
- **Light:** complete, on `filter-light`.
- **Prime:** **design DONE in Blender** (user-confirmed 2026-06-16). Outliner organized +
  named into UI-region collections under root **`Filter76`**: `00_Faceplate`, `01_Display`
  (Display_Bezel/Glass), `02_OutputMeter` (Meter_L/R, MeterStrip_L/R, OutMeter_Bezel/Label),
  `03_FilterType`, `04_SlopeMode`, `05_FilterKnobs` (Knob_FREQ/RES/DRIVE/MIX/OUT/GAIN),
  `06_FilterReadouts`, `07_Modulation` (Knob_MOD_AMOUNT), `08_LFO`
  (Knob_LFO_RATE/DEPTH/PHASE, LFO_Scope_Bezel/Glass), `09_Envelope`
  (Knob_ENV_ATTACK/RELEASE/SENS, Env_Scope_Bezel/Glass), `10_Footer`, `11_Reference`
  (overlay image + helper empties; don't render). Small button plates/LEDs are indexed
  (`_Plate_N`/`_LED_N`), not per-role.
- **Export pipeline (working):** headless `blender --background "Filter76_Prime.blend"
  --python C:\Users\ninov\filter76_export.py -- <sprite>` (full-res is reliable headless;
  the **socket** crashes at full-res). Camera centered on plate, **ortho_scale 15.69**,
  render **2862×1801**, film-transparent RGBA, Cycles 128+denoise → **182.36 px/world-unit**
  (matches Gain). Isolation = `hide_render` toggle; faceplate = all meshes with the 13
  `Knob_*` instances hidden. Assets land in **`C:\dev\filter76\Assets\Export\`**.
  - ✅ `faceplate.png` (static chrome, knob caps hidden) and ✅ `knob.png` exported + verified.
  - ⚠️ `knob.png` currently bakes ticks **and** the indicator dot together — split the dot
    into a separate `indicator.png` (use the new `VSTMOD_LED` asset) so JUCE rotates only the
    dot over static ticks (Gain pattern).
  - **Next sprites:** `indicator.png` (LED dot, LED_Light ON), `meter_strip.png` (amber strip
    as alpha mask), screen glass for Display/LFO/Env (live FFT/scope drawn in JUCE).
- **Prime JUCE editor v1 — BUILT + verified (2026-06-16).** New `Source/Prime76Editor.h/.cpp`
  (`Prime76Editor` + `Prime76LookAndFeel`); `FilterPlugin::createEditor` now returns it
  (was `AuroraFilterEditor`, which stays in the tree unused). Canvas **1431×900**; draws
  `prime_faceplate.png` as background + **13 sprite knobs** (FREQ/RES/DRIVE/MIX/OUT/GAIN,
  MOD_AMOUNT, LFO RATE/DEPTH/PHASE, ENV ATK/REL/SENS) wired to the APVTS via
  `SliderAttachment`. `drawRotarySlider` blits static `prime_knob.png` + rotates
  `prime_indicator.png` (amber dot + glow) by `angle − π/2` (dot baseline 3 o'clock, knob
  rotation 0, scale hero 0.416/small 0.238). Knob box = bbox-diam × (sprite/112) since the
  259² sprite is pad-bigger than the 112px hero bbox. Sprites embedded as **BinaryData**
  (`Resources/prime_*.png`, added to `juce_add_binary_data`). Canvas positions came from a
  Blender camera-projection dump.
  - **Build:** `cmake -B build -S . ; cmake --build build --config Release --target Filter76_VST3`.
    Added **Standalone** to `FORMATS` (for screenshot verification, Gain-style) — drop it for
    release if VST3-only is wanted. Standalone exe locks while running → `Stop-Process -Name
    'AlteredAudio Filter 76'` before rebuild.
  - **Verify:** launch the Standalone via `Start-Process` (not `cmd /c start`), screenshot by
    `MainWindowHandle` (SetWindowPos topmost 0x0041 + CopyFromScreen).
- **Prime editor v2 — buttons + live screens DONE (2026-06-16).** `Prime76Editor` now also
  hosts the Light editor's `ResponseDisplay` (rect 58,97,1118,304), `LfoScope` (337,716,204,79,
  idx 0) and `EnvScope` (887,716,204,79), refreshed by a 30 Hz timer (display.refresh /
  lfoScope.repaint / envScope.push(analysis.envValue)). The live `ResponseDisplay` paints its
  own dark graph over the baked glass — fixes the screen-glass gradient that darkened to the
  right. **Option LEDs:** `OptionLeds` overlay (full-canvas, `hitTest` solid only over the
  button rows so it never blocks knobs) lights the selected filter-type(6)/slope(3)/mode(2) LED
  and sets the choice param on click. LED canvas coords: type x49 y{471,500,528,556,584,612};
  slope x1232 y{472,500,528}; mode x1232 y{584,611}. **Faceplate re-rendered with `_LED_`
  meshes hidden** so JUCE owns the LEDs.
  - ⚡ **Startup fix:** sprites were exported **16-bit** → faceplate was **19 MB**; converted to
    **8-bit** (faceplate 2.3 MB, knob 41 KB) and `filter76_export.py` now sets
    `color_depth='8'`. Standalone time-to-window ≈ **4.6 s** (the rest is the standalone audio-
    device scan, which the VST3 in a DAW avoids).
- **Faceplate right-edge / window size:** the render leaves a ~52 px @2x transparent margin on
  the **right** (plate opaque x 0..2809 of 2862; left/top/bottom flush — a mesh quirk, not
  touched per "don't touch design"). Fix: the editor is sized to the plate's TRUE opaque size
  **kW=1405, kH=900** (= 2810×1801 @2x ÷2) and `paint` draws only the opaque region 1:1
  (`g.drawImage(faceplate, 0,0,getW,getH, 0,0, 2810, 1801)`) — so the window **ends exactly at
  the plate edge**, no strip, knobs aligned (same 0.5 scale as the layout coords). VST3 editor =
  1405×900; the standalone adds ~28 px OS border in screenshots.
- **Amber sprite font + labels + readouts (2026-06-16).** Reused Gain 76's Michroma glyph
  renders (the Blender font): cropped A–Z + 0–9 . + − to a shared baseline band and 8-bit
  (`Resources/glyphs/*.png`, 18 MB → ~100 KB), pulled in via `file(GLOB PRIME_GLYPHS)` →
  BinaryData (loaded with `BinaryData::getNamedResource("A_png"...)`). `AmberGlyphs::drawString`
  lays glyphs out proportionally, centred, height-fitted. **Plate labels** drawn in
  `drawTypeLabels` (editor paint): filter type LP/HP/BP/NOTCH/PEAK/SHELF (left), 12/24/48 +
  ANALOG/CLEAN (right). **`ReadoutScreen`** (5, under the hero knobs at rects {215/406/596/787/978,
  599,63,22}) shows the function name when idle and the live value while the param changes (40-tick
  ≈1.3 s hold), polled in `timerCallback` via `param->convertFrom0to1(getValue())`. Value formats:
  FREQ int Hz, RES/DRIVE/OUT 1-dp, MIX int %. Text height fracs: labels 0.4, readout name 0.44,
  value 0.56 (−20% from first pass).
  - ⚠️ **Choice mapping:** `filterType` has **8** choices (LowPass,HighPass,BandPass,Notch,
    AllPass,Peak,LowShelf,HighShelf). The 6 plate buttons LP/HP/BP/NOTCH/PEAK/SHELF map to indices
    **{0,1,2,3,5,6}** (skip AllPass=4 & HighShelf=7), matching the Light editor's `kTypeMap`.
    `OptionLeds` takes an explicit index list + true choice count — don't assume contiguous
    0..n (that bug made BP→Notch, NOTCH→AllPass, etc.).
- **In-app Layout Editor (2026-06-16).** `LayoutEditor` overlay + `LabelText` components. Each
  text element (6 filter-type labels, SLOPE 12/24/48, MODE ANALOG/CLEAN, 5 readouts) is a movable
  component. **Right-click anywhere → "Layout edit mode"** (handled by `Prime76Editor::mouseDown`
  via `addMouseListener(this,true)`); click a box to select, **arrow keys nudge (Shift=10px)**,
  right-click → "Edit text…" / "Save layout now", Esc exits. Positions+text persist to
  **`Documents\AlteredAudio\Filter 76\prime_layout.json`** (loaded at ctor after
  `applyDefaultPositions()`; `resized()` must NOT reset label/readout bounds). **To bake final
  positions:** read that JSON and update `applyDefaultPositions()` defaults. (Built because the
  filter-type labels measured centered but the user wanted hands-on px control.)
- **Left mod plates (2026-06-16).** Same pattern as filter type: **fixed LED + click via
  `OptionLeds`**, **movable TEXT via `LabelText`** (only the text is selectable/nudgeable in the
  LayoutEditor — ids MOD_LFOA/MOD_LFOB/MOD_EF, DST_FREQ/DST_RES/DST_DRIVE). `OptionLeds` extended
  with `toggleOff` (source: click-active → OFF) and `gate` (dest LED lit only while gate≠0).
  Source **LFO A/LFO B/EF** → `flt_mod_source` idx{1,2,3}, toggleOff; dest **FREQ/RESO/DRIVE** →
  `flt_mod_dest` idx{0,1,2}, gate=source. LED rows x49 y{671,699,727}/{760,788,816}; text x64 w72
  h22 (matches −20% labels). NOTE: dest has no DSP "off" — mod is effectively off when source=OFF.
- **Tweaks (2026-06-16 pm):** (1) saved layout positions baked into `applyDefaultPositions`
  (JSON still loads on top). (2) `fitModLabels()` shrinks mod label boxes to their text width
  (uses `AmberGlyphs::textWidth`, keeps centre) so only text is selected in the editor.
  (3) Visualizer scale now **+24/−24 dB** (`ResponseDisplay::kDbTop/kDbBot` in AuroraFilterEditor.h;
  grid lines {18,12,6,-6,-12,-18}+0, labels {24,12,0,-12,-24}). (4) Faceplate re-rendered from
  current Blender. (5) **`PeakReadout`** on the OutMeter_Label screen (1232,93,102,22): L/R peak
  dB, peak-HOLD, resets after ~45 timer ticks of silence; reads `analysis.peakL/R.exchange(0)`.
  (6) **SHELF cycles LowShelf(6)↔HighShelf(7)** — `OptionLeds` idx is now `vector<vector<int>>`
  (per-option cycle values); SHELF option = {6,7}, lit if cur∈{6,7}.
- **Tweaks (2026-06-16 eve):** GAIN/OUT swapped — hero slot (1010) = `filter_gain`, small slot
  (1150) = `filter_output`; hero readout renamed GAIN. Readouts expanded to **12**: 5 hero +
  7 bottom (AMOUNT/RATE/DEPTH/PHASE/ATTACK/RELEASE/SENS) on the bottom-knob screen slots
  (215/597/694/788/1131/1207/1283 @ y795, 63×22). Readout text −20% (ReadoutScreen fracs name 0.35
  / value 0.45). Faceplate re-rendered for the slope plate. NOTE: small OUT knob has no readout
  slot (unlabeled). Startup ≈4.8s is the **standalone audio-device scan** (VST3 in a DAW is fast).
- **Tweaks (2026-06-16 late):** (1) RATE readout (`readouts[6]`) is clickable — `syncParam` =
  `flt_lfoa_sync`; click toggles tempo-sync, shows **"SYNC"** when on. (2) `LfoScope` waveform
  amplitude now scales by `flt_lfoa_depth` (DEPTH knob). (3) ResponseDisplay: dropped the ±24 dB
  edge labels (now 12/0/-12) and the "Hz" axis title (rightmost freq label "20k" stands alone).
- **LFO A/B rebind + sync division (2026-06-16).** `rebindLfo(idx,sync)` recreates the RATE/DEPTH/
  PHASE knob attachments + readouts[6/7/8] params + scope index based on `flt_mod_source` (==2 → LFO
  B, else A) and that LFO's sync flag; triggered in timerCallback when `boundLfo`/`boundLfoSync`
  change (mirrors Light editor). When synced, RATE knob binds to `flt_lfoX_div` and the RATE readout
  shows the division (aurora::divNames) while turning, "SYNC" when idle. `LfoScope` amplitude scales
  by depth. NOTE: **slope (24/48) only affects LP/HP** by DSP design (`planSections` isLPHP) — BP/
  Notch/Peak/Shelf are single-biquad; could dim SLOPE LEDs when type∉{LP,HP}.
- **Header + footer + tweaks (2026-06-16 night):** main display "dB" corner → **"24"**.
  **`FooterStrip`** (info line, dark plate y851): SYSTEM LED + SAMPLE RATE/OVERSAMPLING/LATENCY/CPU/
  SIGNAL PATH/v1.0.9 (amber mono). **Header line** on the new top strip `Footer_Strip.001` (y26):
  `buildHeader()` — preset `<`/name/`>`/SAVE (INIT + `PresetManager` user presets, `initState` for
  INIT), A/B compare (apvts state slots), PWR (filter_bypass), OS cycle (filter_oversampling), MIX
  readout. The user's header/slope/freq Blender edits were **unsaved** — had to `wm.save_mainfile()`
  via socket before the headless render saw them; faceplate re-rendered to bake the header strip.
  Export samples lowered to **64** (faster renders / less CPU; each faceplate render maxes all cores
  ~5-6 min — that's the "Blender 90% CPU"). Viewport shading is SOLID (not the CPU cause).
- **Glyph-font header/footer + phase (2026-06-16):** `AmberGlyphs::drawString`/`textWidth` now
  handle **spaces** (26px advance). `FooterStrip::paint` rewritten to use the glyph font (sample
  rate/oversampling/latency/cpu/signal-path/version; % and / aren't in the font so omitted).
  **`GlyphLnF`** renders the header buttons + labels (hName/hMix) in the glyph font, with hand-drawn
  chevrons for `<`/`>`; set on all 7 header buttons + 2 labels (cleared in dtor). `LfoScope` now also
  offsets the waveform by **PHASE** (`flt_lfoX_phase`/360). NOTE: glyph font lacks % / / lowercase
  (a-z map to caps) — keep footer/header values unit-light.
- **LFO wave-select screens (2026-06-16).** 4 new screens below the LFO scope (Blender:
  `Mod_Plate_6.001-.004` in 07_Modulation, glass x384/413/442/471 y809). `WaveSelect` component
  (bounds 383,807,113,16) draws sine/triangle/square/random **symbols** (JUCE paths), selected lit
  amber; click a cell → `flt_lfoX_wave`. `param` rebound A/B in `rebindLfo`. Nudgeable (id
  WAVE_SELECT). Faceplate re-rendered to bake the 4 recesses.
- **Footer/LFO polish (2026-06-16):** footer redrawn procedural-style (dim small labels + bright
  values via `g.setOpacity`, glyph font). `LfoScope`: phase cursor now sweeps the **full** window
  (`ph*w`, was `ph/3`); RANDOM rewritten as a stable sample&hold (`rnd[16]`, 3 steps/cycle) instead
  of the buggy held logic. Wave-select symbols shrunk (~0.64 of cell).
- **Screen scan-lines + chroma + LFO polish (2026-06-16):** the faceplate's display/LFO/Env glass
  have baked **amber scan-lines**; the JUCE screens now draw a **semi-transparent** background so
  they blend through subtly — `ResponseDisplay` bg gradient alpha `0xEC`, `LfoScope`/`EnvScope`
  `graphBg.withAlpha(0.92f)` (lower alpha = more visible lines). `ResponseDisplay` curve has a
  **chroma-aberration motion trail** (red/cyan lagging ghosts; `kChromaCurve`=false to revert,
  `kCurveStride`/`kCurveHist` tune speed/length). `LfoScope` cursor only sweeps while that LFO is
  the active mod source (else stops).
- **Prime is now a distinct plugin (2026-06-16):** `PRODUCT_NAME` = **"Filter76 Prime"**,
  `PLUGIN_CODE` = **Fp76** (was AFlt) so it coexists with the Light build without VST3 ID collision.
  Installed to `C:\Program Files\Common Files\VST3\Filter76 Prime.vst3` (system; needs elevated copy
  via `Start-Process -Verb RunAs`). ⚠️ if rebuilding while a DAW has it loaded, the .vst3 is locked.
- **Prime editor — remaining (task #8):** live output meters (`VerticalMeter` + a `meter_strip`
  sprite), preset header (prev/next/save + A/B), mod source/dest dropdowns. Ideally factor a
  **shared AlteredAudio UI module** with Gain 76.

---

## 8. Security
The AlteredAudio git remote URL embeds a **GitHub PAT in plaintext** — rotate it and
switch to a credential helper / SSH.
