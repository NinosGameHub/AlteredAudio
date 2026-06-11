# Altered Audio — Developer Handoff

Maps each **designed surface** (HTML mockup in this design system) to the exact colors, dimensions, and JUCE source files to implement against. Use this with the live mockup (`ui_kits/plugin/index.html`) open as the visual reference.

Everything below is already encoded as CSS custom properties in `tokens/` and as React mockups in `components/` + `ui_kits/plugin/`. The hex values are the canonical `AaColor` values from `source/AaLookAndFeel.h` — match those in C++.

---

## How to use this package

**A. As living visual reference (recommended first step).**
Open any mockup directly in a browser — no build step:
- `ui_kits/plugin/index.html` — the modular effects-rack window (mirrors current JUCE source).
- `ui_kits/mastering/index.html` — the mastering dashboard.
- `components/**/**.card.html` and `guidelines/*.card.html` — component/foundation specimens.

Use them side-by-side with the C++ you're writing; every measurement and color below is what they render.

**B. To reuse the components in a new HTML prototype.**
Link the one stylesheet, load the compiled bundle, read components off the namespace:
```html
<link rel="stylesheet" href="styles.css">
<script src="https://unpkg.com/react@18.3.1/umd/react.development.js" …></script>
<script src="https://unpkg.com/react-dom@18.3.1/umd/react-dom.development.js" …></script>
<script src="https://unpkg.com/@babel/standalone@7.29.0/babel.min.js" …></script>
<script src="_ds_bundle.js"></script>            <!-- compiled component library -->
<script type="text/babel">
  const { Knob, ToggleSwitch, Select, Button, Card, Badge,
          ModuleTile, CRTDisplay, Meter, EQCurve, ModMatrix }
        = window.AlteredAudioDesignSystem_3ace71;   // <-- the namespace
  // …render with React…
</script>
```
`styles.css` `@import`s every token file; `_ds_bundle.js` is generated from `components/**` (do not hand-edit). Component props are documented in each `*.d.ts` and `*.prompt.md`.

**C. To implement in the JUCE/C++ plugin.**
This is the primary path. The plugin's look is defined in `source/AaLookAndFeel.h` (palette + `drawRotarySlider` / `drawToggleButton` / `drawComboBox`) and laid out in `source/PluginEditor.cpp`, `ModulePanels.cpp`, `ModuleTileList.cpp`, `CRTDisplay.cpp`. The §-by-§ map below pairs each designed surface with the file to edit and the exact spec — colors are the `AaColor` constants, so match the hex table in §0.

---

## 0. Color reference (AaColor)

| Token | Hex | Role |
|---|---|---|
| `bg` | `#E7E2D6` | window canvas |
| `surface` | `#EDE8DC` | header, panel header, cards |
| `surfaceAlt` | `#D9D4C8` | pressed / sunken |
| `border` | `#B2ADA1` | hairlines (1px) |
| `textPrimary` | `#1C1815` | labels, values |
| `textSecond` | `#7A746E` | secondary labels |
| `catFilterEQ` | `#3A6694` | Filter, EQ |
| `catTime` | `#7A5438` | Delay, Reverb |
| `catDynamics` | `#C4621D` | Wave, Comp, Clip, Trans, Lim, Gate |
| `catModulation` | `#AA8B10` | Chorus, Flanger, Phaser |
| `catSpatial` | `#5E8480` | Spatial |
| `green` | `#4CA84C` | active / engaged |
| `inactive` | `#B2ADA1` | bypassed |
| CRT glass | `#060504` | spectrum / meter background |
| CRT amber | `#E4A210` | spectrum bars, freq labels |
| knob body | `#E4DFD0` | flat disc fill |
| knob line | `#1C1815` | indicator line |
| knob dots | `#8C8680` | range markers |

Font: host system font (SF Pro on the Apple target). Numerics (knob values, CRT/meter labels, tile numbers) use the **monospaced** system font. Labels are UPPERCASE.

---

## 1. Window shell — `PluginEditor.cpp`

Already implemented. Reference dimensions:
- Window **1000 × 640**, non-resizable.
- Header **40px** (`kHeaderH`), full width, `surface` fill, 1px `border` bottom.
- Left tile list **160px** (`kListW`), 1px `border` divider on its right.
- CRT **180px** (`kCRTH`), spans `kListW`→right, below header; 1px `border` below it.
- Detail panel: `{ kListW, kHeaderH+kCRTH+1 } → bottom-right`.

---

## 2. Header bar (backlog #1) — *to build in `PluginEditor`/new `HeaderBar`*

Mockup: `PluginHeader` in `ui_kits/plugin/index.html`. Layout, left→right:
- **Left block** (width = `kListW`, divider on right): brand dot (7px, `catDynamics`) + `ALTERED AUDIO` wordmark — 13px bold, letter-spacing ~0.10em, uppercase, `textPrimary`.
- **Preset transport**: ◂ button · preset name box (150px min, 24px tall, `bg` fill, 1px border, radius 4px, folder glyph + name 12px) · ▸ button.
- **Channel mode**: 2-segment toggle `ST | M/S`, 24px tall, selected segment = `catDynamics` fill / white text, others `bg` / `textSecond`. Mono 9px.
- **I/O meters** (right-aligned): `IN` label + 2 horizontal meters, `OUT` label + 2 meters. See §4 for meter spec; segments=12, ~42px long, 4px thick.
- **MOD button**: opens the modulation matrix (§6). Icon + "MOD".
- **Global bypass**: power icon; when engaged, tint `accent-tint`, else `accent-primary`.

---

## 3. Tile list — `ModuleTileList.cpp` (implemented)

Mockup: `ModuleTile`. Per row (~40px tall, 15 rows):
- Always-on **4px category strip** at the left edge (`moduleColor(i)`).
- Mono **chain number** top-left (8.5px, `textSecond` @ 0.7 alpha).
- **Centered module name**, 10px bold, uppercase; `textPrimary`, or the category color when selected.
- **LED dot** right (≈6.4px): category color when engaged, `border`/`inactive` when bypassed.
- **Selected**: row fills with `cat.withAlpha(0.14)` (trimmed left of the strip).
- Bottom separator `border @ 0.5` except last row. Drag-to-reorder already wired.

---

## 4. Meters — segmented, on CRT-black (`Meter` component)

Used for header I/O **and** the GR strips (§5). Spec:
- Track on `#060504`, 2px inner padding, radius 3px.
- Segments are flex cells, 2px gap, radius 1px.
- **Level mode**: lit segments green→amber→orange bottom-up — `green` below 78%, `amber` 78–92%, `catDynamics` above 92%. Unlit = `surfaceAlt` @ 0.6.
- **GR mode**: lit `catDynamics`, filling from the top down.

---

## 5. Compressor / Limiter GR meters (backlog #3) — *to build in `CompressorPanel` / `LimiterPanel`*

Mockup: `GRStrip` in `ui_kits/plugin/index.html`, placed above the knob rows.
- Horizontal bar, 16px tall, on `#060504`, radius 3px.
- Fill anchored at the **right edge**, growing left, `linear-gradient(90deg, amber, catDynamics)`.
- dB scale gridlines + labels below: Comp `0…-18`, Limiter `0…-12`.
- Held-peak tick: 1.5px white line @ 0.85 alpha, slow decay.
- Header reads `GAIN REDUCTION   -X.X dB` (value in `catDynamics`, mono).

---

## 6. EQ curve display (backlog #2) — *to build in `EQPanel`*

Mockup: `EQCurve` component, placed above the 8-band knob grid.
- Graph on `#060504`. **Log frequency** X axis (20 Hz–20 kHz), **±18 dB** Y axis.
- Grid: vertical at 20/50/100/200/500/1k/2k/5k/10k/20k; horizontal at ±12/±6/0; the **0 dB** line brighter (amber @ ~0.32).
- Faint **amber spectrum** bars behind (≈0.16 alpha).
- **Summed response curve** in `catFilterEQ`, 2px, with a fill below at ~0.14 alpha.
- One **draggable handle per band** (≈7px), numbered, in the band/category color; greyed when band off. Drag X = freq, Y = gain.
- Freq labels along the bottom in amber mono.
- Keep the curve and the 8 band knobs reading the same band state.

---

## 7. Modulation matrix (backlog #5) — *to build as a `ModMatrix` view/overlay*

Mockup: `ModMatrix` component, opened from the header MOD button.
- Grid: **sources = rows** (LFO 1/2, ENV, MACRO 1–N), **destinations = columns** (any modulatable param).
- Source row label: 8px color swatch (source color) + name; on `surfaceAlt`.
- Destination header: rotated (vertical) uppercase label, 9px.
- Each **cell** (~38–40px): center line; a **bipolar depth bar** anchored at center — positive fills up in the **source's color**, negative fills down in `textSecond`. Empty cells show a faint `·`.
- Interaction: drag a cell vertically to set depth (−1…+1); double-click clears. Hover shows the % value.

---

## 8. Knob modulation arcs + right-click menu — *enhancement to the rotary*

Mockup: `Knob` `mods` prop + `KnobMenu` in `ui_kits/plugin/index.html`.
- Base knob (`drawRotarySlider`): flat `#E4DFD0` disc, radius `0.40 × min(w,h)`, 1px `border` outline; near-black indicator line `0.20r → 0.86r`, 2.5px; range dots at the 7- and 5-o'clock ends.
- Optional **value ring** (`catColor`) just outside the disc.
- **Modulation arcs**: for each active routing, a thin (≈1.6px) arc starting at the current value angle, extending by the routing's bipolar depth, in the **source color**, with a small end-cap dot. Concentric (3px apart) when multiple sources target one knob.
- **Right-click**: context menu "ASSIGN MODULATION" listing mod sources (color swatch + name) + "Clear modulation".

---

## 9. Spectrum analyzer — `CRTDisplay.cpp` (visual done; FFT is the open task)

Mockup: `CRTDisplay` (plugin window) and the `Spectrum` block in `ui_kits/mastering/index.html`.
- **Plugin window (amber-on-black):** `#060504` glass, horizontal grid, scan-lines, a single glowing amber **spectral line** (`#E4A210`) with a soft gradient fill beneath, a sweeping scan highlight and vignette. (Earlier bar version retired in favor of the line.)
- **Mastering dashboard (ink-on-cream):** the same line treatment as a dark `#2A2520` line + faint grey vertical "haze" over a cream panel, log-frequency axis 20 Hz–20 kHz.
- **Open work is code only**: feed a real FFT magnitude curve (log-spaced bins, 20 Hz–20 kHz, smoothed/decayed) into the line. No visual change needed.

---

## 10. Mastering dashboard (new surface) — `ui_kits/mastering/index.html`

A single-window mastering-assistant layout (1520×1000), branded Altered Audio. Not in JUCE source yet — build as its own editor if/when the product ships a mastering view. Section map (all colors from §0; ink lines = `#2A2520`, accent = `catDynamics #C4621D`):
- **Top bar (60px):** logo tile + `ALTERED AUDIO` + `MASTER` (orange) + version; centered preset transport; A/B (active = orange fill); Compare; undo/redo; Bypass.
- **Analyzer row:** `INPUT` panel (L/R 30-segment vertical meters + dB scale `0 … -60`, `-∞` rest) · spectrum panel (§9) · `OUTPUT` panel.
- **Module cards (6, equal grid):** each = `panel` (`#F4F1EA`, 1px border, radius 10) with a 19px circular power toggle, uppercase title, a centered mini-viz, a value + label, and a Details dropdown. Viz: Dynamics = 3/4 **arc gauge** (orange, % center); EQ = response line + 4 category-colored band dots; Saturation = transfer wave; Imaging = polar half-arc; Stereo Link = two overlapping circles; Limiter = transfer curve.
- **Bottom row:** Loudness (big arc gauge `-14.0 LUFS` + Short Term / Momentary / Range / True Peak rows) · Loudness History (area graph, time axis `-60s…0s`) · Correlation (horizontal bar, `-1 … +1`) + Stereo Field (goniometer scatter).
- **Sidechain strip (70px):** power toggle + Source / Low Cut / High Cut / Listen selectors + a mini sidechain waveform + Sample Rate / Oversampling readouts + settings gear.

All graphics are inline SVG using the design-system tokens; reuse the `Meter`, gauge, and curve patterns as JUCE `Component`s.

---

## Implementation order (suggested)

1. Header bar (#1) — ties the window together, reuses the Meter.
2. GR meters (#3) — small, high value, reuses the Meter.
3. EQ curve (#6/#2) — self-contained in the EQ panel.
4. Spectrum real FFT (#4) — DSP wiring, no UI change.
5. Modulation matrix (#7/#5) + knob arcs (#8) — the largest feature; build the routing model first, then the matrix view, then the knob overlays read from it.
6. Mastering dashboard (§10) — only if the product adds a mastering view; otherwise a reference layout.

Pair each with its mockup file and the token table above; all measurements there are the spec.
