# EQ VST Feature & UI Reference

A working reference for building a modern parametric EQ, modeled on what FabFilter Pro-Q 4, Three-Body Kirchhoff-EQ, SSL Native X-EQ 2, and similar pro tools actually do. Split into two parts: **what features they have** (the DSP/feature surface) and **how they look & react** (the UI/UX contract). Build order suggestions at the end.

---

## Part 1 — Feature Surface

### 1.1 Bands

The core unit. Each band is an independent filter with its own parameters.

- **Band count.** Pro-Q 4 goes up to 24 bands; Kirchhoff goes up to 32. For a first build, 8–16 is plenty and keeps the curve math cheap. Bands are added on demand, not all instantiated up front.
- **Per-band parameters:** frequency (20 Hz–20 kHz, log scale), gain (typically ±18 to ±30 dB), Q / bandwidth, filter shape, enabled/bypass, channel routing (stereo / L / R / mid / side), and slope.
- **Dynamic toggle** per band (see 1.3).

### 1.2 Filter shapes

The minimum useful set, then the "pro" extras.

Core shapes everyone has: **Bell** (peak/dip), **Low Shelf**, **High Shelf**, **Low Cut / High-Pass**, **High Cut / Low-Pass**, **Notch**, **Band Pass**.

Pro extras worth knowing:
- **Tilt Shelf** — tilts the whole spectrum around a pivot point.
- **Flat Tilt** — a flat tilting correction curve across the whole audible range (Pro-Q 4 addition).
- **All-Pass** — changes phase without changing magnitude; used for phase alignment.

Pro-Q 4 ships roughly 9 shapes; Kirchhoff ships ~15 filter types **plus** 32 vintage hardware emulations (modeled on Neve, Pultec, SSL units). The vintage models are a differentiator, not a baseline requirement.

### 1.3 Slope (filter order)

How steeply a filter rolls off, in dB/octave.

- Classic stepped slopes: 6, 12, 24, 48 dB/oct.
- Modern EQs offer **continuously variable slope**. Kirchhoff goes from 0 dB/oct up to **96 dB/oct** and supports fractional slopes — e.g. an 8.125 dB/oct high-pass is valid. Bell/notch range from 12 up to 96 dB/oct; shelves from 6 up to 96.
- This is a real selling point and not trivial to implement — variable-order filter design is more than swapping coefficients.

### 1.4 Dynamic EQ

A band whose gain reacts to input level — like a single-band compressor sitting on that frequency.

- Any Bell or Shelf band can be made dynamic. It moves toward a target gain only when the signal at that frequency crosses a threshold.
- Controls: threshold, attack, release, range (how far the gain is allowed to move). Pro-Q has an Auto mode that sets threshold/attack/release from the incoming audio; an expand button reveals manual controls.
- Kirchhoff's dynamics are more granular: **dual-threshold** behaviour (define what happens above *and* below a threshold), independent ratios per side, and both downward compression and upward expansion. This is the deepest dynamics section on the market and it's what its fans cite over Pro-Q.

### 1.5 Spectral dynamics (Pro-Q 4's new trick)

A step beyond dynamic EQ. Instead of moving the gain of the *whole band*, it triggers on **specific frequencies inside the band** that cross threshold and leaves the rest untouched. Think of a band that auto-spawns narrow dynamic sub-bands where problems appear.

- Key control: **Spectral Density** — low values = wide trigger regions, high values = narrow/surgical.
- **Spectral Tilt** — optionally applies a ~3 dB/oct tilt to the trigger signal so highs trigger slightly more than lows (on by default for new spectral bands).
- Good for taming variable resonances (sibilance, ringing) more transparently than a static notch.

### 1.6 Phase modes

How the EQ treats phase, which affects transients and CPU/latency.

- **Zero Latency** (minimum phase) — standard IIR, no latency, introduces phase shift around bands. Default for tracking/mixing.
- **Natural Phase** (Pro-Q term) — minimizes the audible phase artifacts of analog-style EQ while staying low-latency.
- **Linear Phase** — FIR-based, no phase shift, but adds latency and pre-ringing. Has resolution settings (Low → Maximum); higher = more latency. Used for mastering / parallel work.
- Kirchhoff adds **Minimum / Analog / Mix / Linear** options, where Mix preserves low-end transients while linearizing highs.

### 1.7 Stereo / spatial processing

Per-band channel routing: each band can act on **Stereo, Left, Right, Mid, or Side** independently. Essential for mid/side mastering moves (e.g. widen highs on sides, tighten lows in mid).

### 1.8 Character / saturation

Optional coloration on top of clean EQ.

- Pro-Q 4 adds **Character** modes — Clean / Subtle / Warm — a tasteful softening rather than full analog emulation.
- Kirchhoff leans on its 32 vintage models for color instead.
- A first build can skip this entirely and stay clean.

### 1.9 Spectrum analyzer

The real-time FFT display behind the curve. This is the single most important visual element.

- **Pre-EQ and Post-EQ** spectra, plus optionally an **external** spectrum from another instance.
- Adjustable: range, **speed** (release rate of the falling bars), resolution (FFT size — 4096-point Hann window is a common, sane default), and **tilt**.
- **Tilt** rotates the displayed spectrum around ~1 kHz by a dB/oct slope. Pro-Q's default of **4.5 dB/oct** makes the spectrum look "natural" — roughly matching perceived loudness so a balanced mix reads as roughly flat. This is a display-only cosmetic, but users expect it.
- **Freeze** — stops the spectrum falling and accumulates a max-hold over time.
- **Collision detection** — highlights where two tracks/bands overlap in frequency.

### 1.10 Workflow / "magic" features

These are what make a clone feel pro rather than generic:

- **Spectrum Grab** — hover over the live spectrum, it freezes, and you grab a visible peak and drag it down; a Bell band is auto-created with an auto-chosen Q at that frequency. Works best with Post-EQ analyzer active.
- **EQ Sketch** — draw a curve freehand in one gesture and the plugin fits bands to it.
- **EQ Match** — analyze a reference spectrum (another instance or external signal) and auto-generate a curve to match it.
- **Auto Gain** — compensates output level so louder ≠ "better" when judging.
- **Instance list** — see and route between other instances of the plugin in the session.
- **Solo a band** — temporarily hear just that band's frequency range.
- **Piano display** — overlay note names on the frequency axis.

### 1.11 Output / utility

Output gain, global bypass, A/B compare, undo/redo, MIDI Learn, preset load/save, oversampling (Kirchhoff is 2x; Pro-Q offers up to 5 linear-phase quality levels), and GPU-accelerated drawing for the analyzer.

---

## Part 2 — How They Look & React (the UI contract)

This is the part that makes or breaks the "feel." Producers have deeply ingrained muscle memory from Pro-Q; matching these conventions is more valuable than inventing new ones.

### 2.1 Layout

- **Dominant central display**: a large interactive graph. X axis = frequency (logarithmic, 20 Hz on the left → 20 kHz on the right). Y axis = gain (linear in dB, 0 in the middle). The grid lines are labeled (e.g. 100, 1k, 10k on X; ±6/±12 dB on Y).
- The real-time spectrum analyzer is drawn **behind** the curve, semi-transparent.
- The **EQ curve** (the sum of all bands) is drawn as a bright continuous line over the analyzer. Often each band also draws its **own colored sub-curve** so you can see individual contributions vs the composite.
- A **bottom bar** holds global controls: analyzer settings, processing mode, output gain, bypass, undo/redo, A/B.
- A **band control strip / pop-up**: when a band is selected, knobs for Frequency, Q, Gain, shape, slope, channel, and dynamics appear (either docked at the bottom or as a floating panel near the node).
- Display range presets matter: mastering engineers want **3 dB and 6 dB** vertical ranges; mixing wants **12 dB and 30 dB**. Offer a toggle.
- **Resizable / scalable** window and a full-screen mode are expected in 2025-era tools.

### 2.2 The node interaction model (memorize this)

A band is represented by a draggable **node** (a dot) on the curve. The near-universal control mapping:

| Action | Result |
|---|---|
| Double-click empty graph | Create a new band at that freq/gain |
| Drag node horizontally | Change **frequency** |
| Drag node vertically | Change **gain** |
| Scroll wheel over node | Change **Q / bandwidth** |
| Shift + drag (or shift+drag vertical) | Change **Q** (alternative to scroll) |
| Right-click node | Context menu: shape, slope, channel, delete, dynamic toggle |
| Double-click a value field | Type an exact number |
| Drag the frequency scale | Zoom the display in/out on a region |

Notes:
- Hovering a node should reveal/update that band's parameter readout. On hover, dim the other bands slightly so the active one stands out.
- The node's **visual size or fill** often encodes Q (wider Q = visually broader influence). Some EQs draw the band's bandwidth as a shaded region under the node.
- **Adaptive Q**: optionally widen Q automatically as gain increases, which feels more musical. Make it a toggle, not forced.
- Scroll-wheel-to-Q is the single most requested interaction; users complain loudly when it's missing. Implement it early. (Note: in some GUI frameworks the scroll event needs explicit wiring — don't assume it comes free.)

### 2.3 Color & visual feedback

- Each band gets a distinct **color**, reused for its node, its sub-curve, and its readout, so the eye can track which is which.
- The composite curve is usually a single bright neutral color (white/yellow).
- **Pre-EQ spectrum** is drawn dimmer; **post-EQ** brighter or vice-versa — they must be visually distinguishable. An external/reference spectrum gets its own outline color (Pro-Q uses a light red outline).
- During **Spectrum Grab**, existing bands dim and the frozen spectrum line becomes the interactive element; peaks get frequency (or note) labels.
- **Collisions** are shown as highlighted overlap regions.
- Dynamic bands typically show a **moving secondary line or filled region** indicating the live gain movement within the band's range — you can watch it breathe with the signal.
- Use **GPU-accelerated drawing** for the analyzer; redrawing a 4096-point FFT at 60fps on the CPU will fight your audio thread.

### 2.4 Reactivity expectations

- The analyzer should feel **smooth and lag-free** — reviewers explicitly praise/criticize EQs on interface fluidity. Aim for 60fps, decoupled from the audio callback (audio thread computes FFT magnitudes into a lock-free buffer; UI thread reads and draws).
- Parameter changes should be **smoothed** (Pro-Q calls it "Smart Parameter Interpolation") so automation and knob moves don't zipper or click.
- **Sample-accurate automation** of all params.
- Bypass should be **click-to-A/B instantly** — habitual users punch bypass constantly to evaluate.

### 2.5 What a minimum-viable-but-credible clone needs

If you're shipping a first version and want it to feel legit:

1. Log-frequency / linear-dB interactive graph with labeled grid.
2. Real-time pre/post spectrum analyzer behind the curve, with a tilt (default ~4.5 dB/oct) and adjustable speed.
3. 8+ draggable band nodes with the standard drag/scroll/right-click mapping.
4. Core 7 filter shapes + at least stepped slopes (6/12/24/48).
5. Per-band color, composite curve, hover-to-highlight.
6. Mid/side per-band routing.
7. Output gain, global bypass, A/B, undo/redo.
8. Double-click numeric entry.

Add later: dynamic EQ → spectral dynamics → linear phase → spectrum grab → EQ match/sketch → character/vintage models.

---

## Suggested build order

1. **DSP core** — biquad filter bank, get bell/shelf/cut shapes producing correct frequency response. Verify against a known reference curve.
2. **Static graph** — render the composite curve from band params on a log/dB grid. No interaction yet.
3. **Node interaction** — drag freq/gain, scroll Q, double-click to add, right-click menu. This is where the "feel" lives.
4. **Spectrum analyzer** — FFT on a separate thread, draw behind curve, add tilt + speed.
5. **Per-band routing + slopes + numeric entry.**
6. **Dynamic EQ** — single-band detector/envelope per band.
7. **Phase modes** — linear phase (FIR) once the IIR path is solid.
8. **Polish/magic** — spectrum grab, EQ match, freeze, collision detection, character.

The first three steps get you something that already feels like an EQ. Everything after is depth.

---

*Sources: FabFilter Pro-Q 4 help docs & product page, Three-Body Kirchhoff-EQ product page, SSL Native X-EQ 2, and comparison/review coverage (gearnews, DAW Zone, Tape Op, MusicRadar, Audiophiles). Vendor feature claims paraphrased; verify exact numeric ranges against current docs before committing them to your spec.*
