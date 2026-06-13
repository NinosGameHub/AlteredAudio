**SpectrumDisplay** — the Filter 76 hero graph: a clean response view on near-black warm glass. Sparse dB grid (12/0/−12/−24), the filter response as a crisp glowing curve over a soft gradient fill, a draggable cutoff node, and an optional dimmer **ghost curve** marking the modulated / compared state. Floats on `--shadow-float`.

```jsx
const [f, setF] = React.useState(800);
const [q, setQ] = React.useState(6.0);
<SpectrumDisplay type="LP" freq={f} q={q} slope={24}
  showSpectrum={false}
  ghost={{ freq: 2000 }}            // dimmer modulated-target curve + dot
  onNodeChange={(n) => { setF(n.freq); setQ(n.q); }} />
```

Props: `type` (LP/HP/BP/NOTCH/PEAK/SHELF), `freq`, `q`, `gain`, `slope`, `ghost` (partial param override, e.g. `{freq}`), `showSpectrum` (live analyzer fill — off in the v0.7.x plugin), `showNode`, `live`. Drag the node: X = frequency, Y = resonance. Purely cosmetic — wire the params you already hold in state.
