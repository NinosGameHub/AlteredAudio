# AlteredAudio — UI Design TODO
### What needs to be designed before it can be built

This document covers every UI component that still needs visual design. DSP and parameter infrastructure are complete. The shell (1000×640, left panel, CRT area, detail panel) exists but most of it is functional-only with no design polish.

---

## Current State Summary

| Area | Status |
|---|---|
| Window shell (1000×640, layout zones) | Built, no polish |
| AaLookAndFeel — knobs, toggles, combos | Built, Braun-style |
| Color palette | Locked (warm cream / category colors) |
| ModuleTileList (left sidebar) | Built, functional |
| CRTDisplay (amber spectrum) | Built, FAKE (not real FFT) |
| All 15 module detail panels | Built, knob-rows only — no visualizers |
| Modulation Matrix | DSP complete, ZERO UI |
| Preset browser | DSP complete, ZERO UI |
| Global controls (header bar) | Title label only |
| Metering (GR, I/O levels) | DSP data available, ZERO UI |

---

## 1. Header Bar (40px strip, full width)

Currently: just the text "ALTERED AUDIO".

**Needs design for:**
- Plugin name / logo lockup (left side)
- **Preset display** — current preset name, prev/next arrows, save button (center or right)
- **Channel mode selector** — Stereo / Mono / Mid-Side toggle (parameter exists: `channelMode`)
- **Global I/O level meters** — a pair of thin input and output level bars (pre- and post-chain)
- **Global bypass** button

This is the most impactful single surface — it frames the whole plugin.

---

## 2. ModuleTileList (left sidebar, 160px wide)

Built but needs design review:

- Tile height and spacing — currently uniform rows, could benefit from proportional spacing
- **Category color LED** placement and size (currently functional, may need sizing pass)
- **Active/bypass visual state** — LED on/off, tile dimming when bypassed
- **Selected tile** highlight (which tile's panel is shown on the right)
- **Drag handle** visual affordance — currently it's drag-anywhere on the tile; should there be an explicit grip area?
- **Module name truncation** — "TransientShaper" won't fit at 160px; needs a label strategy

---

## 3. CRTDisplay — Real FFT Connection

Currently the 32-bar amber spectrum shows animated fake movement. It needs to be connected to actual FFT data from the audio thread.

**Design decisions needed:**
- Peak hold bars? (decay rate, visual style)
- dB range shown (e.g. -60 to 0 dBFS)
- Should it show pre-chain, post-chain, or selectable?
- Freeze / pause button?
- The existing scan-line overlay, amber color, and bar style can stay — just needs real data

This is a medium-complexity wiring task once the design is settled.

---

## 4. Module Detail Panels — Visualizers

All 15 panels exist with knob rows. The panels that need additional visualizers:

### 4.1 EQ Panel (highest priority)
The 8-band EQ has 8 rows of knobs (type, freq, Q, gain per band). It **must** have an EQ curve display.

- Frequency response curve drawn over a log-scale frequency axis (20Hz–20kHz)
- Overlaid on a dark background sub-panel above the knob rows
- Each band's contribution shown in its category color (or all in a single accent color)
- Curve updates in real time as knobs move
- Bands with `enabled=false` should appear dimmed or absent from the curve

### 4.2 Compressor Panel
`gainReductionDb` is exposed by CompressorModule but nothing displays it.

- **GR meter** — vertical bar showing current gain reduction (0 to −24 dB range)
- Should sit near the threshold/ratio controls
- Color: burnt orange (`#C4621D`) for active GR, warm grey for idle
- Optional: threshold line drawn on meter

### 4.3 Limiter Panel
Same as compressor — `gainReductionDb` available, needs a GR meter.

### 4.4 Gate Panel
- Threshold indicator line (or meter showing how far below threshold the signal is)
- GR amount bar (how much attenuation is being applied)

### 4.5 Waveshaper Panel
- **Transfer curve** — a small XY graph showing the input-to-output nonlinearity for the selected algorithm at the current drive setting
- X axis = input level, Y axis = output level, drawn as a curve
- Useful for showing the difference between SoftClip, HardClip, TanhSat, BitCrush, FoldBack visually

### 4.6 Clipper Panel
Same as Waveshaper — transfer curve showing the current mode's clipping characteristic.

### 4.7 Transient Shaper Panel
- Small envelope display showing fast envelope (attack detection) vs slow envelope (sustain)
- Could be simplified to two animated bars: attack energy indicator, sustain energy indicator

### 4.8 Modulation Panels (Chorus, Flanger, Phaser)
All three have an LFO rate parameter but no visual feedback of the LFO.

- Small LFO shape display — shows current waveform shape (sine only for these modules)
- Animates at the current rate (or shows a static wave at 30fps — just phase-advancing)
- Rate and depth can be read off the display visually

---

## 5. Modulation Matrix — Needs Full UI Design

The infrastructure is complete: LFO1, LFO2, EnvelopeFollower, 16-connection routing matrix. Zero UI exists.

**This is the largest design task.** Options for where it lives:

**Option A — Dedicated panel** (accessed via a button in the header or a tab below the tile list)
- Full-screen overlay or a drawer that slides in
- Shows a 16-slot connection list: source → target → depth
- Each row: Source dropdown (LFO1 / LFO2 / Envelope), Target dropdown (any parameter), Depth slider (−1 to +1)
- LFO shape + rate mini-display per LFO source

**Option B — Per-knob assignment** (right-click a knob → assign modulation)
- No dedicated matrix panel
- Right-click any knob → "Assign modulation" → pick source + set depth
- Assigned knobs show a colored arc (see section 7)

**Option C — Both** (Option A for overview, Option B for quick assignment)

**Design deliverable:** mockup of at least Option A (the matrix panel view). This is the feature that differentiates the plugin from a static effects chain.

---

## 6. Preset Browser — Needs Full UI Design

APVTS state serialization works. No UI exists.

**Needs design for:**
- Where it appears (overlay panel, sidebar, header dropdown?)
- Preset list — factory presets vs user presets (separate folders or tagged)
- Save / Load / Delete / Rename controls
- Per-module preset save (save just one module's settings independently)
- Visual treatment: consistent with the warm analog palette

---

## 7. Knob Modulation Depth Arcs

The roadmap specifies: "modulation depth shown as coloured arc around knob."

- When a knob has a modulation connection, draw an arc outside the normal range arc
- Color = modulation source color (LFO1 = one color, LFO2 = another, Envelope = another)
- Arc width proportional to modulation depth
- This requires knowing which knobs are modulation targets — needs ModulationMatrix UI to be wired first

**Design deliverable:** how the arc visually integrates with the existing flat Braun knob style (flat cream disc, range dots at 7-o'clock / 5-o'clock).

---

## 8. Knob Right-Click Context Menu

No right-click behavior exists on any knob.

**Design for:**
- "Reset to default" — restores parameter to its declared default value
- "Enter value" — shows a text input popup for precise entry
- "Assign modulation" — opens source picker for the Modulation Matrix (links to section 5)
- "Remove modulation" — clears connection if one exists

JUCE supports this via `Slider::addMouseListener` + popupMenu. Design the menu visual style (consistent with the LookAndFeel palette).

---

## 9. Bypass State — Visual Polish

When a module is bypassed, the detail panel should feel "off." Currently there's no dim/grey-out behavior.

- Bypass = all knobs in the panel reduce opacity / render in greyed-out palette
- "Active" toggle button in each panel drives this
- The corresponding tile in the left sidebar dims its LED

---

## Priority Order

| # | Item | Why now |
|---|---|---|
| 1 | Header bar (preset + channel mode + meters) | Frames everything, needed before first demo |
| 2 | EQ curve display | Most expected feature in any EQ |
| 3 | Compressor + Limiter GR meter | Core dynamics feedback |
| 4 | CRT real FFT connection | CRT is prominent, currently misleading |
| 5 | Modulation Matrix UI (Option A mockup) | Largest feature gap |
| 6 | Waveshaper + Clipper transfer curve | Differentiating visual |
| 7 | Preset browser | Needed for release |
| 8 | Knob mod arcs + right-click menu | Depends on Matrix being wired |
| 9 | Bypass visual polish | Polish pass |
| 10 | LFO shape displays (Chorus/Flanger/Phaser) | Nice-to-have |

---

## What Does NOT Need Design

- The LookAndFeel knob/toggle/combo style — locked and implemented
- The color palette — locked
- The 1000×640 window size — fixed
- The 160px left tile list width — fixed
- The 40px header height — fixed (though content needs design)
- The 180px CRT height — fixed
- All DSP modules — complete, no changes needed
