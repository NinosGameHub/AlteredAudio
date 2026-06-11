# UI Kit — Altered Audio Plugin Window

An interactive recreation of the full plugin window, faithful to the JUCE `AlteredAudioEditor`: a fixed **1000×640** window with a 40px header, a 160px left tile list, a 180px amber CRT spectrum, and a swappable module detail panel.

## What it demonstrates

- **Header** — `ALTERED AUDIO` wordmark (left, above the tile list); preset transport, ST / M-S channel mode, animated I/O meters and a global-bypass power button (right).
- **Tile list** — all 15 modules as selectable `ModuleTile`s with category strips and LED dots. Click to open; the **+ Module** button opens the add-module picker.
- **CRT** — animated amber `CRTDisplay` spectrum; click it to open the preset browser.
- **Detail panel** — data-driven from `panels.jsx`: combo pickers, rows of flat indicator-line `Knob`s (value ring in the module's category color), and pill `ToggleSwitch`es, with the module's **Active** toggle and category dot in the header. EQ has its own 8-band grid.
- **Dialogs** — preset browser and add-module picker.

## Files

| File | Role |
|---|---|
| `index.html` | The whole kit: window shell, fixed-size scaler, and one inline `<script type="text/babel">` holding the 15-module catalog, detail panels, chrome (header / tile list / dialogs) and the `App` state wiring. |

The UI-kit logic is kept **inline in one Babel script** (not separate `.jsx` files) on purpose: the design-system compiler sweeps every project `.jsx` into `_ds_bundle.js`, so loose kit files would be double-bundled and could collide. Inlining keeps the kit self-contained and load-order-safe.

## Notes

- Composes the DS primitives (`Knob`, `Select`, `ToggleSwitch`, `ModuleTile`, `CRTDisplay`, `Meter`, `Button`) from `window.AlteredAudioDesignSystem_3ace71` — it does not re-implement them.
- Icons are **Lucide** (CDN), the documented substitution for the plugin's absent icon set.
- Cosmetic recreation: knobs are display-only (no audio); values are realistic defaults lifted from `ModulePanels.cpp`.
- The header controls, preset browser and add-module dialog are on-brand designs from the `ui-design-todo.md` backlog, not yet in plugin source.
