**PeakMeter** — segmented peak meter with built-in hold: the segment at the recent peak **stays lit** (holds ~1.4s, then decays) — classic LED peak hold, no separate marker. Use vertical for channel I/O meters, horizontal for L/R strips. Segments above `warnAt` (default 0 dB) light warning-orange.

```jsx
// Vertical input meter, −24..+24 dB
<PeakMeter label="IN" value={levelDb} min={-24} max={24} length={300}
  scale={[24, 12, 0, -12, -24]} />

// Center-split L|R butterfly: L fills ← from center, R fills → (quiet at center)
<PeakMeter orientation="horizontal" reverse value={lDb} min={-24} max={6} length={390} />
<PeakMeter orientation="horizontal" value={rDb} min={-24} max={6} length={390} />
```

Props: `value` (dB), `min`/`max`/`warnAt`, `orientation`, `reverse` (horizontal: fill right→left for the L half of a butterfly pair), `length`/`thickness`/`segments`, `scale` (loud→quiet for vertical), `holdMs`/`decayDbPerSec`/`showHold`. Purely cosmetic — drive `value` from your state loop.
