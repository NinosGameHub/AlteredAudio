# Filter 76 — Complete Handoff (Light + Prime)

_Last updated 2026-06-15. Read this first. Covers both the shipping procedural
build (**Light**) and the in‑progress Blender‑rendered redesign (**Prime**)._

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
- **Prime:** Blender faceplate + knob/meter/screen blockout laid out against the Light
  reference; render fills the whole faceplate at full quality. **Next:**
  1. Finish arranging the Prime layout in Blender (knobs, screens, meters, response
     display) against the reference.
  2. Render the sprite assets (faceplate, knob, meter, screens, glyphs) — F12.
  3. Build the Prime JUCE editor reusing the Gain 76 patterns (sprite `loadAsset`,
     `drawRotarySlider`, `VerticalMeter`, amber glyph `drawAmberReadout`/`drawAmberWord`,
     tooltip LnF). Ideally factor a **shared AlteredAudio UI module** so Gain 76 and
     Filter 76 Prime share this code.
  4. Embed assets as **BinaryData** for a portable VST3 (avoid absolute‑path loading).

---

## 8. Security
The AlteredAudio git remote URL embeds a **GitHub PAT in plaintext** — rotate it and
switch to a credential helper / SSH.
