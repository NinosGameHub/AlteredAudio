# Sync notes

Pulled from the claude.ai/design project **"Gain 76 Design System"**
(projectId `86f4c874-65d5-4e05-ba74-06c02bb00411`) on 2026-06-13 via DesignSync.

## Pulled 2026-06-13 (initial local mirror)

First pull establishing the local `gain76-design/` mirror in the AlteredAudio repo
(`gain` branch). The remote project was already fully populated with the complete
Gain 76 design system.

Files mirrored:
- `readme.md` — full design spec and component manifest
- `SKILL.md` — agent-invocable skill manifest (`gain-76-design`)
- `styles.css` — global entry point
- `tokens/` — colors, fonts, typography, spacing, effects, base reset
- `components/controls/*.prompt.md` — Knob, Select, PowerButton, Button, OptionList, Readout, SegmentedControl
- `components/displays/PeakMeter.prompt.md`
- `components/surfaces/Badge.prompt.md`, `LED.prompt.md`, `Panel.prompt.md`
- `ui_kits/gain/gain-app.jsx` — full interactive Gain 76 window (React)
- `ui_kits/gain/index.html` — kit page, mounts `AF_GainApp`

Intentionally NOT mirrored (generated artifacts / previews):
- `_ds_bundle.js`, `_ds_manifest.json`, `_adherence.oxlintrc.json` — compiled by the DS compiler
- `components/**/*.d.ts` — generated TypeScript typings
- `components/**/*.card.html`, `guidelines/*.card.html` — Design System pane preview cards
- `uploads/*.png` — reference renders
- `CLAUDE.md` — remote project-specific instructions

## Remote project state (2026-06-13)

Components: Knob, Readout, Select, OptionList, SegmentedControl, Button, PowerButton,
Panel, LED, Badge, PeakMeter. UI kit: `ui_kits/gain/` (interactive 980×980 window).
Compiler namespace: `AuroraFilterDesignSystem_83b750`.
