---
name: altered-audio-design
description: Use this skill to generate well-branded interfaces and assets for Altered Audio (a VST3 modular effects-rack plugin with a Braun/Moog warm-analog look), either for production or throwaway prototypes/mocks/etc. Contains essential design guidelines, colors, type, fonts, assets, and UI kit components for prototyping.
user-invocable: true
---

Read the README.md file within this skill, and explore the other available files.

If creating visual artifacts (slides, mocks, throwaway prototypes, etc), copy assets out and create static HTML files for the user to view. If working on production code, you can copy assets and read the rules here to become an expert in designing with this brand.

If the user invokes this skill without any other guidance, ask them what they want to build or design, ask some questions, and act as an expert designer who outputs HTML artifacts _or_ production code, depending on the need.

## Quick map

- `README.md` — the full design guide: product context, content & visual foundations, iconography, manifest. **Start here.**
- `styles.css` — link this one file to inherit every token; it `@import`s `tokens/*.css`.
- `tokens/` — colors, typography, spacing, effects, base element defaults.
- `guidelines/*.card.html` — foundation specimens (colors, type, spacing, brand).
- `components/` — React primitives (`Knob`, `ToggleSwitch`, `Select`, `Button`, `Card`, `Badge`, `ModuleTile`, `CRTDisplay`, `Meter`, `EQCurve`, `ModMatrix`), each with a `.d.ts` and `.prompt.md`.
- `ui_kits/plugin/` — interactive recreation of the full plugin window.

## Non-negotiables

- Warm-analog, not Apple: dirty cream ABS-plastic surfaces (`#E7E2D6` / `#EDE8DC`), warm-grey hairlines (`#B2ADA1`), near-black warm text.
- Five **category accent colors** organize modules: Dynamics orange `#C4621D`, Time brown `#7A5438`, Modulation mustard `#AA8B10`, Filter/EQ blue `#3A6694`, Spatial teal `#5E8480`. A category owns its module everywhere (tile strip, LED, panel dot, knob ring). Active green `#4CA84C`.
- Flat surfaces, no card shadows, no gradients — the only luminous element is the amber CRT (`#E4A210` on `#060504`).
- Flat indicator-line knobs (cream disc + black line + range dots), pill toggles, tight radii (4px combos, 6px cards).
- System font; monospace for ALL numerics. Dense **UPPERCASE** technical labels. No emoji; terse, technically-literate copy.
