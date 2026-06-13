**Knob** — the Filter 76 hero dial, matching the AB·VALUE reference: a large flat matte-cream puck (thin dark edge, faint top light, soft shadow) with a ring of long fine ticks floating outside it, a **bold tick that tracks the value**, and a small glowing **amber dot** on the face aligned with it. Hero knobs print the value big (light weight, number only) with the unit folded into the small tracked label — `1563` / `FREQ · HZ`.

```jsx
// Hero filter knob — value on the face, unit in the label
<Knob face label="FREQ · HZ" value={freq} min={20} max={20000} defaultValue={1000}
  display={Math.round(freq)} onChange={setFreq} />

// Compact knob — label above, dark readout below
<Knob label="RATE" size="var(--knob-md)" ticks={28} value={rate} min={0.01} max={20}
  defaultValue={1} display={rate.toFixed(2) + " Hz"} onChange={setRate} />
```

Variants & props:
- `face` — value (big, number only) + label (small, tracked, unit folded in: "FREQ · HZ") print on the disc. Use `var(--knob-lg)` (≈132px) or larger.
- `size` — `var(--knob-lg)` hero, `var(--knob-md)` compact (pass `ticks={28}` at small sizes).
- `accent` — recolor the indicator dot (defaults to amber).
- Drag vertically (hold Shift = fine), scroll to nudge, double-click → `defaultValue`.
- `showRing` is deprecated — the value tick + amber dot are the indicator.
