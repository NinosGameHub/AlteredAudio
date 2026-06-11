`Select` is the flat combo box — white fill, hairline border, grey chevron; use it for discrete mode/type choices (filter type, distortion algorithm, oversampling).

```jsx
<Select
  label="TYPE"
  value={type}
  onChange={setType}
  options={["LowPass", "HighPass", "BandPass", "Notch", "Peak"]}
/>
```

- Pass `options` as strings or `{value, label}` objects.
- The optional uppercase `label` matches the knob-label treatment for aligned rows.
