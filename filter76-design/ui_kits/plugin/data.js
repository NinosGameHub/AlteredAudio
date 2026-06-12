// Aurora Filter UI kit — shared data (plain JS, no JSX).
window.AF_DATA = {
  presets: [
    "ANALOG LOWPASS",
    "VOCAL FORMANT",
    "TAPE HIGHPASS",
    "RESONANT SWEEP",
    "NOTCH NARROW",
    "WARM SHELF",
    "ACID 303",
    "MOON DUB BP",
  ],
  filterTypes: ["LP", "HP", "BP", "NOTCH", "PEAK", "SHELF"],
  slopes: ["12", "24", "36", "48"],
  modes: ["ANALOG", "CLEAN"],
  lfoSources: ["OFF", "LFO A", "LFO B", "ENV"],
  destinations: ["FREQUENCY", "RESONANCE", "DRIVE"],
  waveforms: ["sine", "triangle", "square", "sawtooth", "random"],
};

// Small procedural waveform glyphs for the LFO selector — drawn in the
// plugin's own line-glyph idiom (matches the Select chevron / scope trace).
window.AF_WAVE_GLYPH = function (kind, on) {
  const stroke = on ? "var(--text-on-accent)" : "var(--text-secondary)";
  const paths = {
    sine: "M2 9 Q 6 1, 10 9 T 18 9",
    triangle: "M2 13 L 6 4 L 10 13 L 14 4 L 18 13",
    square: "M2 13 L2 5 L7 5 L7 13 L12 13 L12 5 L18 5",
    sawtooth: "M2 13 L7 4 L7 13 L12 4 L12 13 L17 4",
    random: "M2 10 L5 10 L5 5 L9 5 L9 12 L13 12 L13 7 L18 7",
  };
  return { d: paths[kind] || paths.sine, stroke };
};
