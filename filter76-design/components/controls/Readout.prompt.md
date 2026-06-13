**Readout** — the dark inset box with amber monospace digits. Use it anywhere a value is *displayed* (not turned): the preset name, oversampling badge, mix %, meter dB numbers, header status.

```jsx
<Readout label="OVERSAMPLING" value="4x" size="sm" />
<Readout value="001  ANALOG LOWPASS" align="left" />
```

Props: `size` (sm/md/lg), `tone` ("amber" default, "muted" dimmed), `label` + `labelPlacement` ("left"/"top"/"none"), `align`. For knob values prefer the `Knob`'s built-in `display` instead of a standalone Readout.
