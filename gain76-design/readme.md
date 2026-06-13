# Altered Audio — Gain 76 · Design System

A design system for **Gain 76**, a retrofuturist utility *gain* plugin by **Altered Audio** (the '76 series). One square window holds a single hero gain dial with the value printed on its face, flanked by INPUT and OUTPUT peak meters, over an L/R peak-hold strip and a MODE / PEAK / LUFS / OVERSAMPLING status row.

The design language is **"a vision of the future from 1974"**: laboratory equipment from an alternate timeline where **Braun**, **NASA Mission Control**, **Olivetti** and **Bang & Olufsen** built audio tools for a lunar research station. Clean, functional, scientific, confident, warmly industrial. Dirty-cream ABS plastic, warm-grey hairlines, amber phosphor accents, and a flat physical dial with a glowing amber indicator dot.

It is deliberately **engineered, not designed** — and explicitly avoids RGB lighting, cyberpunk, skeuomorphic metal, fake rack ears, excessive shadow and modern glassmorphism.

> **Brand:** the wordmark is **type-only** — `ALTERED AUDIO` (brand, the Michroma masthead) over `GAIN 76` (product, in amber). **There is no logo mark.**

---

## Sources

Built from the Gain 76 design reference (the AG-74 faceplate render) and the '76-series design language. There is no separate brand book.

- **GitHub — `NinosGameHub/AlteredAudio`**: https://github.com/NinosGameHub/AlteredAudio — the plugin series' engine source; readers with repo access can explore it to extend the system.
- **Reference render:** the supplied square gain-module faceplate (hero dial with tick arc, twin vertical meters, L/R strip, status row). Per direction, this system replaces its knob with the series face dial (value inside), swaps LINK → PEAK and HEADROOM → LUFS, makes MODE a dropdown (STEREO / MONO / SIDE), and gives the L/R strip true per-channel peak hold.

---

## CONTENT FUNDAMENTALS

**Casing is the signature.** UI labels are **ALL-CAPS**, terse and technical: `GAIN · DB`, `INPUT`, `OUTPUT`, `MODE`, `PEAK`, `LUFS`, `OVERSAMPLING`, `BYPASS`, `POWER`. Knob-face labels fold the unit in after a middot (`GAIN · DB`). The wordmark is all-caps (`ALTERED AUDIO` / `GAIN 76`).

**Units live with the value, lowercase or short.** `+3.46`, `-2.54 dB`, `-14.2` (LUFS), `4x`. Gain values are signed (`+3.46`, `-0.50`); on the dial face the number prints alone — the unit lives in the label. Modes are named directly: `STEREO` / `MONO` / `SIDE`; oversampling: `1x` / `4x` / `8x`.

**Prose voice** (docs, marketing) is second person, plain, confident, lightly opinionated. Short sentences; precise technical terms, never dumbed down. **No emoji, no hype, no exclamation marks** — emphasis comes from structure and the amber accent.

**Vibe:** a precision instrument from an optimistic future that never happened. Warm, engineered, quietly premium.

---

## VISUAL FOUNDATIONS

**Color.** A warm near-monochrome plastic base — `#E6E0D2` surface, `#D8D1C1` panel, `#EDE7D8` raised/knob cream, `#CFC7B5` sunken, `#7A7468` warm-grey hairlines, `#1F1F1F` near-black text, `#5E594E` secondary text. The single accent family is **amber** `#D08A2E` (with mustard `#B8943A` and warning orange `#C56A2B`). Meter segments sit directly on the faceplate — unlit warm grey `#6A6356`, lit amber `#DE9F35`, over-zero orange; indicator LEDs glow `#D99A33`. Dark near-black wells remain for **readouts** only. There is **one** accent system; restraint is the point.

**Surfaces are flat warm plastic separated by 1px hairlines.** No gradients on panels beyond a barely-there top highlight; the only luminous elements are the **lit meter segments** and the **amber LEDs**. Shadows are minimal — a faint panel lift and a deep inset for readout wells, never glassy bloom.

**Backgrounds** are solid warm plastic or well-black. No photography, no texture. The whole unit sits in a **black bezel** with a soft drop shadow.

**Type.** Headings/wordmark use **Eurostile Extended** (substituted with **Michroma** — see Caveats): wide, squared, uppercase, tracked 0.14–0.22em. Everything else — body, control labels, every numeric readout — is **IBM Plex Mono**. Control labels are uppercase with ~0.08em tracking.

**The knob** is the hero control, modeled on the series face dial: a large **flat matte-cream puck** (thin dark edge ring, faint top-light crescent, soft drop shadow) with a ring of **long fine tick marks floating outside it** with a clear air gap — majors every 90°, a bold mark at top — plus a **bold tick that tracks the current value**, aligned with a small **glowing amber dot** on the disc face. The hero gain dial (`face`, ~400px) prints the **value big in light-weight mono (signed, number only)** with the unit folded into the small tracked label beneath (`+3.46` / `GAIN · DB`); compact knobs (~60px) keep a label above and a **light cream readout** (mustard digits) below. Drag vertically, scroll, or double-click to reset.

**Meters.** Peak meters are rows of **embossed segments mounted directly on the faceplate** — no dark well. Unlit segments are warm dark grey (`#6A6356` range), lit segments amber (`#DE9F35` range) turning **warning orange at/above 0 dB**, each with a subtle top highlight + bottom shade so they read as physical lenses. **Peak hold is a lit segment**: the segment at the recent peak stays lit above the fill (holds ~1.4s, then decays) — no separate marker. Vertical for channel I/O (±24 dB, scale alongside); the L/R strip is a **center-split butterfly** — quiet at center, L fills outward to the left, R outward to the right, mirrored −24…+6 scales below each half.

**Controls.** **Select** combos (MODE, OVERSAMPLING) are cream with a 1.5px chevron. Vertical **OptionList** — flat cream rows with a lit amber LED on the selected row — for stacked exclusive choices. Readouts come in two tones: **dark wells with amber digits** for header-level values and **light cream wells with mustard digits** under compact knobs. `BYPASS` is a labelled LED; `POWER` is the round cream key.

**Corner radii** are tight: 2px wells, 3px buttons/combos, 4px option rows, 6px panels, 10px the body, 999px pills.

**Hover / press:** buttons darken slightly; amber lightens to `#E0A047` on hover, darkens to `#B0721F` on press. **Focus** is a 3px `rgba(208,138,46,0.34)` ring.

**Motion** is calm and minimal — 120–260ms standard ease, **no bounce, no overshoot, no glow animation**. Only knob rotation, meter/hold updates and LED states animate.

**Layout.** Fixed proportions inside a square 980×980 window: a host title bar → header (wordmark · POWER) → main panel (INPUT meter+readout · hero dial with 0 / −24 / +24 captions · OUTPUT meter+readout) → L/R peak-hold butterfly strip → status row (MODE dropdown · LINK · PEAK · LUFS · OVERSAMPLING dropdown, cells separated by hairlines) → version mark. 4px spacing grid, 18px panel padding.

**Transparency / blur:** essentially none — opaque warm plastic. Reserve translucency for the modal scrim and the faint amber tints.

**Summary motif:** warm cream plastic, warm hairlines, one amber accent, a flat tick-ringed dial with the value on its face, segmented peak meters with hold, uppercase mono labels. Hardware, not software.

---

## ICONOGRAPHY

**The brand has no logo mark and the plugin ships essentially no icon set** — identity is carried by the type-only wordmark, uppercase text labels, amber LEDs and meter wells. The few glyphs are **simple procedural line drawings**:

- **Select chevron & power glyph** — thin rounded-join strokes (the `Select` chevron / `PowerButton`).

There is **no icon font, no SVG sprite, no PNG set, no emoji, no decorative Unicode**. Keep any added glyphs monochrome in `--text-secondary` (or amber when active), thin rounded strokes, never filled or multicolor. **If you need general UI icons** (folder, settings, close, check) for web/marketing surfaces, substitute **[Lucide](https://lucide.dev)** via CDN — thin ~1.5–2px rounded strokes match — and keep them monochrome. *(Flagged substitution; none is used in the core kit.)*

---

## CAVEATS / SUBSTITUTIONS

- **Font — Eurostile Extended → Michroma.** The spec calls for *Eurostile Extended* (a commercial, non-web-distributable typeface). It is **substituted with Michroma** (Google Fonts) — a free wide, squared geometric face with the same instrument-panel character. **Body / numeric IBM Plex Mono is exact.** ⚠️ **Action:** supply licensed Eurostile Extended `woff2` files and we'll wire `@font-face` to replace Michroma. Fonts load via a Google Fonts `@import` in `tokens/fonts.css` (requires internet).
- **No logo mark.** Per direction, the brand is a **type-only wordmark** (`ALTERED AUDIO` / `GAIN 76`).
- **DSP is cosmetic.** The meters, hold lines, PEAK and LUFS readouts are plausible visualisations driven by a small signal simulation — not a real audio engine.

---

## INDEX / MANIFEST

Root:

| File | What it is |
|---|---|
| `styles.css` | Global entry point — `@import`s fonts + every token + base. Consumers link this. |
| `tokens/fonts.css` | Webfont `@import` (IBM Plex Mono + Michroma). |
| `tokens/colors.css` | Warm palette, amber accents, meter + knob colors, semantic aliases. |
| `tokens/typography.css` | Display + mono stacks, dense size scale, weights, tracking. |
| `tokens/spacing.css` | 4px scale, radii, control/knob sizing. |
| `tokens/effects.css` | Borders, minimal shadow, LED glow, motion. |
| `tokens/base.css` | Reset + on-brand body / label / wordmark defaults. |
| `README.md` / `SKILL.md` | This guide / Agent-Skills manifest. |

Foundation specimen cards live in `guidelines/` (Design System tab: **Colors**, **Type**, **Spacing**, **Brand**).

Components (`window.AuroraFilterDesignSystem_83b750`):

| Component | Group | Role |
|---|---|---|
| `Knob` | controls | Flat cream puck, floating tick ring, value tick + amber dot, value-on-face. |
| `Readout` | controls | Dark amber-text value well. |
| `Select` | controls | Flat combo box with chevron (MODE, OVERSAMPLING). |
| `OptionList` | controls | Vertical LED-marked selector. |
| `SegmentedControl` | controls | Horizontal amber-selected option row. |
| `Button` | controls | Text / toggle / square utility button. |
| `PowerButton` | controls | Round power key with status LED. |
| `Panel` | surfaces | Titled module enclosure with status LED + actions. |
| `LED` | surfaces | Indicator lamp (BYPASS, SIGNAL). |
| `Badge` | surfaces | Status / count pill. |
| `PeakMeter` | displays | Segmented peak meter with hold — vertical I/O or horizontal L/R. |

UI kit — `ui_kits/gain/` (group **Plugin**): the full interactive **Gain 76** window — host title bar, header, INPUT/OUTPUT meters + readouts flanking the hero dial, L/R peak-hold butterfly strip, MODE / LINK / PEAK / LUFS / OVERSAMPLING row, version mark.

> **Note:** the compiler namespace is `AuroraFilterDesignSystem_83b750` (an internal identifier fixed at project creation) — use it verbatim in card / kit HTML even though the brand is Altered Audio · Gain 76.

---

*Build on-brand surfaces by linking `styles.css`, composing the components, and obeying the system: warm cream plastic, one amber accent, a flat tick-ringed dial with the value on its face, peak meters with hold, uppercase mono labels. Engineered, not designed.*
