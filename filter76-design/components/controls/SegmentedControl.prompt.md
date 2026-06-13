**SegmentedControl** — a horizontal row of mutually-exclusive options where the active one fills amber. Use for A/B compare, signal-path MONO/STEREO/MID·SIDE, and the display tabs.

```jsx
const [path, setPath] = React.useState("STEREO");
<SegmentedControl value={path} onChange={setPath} options={["MONO","STEREO","MID/SIDE"]} />

// On the dark spectrum display, the selected tab fills cream instead:
<SegmentedControl tone="display" value={tab} onChange={setTab}
  options={["SPECTRUM","RESPONSE","NODE"]} />
```

Props: `tone` ("panel" default / "display"), `size` ("sm"/"md"). For a vertical list with LED indicators (FILTER TYPE, SLOPE, MODE) use `OptionList` instead.
