`EQCurve` is the EQ module's frequency-response display — a CRT-black graph with a log-frequency grid, a faint amber spectrum, the summed response curve in Filter/EQ blue, and a draggable handle per band; use it at the top of the EQ panel.

```jsx
const [bands, setBands] = React.useState([
  { freq: 80,   gain: 2,  q: 0.7, type: "LowShelf",  on: true },
  { freq: 1200, gain: 4,  q: 0.8, type: "Peak",      on: true },
  { freq: 6000, gain: -3, q: 1.2, type: "Peak",      on: true },
]);
<EQCurve bands={bands} onChange={setBands} />
```

- Drag a handle horizontally for FREQ, vertically for GAIN — `onChange` returns the updated bands.
- `on: false` greys a band's handle and drops it from the summed curve.
- Pairs with the EQ panel's 8-band knob grid; keep the curve and knobs reading the same band state.
