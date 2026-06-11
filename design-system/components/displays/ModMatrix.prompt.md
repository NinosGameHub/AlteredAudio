`ModMatrix` is the modulation routing grid — mod sources down the rows, destination parameters across the columns, each cell a bipolar depth bar; use it as the plugin's modulation-assignment surface.

```jsx
const sources = [
  { id: "lfo1", label: "LFO 1", color: "var(--cat-modulation)" },
  { id: "env1", label: "ENV 1", color: "var(--cat-dynamics)" },
  { id: "mac1", label: "MACRO 1", color: "var(--cat-filtereq)" },
];
const dests = [
  { id: "freq", label: "Filter Freq" },
  { id: "drive", label: "Drive" },
  { id: "mix", label: "Wet/Dry" },
];
const [vals, setVals] = React.useState({ "lfo1:freq": 0.6, "env1:drive": -0.4 });
<ModMatrix sources={sources} destinations={dests} values={vals} onChange={setVals} />
```

- Drag a cell vertically to set depth (−1..+1); positive fills upward in the source color, negative downward in grey; double-click clears.
- Give each source a `color` (usually a category token) so its routings are scannable across the grid.
