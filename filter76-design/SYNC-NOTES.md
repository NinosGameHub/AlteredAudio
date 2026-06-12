# Sync notes

Pulled from the claude.ai/design project **"Filter 76 Design System"**
(projectId `83b75076-44a8-41a3-8c95-6789d7d779d6`) on 2026-06-12 via DesignSync.

## Pushed back 2026-06-12 (plugin v0.8.5 sync)

JUCE implementation changes mirrored INTO the design system:

- `components/displays/SpectrumDisplay.jsx`
  - dB scale +12…−30 (was +12…−60); grid/labels at 6 dB steps down to −24
  - background: even linear gradient `#171511 → #11100D` — the fixed radial
    glow and the CRT vignette overlay are REMOVED (user decision: light may
    come from the node only, never from a fixed point on the glass)
  - new node spotlight: warm radial glow (`#FBD27A`, 0.20 → 0) centered on
    the node, radius 0.85 × plot height, tracks the node as it moves
  - analyzer fill quieted (0.28/0.11 stops); analyzer stroke now brushed
    with a vertical gradient that stays strong to ~93% height and dissolves
    only in the last few percent (no jittery floor line, HF stays visible)
- `components/controls/Knob.jsx`
  - face label: 6.2 viewBox-units (was 4.6), letter-spacing 0.3 (was 1.5) —
    bigger type, tighter tracking, per user feedback

## Pushed back 2026-06-12 (Gain 76 kit, plugin v0.8.7)

- `ui_kits/gain/App.jsx` — NEW: the Gain 76 utility plugin layout matching
  the JUCE GainEditor: 820×820 square faceplate, header wordmark + power
  key, main panel with INPUT/OUTPUT vertical meters flanking a 340px hero
  face knob (live value on the disc, GAIN · DB folded label, ±24 dB,
  scale labels -24/0/+24), L/R peak strip with hold markers (R mirrored),
  footer cells MODE (STEREO/MONO/SIDE select) | PEAK (held, click resets)
  | LUFS (momentary, K-weighted) | OVERSAMPLING (1x/4x/8x select).

Everything that *defines* the design is mirrored here: `readme.md` (the full
spec), `SKILL.md`, `styles.css`, `tokens/`, all component sources
(`components/**/*.jsx`) and usage docs (`*.prompt.md`), and the full plugin
UI kit (`ui_kits/plugin/`).

Intentionally NOT mirrored (generated artifacts / previews, re-fetch from the
remote project if ever needed):

- `_ds_bundle.js`, `_ds_manifest.json`, `_adherence.oxlintrc.json` — compiled
  by the DS compiler from the component sources. Without `_ds_bundle.js` the
  local `ui_kits/plugin/index.html` will not render standalone.
- `components/**/*.d.ts` — generated TypeScript typings.
- `components/**/*.card.html`, `guidelines/*.card.html` — Design System pane
  preview cards (specimens of the same tokens mirrored here).
- `templates/af74-plugin/` — starter shell for consuming web projects.
- `uploads/*.png` — ChatGPT reference renders of the faceplate.
