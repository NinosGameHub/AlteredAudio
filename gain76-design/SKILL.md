---
name: gain-76-design
description: Use this skill to generate well-branded interfaces and assets for Altered Audio's Gain 76 (a retrofuturist utility gain plugin), either for production or throwaway prototypes/mocks/etc. Contains essential design guidelines, colors, type, fonts, UI components and the full plugin UI kit for prototyping.
user-invocable: true
---

Read the `README.md` file within this skill, and explore the other available files.

If creating visual artifacts (slides, mocks, throwaway prototypes, etc), copy assets out and create static HTML files for the user to view. If working on production code, you can copy assets and read the rules here to become an expert in designing with this brand.

If the user invokes this skill without any other guidance, ask them what they want to build or design, ask some questions, and act as an expert designer who outputs HTML artifacts _or_ production code, depending on the need.

Key entry points:
- `styles.css` — link this one file to inherit every token + webfont.
- `tokens/` — colors, typography, spacing, effects (CSS custom properties).
- `components/` — React UI primitives (controls, surfaces, displays); read each `*.prompt.md` for usage.
- `ui_kits/gain/` — the full Gain 76 window, a worked example of composing the components.
- `guidelines/` — foundation specimen cards.

Brand in one line: **Altered Audio · Gain 76 — warm cream ABS plastic, one amber accent, a flat tick-ringed hero dial with the signed value printed on its face, segmented peak meters with hold lines, uppercase IBM Plex Mono labels, a type-only wordmark (no logo) — engineered, not designed.** Headings want Eurostile Extended (substituted with Michroma until licensed files are supplied).
