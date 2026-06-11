`Knob` is Altered Audio's flat Braun/Moog-style rotary — a dirty-cream disc with a near-black indicator line and range dots at 7 & 5 o'clock; use it for any continuous audio parameter.

```jsx
<Knob label="FREQ" value={440} min={20} max={20000} display="440 Hz" />
<Knob label="DRIVE" value={6} min={0} max={24} display="6.0 dB"
      showArc accent="var(--cat-dynamics)" />
<Knob label="GAIN" value={0} min={-24} max={24} display="0.0 dB" disabled />
```

- `disabled` greys the line + disc for a bypassed module.
- `showArc` adds a value-sweep ring; pass the module's category color via `accent` (e.g. `var(--cat-time)`).
- Pair with an uppercase `label` and a mono `display` value to match the native panels.
