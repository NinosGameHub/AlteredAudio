`Meter` is the thin segmented level / gain-reduction meter on CRT-black; use it for header I/O VU and the compressor/limiter GR readouts.

```jsx
<Meter value={0.7} label="L" />
<Meter value={0.6} label="R" />
<Meter value={0.4} mode="gr" label="GR" />
<Meter value={0.5} orientation="horizontal" length={120} />
```

- `mode="level"` lights green→amber→orange from the bottom up; `mode="gr"` lights dynamics-orange from the top down.
- Always sits on the near-black CRT surface to match the amber display.
