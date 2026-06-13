**Meter** — segmented amber level meter for the I/O readouts.

```jsx
<Meter title="OUTPUT" scale={[24,12,0,-12,-24]}
  channels={[{value:0.72,label:"L",readout:"-1.2"},{value:0.68,label:"R",readout:"-1.4"}]} />
```

Props: `channels` (each `{value 0..1, label, readout}`), `scale` (dB labels at left), `segments`, `height`, `showReadout`, `title`. Top ~10% of segments light orange as an over warning.
