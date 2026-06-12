**OptionList** — the vertical, LED-marked selector stack. Use for FILTER TYPE, SLOPE and MODE — anywhere the AF-74 shows a column of options with a lit amber LED on the chosen row.

```jsx
const [type, setType] = React.useState("LP");
<OptionList value={type} onChange={setType}
  options={["LP","HP","BP","NOTCH","PEAK","SHELF"]} />
```

Props: `dense` (tighter rows), `disabled`. For horizontal exclusive choices use `SegmentedControl`.
