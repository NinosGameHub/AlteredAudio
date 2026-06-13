**Oscilloscope** — the small amber trace box. Use `waveform` for the LFO ENGINE preview and `waveform="envelope"` for the ENVELOPE FOLLOWER display.

```jsx
<Oscilloscope waveform="sine" cycles={3} />
<Oscilloscope waveform="envelope" height={72} />
```

Props: `waveform` (sine/triangle/square/sawtooth/random/envelope), `cycles`, `live`, `height`.
