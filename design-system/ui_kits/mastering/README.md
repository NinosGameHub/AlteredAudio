# UI Kit — Altered Audio Mastering Dashboard

A single-window **mastering assistant** dashboard, built to match a supplied reference layout and rendered in the Altered Audio warm-analog brand (cream surfaces, burnt-orange accent, thin-ink line graphics, small-caps labels, monospace numerics).

> This is a **different surface** from `ui_kits/plugin/` (the modular effects-rack window that mirrors the current JUCE source). The mastering dashboard is a layout the user requested from a reference image; it is branded Altered Audio but is its own product view.

## Layout

- **Top bar** — logo + `ALTERED AUDIO MASTER` wordmark, preset transport (`◂ MODERN PUNCH ▸`), A/B compare, Compare, undo/redo, Bypass.
- **Analyzer row** — `INPUT` segmented L/R meters · big **spectrum analyzer** (smooth ink line + grey spectral haze, log-frequency axis — the line treatment carried over from the CRT spectrum work) · `OUTPUT` meters.
- **Module cards** (6) — Dynamics (arc gauge, % Punch), EQ (response line + colored band dots), Saturation (transfer wave), Imaging (polar arc), Stereo Link (venn), Limiter (transfer curve). Each has a power toggle, a mini-visualization, a value, and a Details dropdown.
- **Bottom row** — Loudness (LUFS arc gauge + Short Term / Momentary / Range / True Peak), Loudness History (area graph), Correlation bar + Stereo Field goniometer.
- **Sidechain strip** — Source / Low Cut / High Cut / Listen selectors, a mini sidechain waveform, sample-rate / oversampling readouts, settings.

## Files

| File | Role |
|---|---|
| `index.html` | The entire dashboard: window shell + fixed-size scaler, with one inline `<script type="text/babel">` holding every component (spectrum, meters, gauges, mini-viz, goniometer, App). |

Kept inline (not separate `.jsx`) on purpose — the DS compiler bundles every project `.jsx`, so loose kit files would be double-bundled. Inlining keeps the screen self-contained and load-order-safe.

## Notes

- All graphics are drawn inline (SVG) and use the design-system tokens via `styles.css` (warm palette, category colors, mono font). Lucide (CDN) supplies the few icons, consistent with the rest of the system.
- Cosmetic recreation: meters/curves are static realistic states, not live audio.
- Fixed 1520×1000, scaled to fit the viewport (letterboxed).
