**SpectrumDisplay** — the AF-74's hero graph. A near-black amber CRT showing the live spectrum, the filter response curve, and a draggable cutoff node. It should dominate any full-plugin layout (~40% height).

```jsx
const [f, setF] = React.useState(824);
const [q, setQ] = React.useState(3.2);
<SpectrumDisplay type="LP" freq={f} q={q} slope={12}
  onNodeChange={(n) => { setF(n.freq); setQ(n.q); }} />
```

Props: `type` (LP/HP/BP/NOTCH/PEAK/SHELF), `freq`, `q`, `gain`, `slope`, `showSpectrum`, `showNode`, `live`. Drag the node: X = frequency, Y = resonance. Purely cosmetic — wire the params you already hold in state.
