# Altered Audio — Design System

A design system for **Altered Audio**, a VST3 *modular effects rack* audio plugin. Users load one plugin into a DAW and build signal chains by adding, reordering and blending DSP modules — filter, EQ, gain, delay, reverb, waveshaper, compressor, clipper, transient shaper, limiter, gate, chorus, flanger, phaser and a stereo/spatial module — inside a single custom window.

The visual language is **Braun SK / Moog-modular warm-analog**: a dirty cream ABS-plastic canvas, warm-grey hairlines, five category accent colors that organize the modules, an amber CRT spectrum display, and flat physical-style knobs with a single black indicator line. It is deliberately *hardware*, not *Apple* — engineered, warm, restrained.

> **Direction history:** an earlier revision of the plugin used an Apple-inspired light-grey / blue look. The project pivoted (see `ui-design-todo.md` in the repo) and this design system now tracks the **shipped warm-analog direction**. If you find any cool-grey/blue residue, treat it as a bug.

---

## Sources

Reverse-engineered from the plugin source — there is no separate brand book. Canonical references:

- **GitHub — `NinosGameHub/AlteredAudio`** (branch `master`): https://github.com/NinosGameHub/AlteredAudio
  - `source/AaLookAndFeel.h` — the entire palette (`namespace AaColor`): warm surfaces, the 5 category colors, CRT amber, and the flat-disc knob render. **Source of truth for color + knob.**
  - `source/ModuleTileList.cpp` / `.h` — the 160px left tile list: 15 modules, per-module category color, tile rendering (4px strip, mono number, centered name, LED dot), drag-to-reorder.
  - `source/CRTDisplay.cpp` / `.h` — the 180px amber phosphor spectrum strip.
  - `source/ModulePanels.cpp` / `.h` — every module's detail panel: exact knob/combo/toggle layouts for all 15 modules.
  - `source/PluginEditor.cpp` / `.h` — window chrome: 1000×640 window, 40px header, layout math.
  - `source/ParameterIDs.h` — full parameter list per module (realistic mock copy).
  - `ui-design-todo.md` — the live design backlog (header controls, EQ curve, GR meters, modulation matrix, preset browser, …).

> Readers with repo access should explore these to extend the system — `AaLookAndFeel.h` and `ModulePanels.cpp` encode pixel-level decisions (knob disc ratio, indicator-line geometry, category-color mapping) that this system mirrors as tokens and components.

There are **no image, logo or font assets** in the repo — the plugin draws all chrome procedurally and uses the host OS system font. The "logo" is a typographic wordmark; the typeface is the native system stack (see Iconography and Visual Foundations).

---

## The product at a glance

- **One window, fixed at 1000×640**, non-resizable.
- **Header (40px):** warm-white, hairline bottom. Left block (above the tile list) holds the `ALTERED AUDIO` wordmark; the right block holds the preset transport, channel-mode (ST / M-S), animated I/O meters and a global-bypass power button. *(The header controls beyond the wordmark are net-new design from the backlog, built on-brand.)*
- **Left tile list (160px):** 15 module tiles stacked vertically. Each tile shows a 4px **category-color** left strip (always on), a mono chain number, the centered module name, and an LED dot (category color when engaged, warm-grey when bypassed). The selected tile gets a faint category tint and a category-colored name. Drag to reorder in the real plugin.
- **Amber CRT (180px):** a near-black phosphor strip with a 32-bar amber spectrum, grid + scan-line texture and a mono frequency axis, sitting above the detail panel.
- **Detail panel:** the selected module's controls — a 44px header (category dot + name + Active toggle) over rows of flat knobs, combo boxes and pill toggles.

---

## CONTENT FUNDAMENTALS

How Altered Audio writes copy — derived from every visible string (`ModulePanels.cpp`, `ModuleTileList.cpp`) and the roadmap's voice.

**Casing is the signature.** UI labels are **ALL-CAPS** and terse: `FILTER`, `FREQ`, `Q`, `GAIN`, `WET/DRY`, `THRESH`, `PRE-DELAY`, `BIT DEPTH`, `MOD RATE`, `ST LINK`, `HAAS (ms)`. The wordmark is all-caps. Tile names are abbreviated to fit (`COMP`, `TRANS`, `LIM`, `FLANGE`, `WAVE`); panel titles spell out (`COMPRESSOR`, `TRANSIENT SHAPER`). This clipped, engraved-faceplate voice is the brand's verbal texture.

**Units live in the label, in parentheses or lowercase.** `TIME (ms)`, `RATE (Hz)`, `GAIN (dB)`, `CEILING (dB)`, `DEPTH (ms)`. Never spell units long-form in a control.

**Toggles read as plain capitalized phrases**, not all-caps: `Active`, `Ping-Pong`, `Auto Gain`. The mode is named directly — no "Enable/Disable" wrapper.

**Prose voice (roadmap, docs) is second person, plain, confident, lightly opinionated.** Short sentences. Technical terms used precisely, never dumbed down. When writing longer-form copy, keep that register: direct, technically literate, never marketing-fluffy.

**No emoji. No exclamation-point hype.** Emphasis comes from **bold** and structure, not decoration.

**Vibe:** a pro-audio tool with a vintage-hardware soul. Warm, engineered, quietly premium — Braun calculator meets Moog modular.

---

## VISUAL FOUNDATIONS

**Color.** A warm near-monochrome base — `#E7E2D6` canvas, `#EDE8DC` cards, `#D9D4C8` pressed/sunken, `#B2ADA1` warm-grey hairlines, `#1C1815` near-black text, `#7A746E` warm secondary text — organized by **five category accent colors** that map module groups: **Dynamics** burnt-orange `#C4621D` (comp / clip / gate / limiter / transient / waveshaper), **Time** warm-brown `#7A5438` (delay / reverb), **Modulation** mustard `#AA8B10` (chorus / flanger / phaser), **Filter/EQ** space-age blue `#3A6694`, **Spatial** desaturated teal `#5E8480`. Gain is neutral grey. **Active green** is `#4CA84C`. A category color owns its module everywhere — tile strip, tile LED, panel header dot, knob value ring.

**No gradients on surfaces. No glow except the CRT.** Surfaces are flat warm fills separated by 1px hairlines; the native plugin draws **no card shadows**. The one luminous element is the **amber CRT** (`#E4A210` on near-black `#060504`) with a soft bar glow, scan-lines and a faint grid.

**Backgrounds** are solid warm plastic (`#E7E2D6` / `#EDE8DC`) or CRT-black. No photography, no texture beyond the CRT scan-lines.

**Type.** OS **system font** at small, dense faceplate sizes: 8.5px mono tile numbers, 9px mono CRT/meter labels, 10px uppercase control labels, 12–13px body/combo, 13px bold wordmark, 15px panel headers. A **monospace** face carries all numerics — knob values, CRT frequency labels, tile chain numbers. Control labels uppercase with light ~0.02em tracking; wordmark ~0.10em.

**Corner radii** are hardware-flat and tight: **3px** micro, **4px** combos/meters, **6px** cards/dialogs, **999px** pills for toggles. Nothing heavily rounded.

**Knobs** are the hero control and the biggest change from the old look: a **flat dirty-cream disc** (`#E4DFD0`, 1px warm outline), a **near-black indicator line** from ~0.2r to ~0.86r, and two small **range-marker dots** at the 7-o'clock start and 5-o'clock end of the 270° sweep — a Braun/Moog physical knob. An optional thin **value ring** in the module's category color sweeps outside the disc. The value reads as mono text below.

**Toggles** are pill switches: a 32×18 track, **green when on / warm-grey when off**, with a white sliding thumb. Used for Active/bypass, Ping-Pong, Auto Gain, EQ band enables.

**Combo boxes / selects:** warm-white fill, 4px radius, 1px border, a 1.5px chevron in secondary grey.

**Tiles** (left list): the defining surface. Always-on 4px category strip; selected = 14% category tint + category-colored name; LED dot = category color (engaged) or warm-grey (bypassed).

**Meters:** thin segmented bars on CRT-black — level meters light green→amber→orange bottom-up; gain-reduction lights dynamics-orange top-down.

**Hover / press:** tiles warm to the pressed fill on hover; buttons darken slightly; accent orange lightens to `#D26C24` / darkens to `#A8511A`. Transitions are short (120–200ms), standard ease — calm, no spring/overshoot.

**Focus** is a 3px `rgba(196,98,29,0.32)` ring.

**Layout rules.** Fixed 40px header across the top; below it a fixed 160px tile column on the left and a flexible right column (180px CRT + detail panel). 4px spacing grid. 16px panel padding. Knobs lay out in equal columns per row.

**Transparency / blur:** essentially none — surfaces are opaque warm plastic. Reserve translucency for the modal scrim (`--aa-overlay-scrim`) and the 14% category selection tint.

**Summary motif:** warm cream plastic, warm hairlines, five category accent colors, amber CRT, flat indicator-line knobs, pill toggles, dense uppercase technical labels, monospace numerics. Hardware, not software.

---

## ICONOGRAPHY

**The plugin ships no icon assets.** Native glyphs are drawn procedurally with JUCE paths: the combo-box chevron (1.5px stroked "v"), the tile LED dot, the toggle thumb, knob range dots. **No icon font, no SVG sprite, no PNG set, no emoji, no decorative Unicode.** Meaning is carried by text labels and the category colors.

**For web/marketing/prototype surfaces that need icons** (preset/folder, prev/next, power/bypass, add, close, check), this system substitutes **[Lucide](https://lucide.dev)** via CDN — thin, rounded-join ~1.5–2px stroke, matching the procedurally-drawn chevrons. **This is a substitution, flagged for the user.** Keep icons monochrome in `--text-muted` (or a category/accent color when active), 13–18px. Never filled, duotone or multicolor.

> If you have official Altered Audio icons, drop them in `assets/icons/` and update this section.

---

## CAVEATS / SUBSTITUTIONS

- **Font:** no webfont in the repo; the plugin uses the host system font. This system uses the native `-apple-system / system-ui` stack plus the OS monospace for numerics — the faithful recreation. Supply a specific typeface if you want guaranteed cross-platform rendering and we'll wire `@font-face`.
- **Icons:** Lucide (CDN) substituted for the absent icon set — see Iconography.
- **Logo:** no logo file; the brand mark is the typographic `ALTERED AUDIO` wordmark.
- **Header controls, preset browser, add-module dialog** are net-new design built from the `ui-design-todo.md` backlog, on-brand — not yet reflected in plugin source. The **EQ curve display** (backlog #2) and **modulation matrix** (backlog #5) are now designed as the `EQCurve` and `ModMatrix` components and wired live into the UI kit (EQ panel + a MOD overlay from the header). **Still open in the backlog and NOT yet designed here:** in-panel compressor/limiter gain-reduction meters, waveshaper/clipper transfer-curve graphs, knob modulation arcs + right-click menus, bypass dimming of the whole panel, and the CRT real-FFT wiring (a C++ task — the visual is done).

---

## INDEX / MANIFEST

Root files:

| File | What it is |
|---|---|
| `styles.css` | Global entry point — `@import`s every token + base file. Consumers link this. |
| `tokens/colors.css` | Warm palette, 5 category colors, CRT + knob colors, semantic aliases. |
| `tokens/typography.css` | System + mono font stacks, dense size scale, weights, tracking. |
| `tokens/spacing.css` | 4px scale, fixed layout dimensions (header/list/CRT/window), radii. |
| `tokens/effects.css` | Borders, minimal elevation, focus ring, motion, knob geometry, CRT overlays. |
| `tokens/base.css` | Reset + on-brand body/label/wordmark defaults. |
| `README.md` | This document. |
| `SKILL.md` | Agent Skills manifest (for Claude Code). |

Foundation specimen cards live in `guidelines/` (Design System tab: **Type**, **Colors**, **Spacing**, **Brand**).

Components:

| Component | Group | Role |
|---|---|---|
| `Knob` | controls | Flat Braun-style disc + indicator line + range dots; optional category value ring. |
| `ToggleSwitch` | controls | Pill switch (green on / warm-grey off). |
| `Select` | controls | Flat combo box with chevron. |
| `Button` | controls | Text button — primary / secondary / ghost. |
| `ModuleTile` | surfaces | Left-list tile: category strip, number, name, LED dot. |
| `Card` | surfaces | Warm surface card with selected state. |
| `Badge` | surfaces | Small status / count pill. |
| `CRTDisplay` | displays | Amber phosphor spectrum strip. |
| `Meter` | displays | Segmented I/O & gain-reduction meter on CRT-black. |
| `EQCurve` | displays | EQ frequency-response graph with draggable bands (backlog #2). |
| `ModMatrix` | displays | Modulation routing grid — sources × destinations (backlog #5). |

UI kit — `ui_kits/plugin/` (group **Plugin**): the full interactive window — header, tile list, CRT, swappable detail panels for all 15 modules, preset browser and add-module dialogs.

---

*Build on-brand screens by linking `styles.css`, composing the components, and obeying the category-color system. When in doubt: warmer, flatter, category-coded, monospace numbers, hardware not software.*
