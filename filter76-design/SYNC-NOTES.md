# Sync notes

Pulled from the claude.ai/design project **"Filter 76 Design System"**
(projectId `83b75076-44a8-41a3-8c95-6789d7d779d6`) on 2026-06-12 via DesignSync.

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
