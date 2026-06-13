# Altered Audio — Filter 76 · Design System

A design system for **Filter 76**, a retrofuturist audio *filter* plugin by **Altered Audio**. One window holds a dominant amber spectrum display over a single multimode filter (LP / HP / BP / NOTCH / PEAK / SHELF) with Frequency, Resonance, Drive, Mix and Output, plus modulation routing, an LFO engine and an envelope follower.

The design language is **"a vision of the future from 1974"**: laboratory equipment from an alternate timeline where **Braun**, **NASA Mission Control**, **Olivetti** and **Bang & Olufsen** built audio tools for a lunar research station. Clean, functional, scientific, confident, warmly industrial. Dirty-cream ABS plastic, warm-grey hairlines, an amber CRT phosphor display, and flat physical knobs with a single black indicator line.

It is deliberately **engineered, not designed** — and explicitly avoids RGB lighting, cyberpunk, skeuomorphic metal, fake rack ears, excessive shadow and modern glassmorphism.

> **Brand:** the wordmark is **type-only** — `ALTERED AUDIO` (brand, the Michroma masthead) over `FILTER 76` (product, in amber). **There is no logo mark.**

---

## Sources

Built from the Filter 76 design specification and grounded in the plugin's engine source. There is no separate brand book.

- **GitHub — `NinosGameHub/AlteredAudio`** (branch `Filter`): https://github.com/NinosGameHub/AlteredAudio/tree/Filter
  - `source/AaLookAndFeel.h` — the procedural warm palette and the flat cream **knob** render (disc ratio, indicator-line geometry) this system mirrors.
  - `source/ParameterIDs.h` — the filter / LFO / envelope / modulation parameter list behind the realistic readouts.
  - `source/LFOEngine.h` — the LFO shapes (Sine / Triangle / Square / Sawtooth / Random S&H) used by the LFO selector.
  - `source/CRTDisplay.*`, `source/BiquadFilter.*`, `source/EnvelopeFollower.*` — the display and DSP behaviours the components recreate cosmetically.
  - The repo also documents a *separate* design system for the sibling **modular-rack** product (`design-system/`). Filter 76 is the focused single-filter product; readers with repo access can explore both to extend the system.
- **Reference render:** the supplied Filter 76 faceplate image — the canonical layout this system reproduces (header, spectrum, filter cluster, modulation / LFO / envelope / utility row, footer).

---

## CONTENT FUNDAMENTALS

**Casing is the signature.** UI labels are **ALL-CAPS**, terse and technical: `FILTER TYPE`, `FREQ · HZ`, `RES · Q`, `DRIVE · DB`, `MIX · %`, `OUT · DB`, `GAIN`, `SLOPE`, `MODE`, `MODULATION`, `LFO ENGINE`, `ENVELOPE FOLLOWER`, `SIGNAL PATH`. Knob-face labels fold the unit in after a middot (`FREQ · HZ`). The wordmark is all-caps (`ALTERED AUDIO` / `FILTER 76`).

**Units live with the value, lowercase or short.** `800`, `6.0`, `3.0`, `+60.0 %`, `0.1 Hz`, `120.0 ms`, `44.1 kHz`, `0.00 ms`, `0.5 %`. On knob faces the number prints alone — the unit lives in the label. Range captions are rare; readouts carry the precision.

**Presets are numbered + named:** `004 SLOW SWEEP`, `001 ANALOG LOWPASS`. Modes are named directly: `ANALOG` / `CLEAN`, `SINE` / `TRI` / `SQR` / `RND`, `LP` / `HP` / `BP`. Slopes: `12 dB` / `24 dB` / `48 dB`. The build version reads quietly at the footer's right edge (`v0.7.6`).

**Prose voice** (docs, marketing) is second person, plain, confident, lightly opinionated. Short sentences; precise technical terms, never dumbed down. **No emoji, no hype, no exclamation marks** — emphasis comes from structure and the amber accent.

**Vibe:** a precision instrument from an optimistic future that never happened. Warm, engineered, quietly premium.

---

## VISUAL FOUNDATIONS

**Color.** A warm near-monochrome plastic base — `#E6E0D2` surface, `#D8D1C1` panel, `#EDE7D8` raised/knob cream, `#CFC7B5` sunken, `#7A7468` warm-grey hairlines, `#1F1F1F` near-black text, `#5E594E` secondary text. The single accent family is **amber** `#D08A2E` (with mustard `#B8943A` and warning orange `#C56A2B`). The display is amber phosphor — `#F0B547` lines on near-black `#171717` — and indicator LEDs glow `#D99A33`. There is **one** accent system; restraint is the point.

**Surfaces are flat warm plastic separated by 1px hairlines.** No gradients on panels beyond a barely-there top highlight; the only luminous elements are the **spectrum display** and the **amber LEDs**. Shadows are minimal — a faint panel lift and a deep inset for readout wells / the display, never glassy bloom.

**Backgrounds** are solid warm plastic or graph-black. No photography. The faceplate carries an optional **subtle sheen gradient** (`--faceplate-sheen`, soft-light: light falls from the top, faint shade pools at the bottom) and the **spectrum window floats** on a very subtle omnidirectional shadow (`--shadow-float`). An **optional tactile finish** also exists for faceplates: fine grain (`--tex-grain`, blend `overlay`) plus a wear mottle + vignette (`--tex-mottle` / `--tex-vignette`, blend `multiply`) — the **clean finish is the default**. The whole unit sits in a **black bezel** with a soft drop shadow.

**Type.** Headings/wordmark use **Eurostile Extended** (substituted with **Michroma** — see Caveats): wide, squared, uppercase, tracked 0.14–0.22em. Everything else — body, control labels, every numeric readout — is **IBM Plex Mono**. Control labels are uppercase with ~0.08em tracking; readout values are mono amber in dark wells.

**Knobs** are the hero control, modeled on the AB·VALUE reference dial: a large **flat matte-cream puck** (thin dark edge ring, faint top-light crescent, soft drop shadow) with a ring of **long fine tick marks floating outside it** with a clear air gap — majors every 90°, a bold mark at top — plus a **bold tick that tracks the current value**, aligned with a small **glowing amber dot** on the disc face. Hero filter knobs (`face`, ~132px) print the **value big in light-weight mono (number only)** with the unit folded into the small tracked label beneath (`1563` / `FREQ · HZ`); compact LFO / envelope knobs (~60px) keep a label above and a dark amber readout below. Drag vertically, scroll, or double-click to reset.

**The spectrum display** is a clean **response view**: a near-black warm glass with a sparse log grid (dB lines at 12 / 0 / −12 / −24, 0 dB brighter), the filter's response drawn as a crisp glowing curve over a soft gradient fill, a round draggable cutoff node with a full-height hairline, and — when modulation targets frequency — a dimmer **ghost curve** with a small dot marking the modulated target. No analyzer clutter by default; the window **floats** on a very subtle omnidirectional shadow (`--shadow-float`).

**Controls.** Vertical **OptionList** (FILTER TYPE, SLOPE 12/24/48, MODE) — flat cream rows with a lit amber LED on the selected row. **A/B are two square toggle keys** (active fills amber); the LFO waveform selector is a row of small **text toggle keys** (SINE / TRI / SQR / RND). **Select** combos are cream with a 1.5px chevron. Readouts come in two tones: **dark wells with amber digits** for the header (preset, oversampling, mix) and **light cream wells with mustard digits** under compact knobs (GAIN, RATE, DEPTH, AMOUNT…). The footer states signal path as plain text (`STEREO`).

**Corner radii** are tight: 2px wells, 3px buttons/combos, 4px option rows, 6px panels, 10px the display/body, 999px pills.

**Hover / press:** buttons darken slightly; amber lightens to `#E0A047` on hover, darkens to `#B0721F` on press. **Focus** is a 3px `rgba(208,138,46,0.34)` ring.

**Motion** is calm and minimal — 120–260ms standard ease, **no bounce, no overshoot, no glow animation**. Only knob rotation, spectrum/meter updates and LED states animate.

**Layout.** Fixed proportions inside a 1400×900 (resizable) window: header band (wordmark · preset transport · oversampling + mix readouts · A/B keys · power) → dominant response display with OUTPUT L/R meter → filter cluster (type list · 5 face knobs + small GAIN · slope/mode lists) → bottom row (MODULATION · LFO ENGINE · ENVELOPE FOLLOWER) → system footer (stats · signal path · version). 4px spacing grid, 18px panel padding.

**Transparency / blur:** essentially none — opaque warm plastic. Reserve translucency for the modal scrim and the faint amber value/grid tints.

**Summary motif:** warm cream plastic, warm hairlines, one amber accent + amber display, beveled tick-ringed indicator knobs, uppercase mono labels, amber readout wells. Hardware, not software.

---

## ICONOGRAPHY

**The brand has no logo mark and the plugin ships essentially no icon set** — identity is carried by the type-only wordmark, uppercase text labels, amber LEDs and readout wells. The few glyphs are **simple procedural line drawings** in the plugin's own idiom, matching the Select chevron and the oscilloscope trace:

- **LFO waveform keys** — v0.7.x uses small **text toggle buttons** (SINE / TRI / SQR / RND), not glyphs.
- **Transport chevrons & power glyph** — thin rounded-join strokes (the `Button` icon slot / `PowerButton`).

There is **no icon font, no SVG sprite, no PNG set, no emoji, no decorative Unicode**. Keep any added glyphs monochrome in `--text-secondary` (or amber when active), thin rounded strokes, never filled or multicolor. **If you need general UI icons** (folder, settings, close, check) for web/marketing surfaces, substitute **[Lucide](https://lucide.dev)** via CDN — thin ~1.5–2px rounded strokes match the procedural glyphs — and keep them monochrome. *(Flagged substitution; none is used in the core kit.)*

---

## CAVEATS / SUBSTITUTIONS

- **Font — Eurostile Extended → Michroma.** The spec calls for *Eurostile Extended* (a commercial, non-web-distributable typeface). It is **substituted with Michroma** (Google Fonts) — a free wide, squared geometric face with the same instrument-panel character. **Body / numeric IBM Plex Mono is exact.** ⚠️ **Action:** supply licensed Eurostile Extended `woff2` files and we'll wire `@font-face` to replace Michroma. Fonts load via a Google Fonts `@import` in `tokens/fonts.css` (requires internet); the DS compiler does not bundle them as local binaries.
- **No logo mark.** Per direction, the brand is a **type-only wordmark** (`ALTERED AUDIO` / `FILTER 76`). The LFO waveform glyphs are clean line recreations of the marks in the reference render.
- **DSP is cosmetic.** The spectrum, response curve, meters and oscilloscope are plausible visualisations driven by the control state — not a real audio engine.

---

## INDEX / MANIFEST

Root:

| File | What it is |
|---|---|
| `styles.css` | Global entry point — `@import`s fonts + every token + base. Consumers link this. |
| `tokens/fonts.css` | Webfont `@import` (IBM Plex Mono + Michroma). |
| `tokens/colors.css` | Warm palette, amber accents, display + knob colors, semantic aliases. |
| `tokens/typography.css` | Display + mono stacks, dense size scale, weights, tracking. |
| `tokens/spacing.css` | 4px scale, radii, control/knob sizing, fixed window dimensions. |
| `tokens/effects.css` | Borders, minimal shadow, knob bevel, LED glow, motion, display overlays. |
| `tokens/base.css` | Reset + on-brand body / label / wordmark defaults. |
| `README.md` / `SKILL.md` | This guide / Agent-Skills manifest. |

Foundation specimen cards live in `guidelines/` (Design System tab: **Colors**, **Type**, **Spacing**, **Brand**).

Components (`window.AuroraFilterDesignSystem_83b750`):

| Component | Group | Role |
|---|---|---|
| `Knob` | controls | Flat cream puck, floating tick ring, value tick + amber dot, value-on-face. |
| `Readout` | controls | Dark amber-text value well. |
| `SegmentedControl` | controls | Horizontal amber-selected option row / display tabs. |
| `OptionList` | controls | Vertical LED-marked selector (filter type, slope, mode). |
| `Select` | controls | Flat combo box with chevron. |
| `Button` | controls | Text / toggle / square utility button. |
| `PowerButton` | controls | Round power key with status LED. |
| `Panel` | surfaces | Titled module enclosure with status LED + actions. |
| `LED` | surfaces | Indicator lamp. |
| `Badge` | surfaces | Status / count pill. |
| `SpectrumDisplay` | displays | The dominant response display + ghost curve + draggable node. |
| `Meter` | displays | Segmented amber I/O meter with dB scale. |
| `Oscilloscope` | displays | Small amber trace (LFO waveform / envelope). |

UI kit — `ui_kits/plugin/` (group **Plugin**): the full interactive Filter 76 window. Template — `templates/af74-plugin/`: a starter shell consuming projects can copy.

> **Note:** the compiler namespace is `AuroraFilterDesignSystem_83b750` (an internal identifier fixed at project creation) — use it verbatim in card / kit HTML even though the brand is now Altered Audio · Filter 76.

---

*Build on-brand surfaces by linking `styles.css`, composing the components, and obeying the system: warm cream plastic, one amber accent, beveled tick-ringed knobs, uppercase mono labels, dark amber readout wells. Engineered, not designed.*
