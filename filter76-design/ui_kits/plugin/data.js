// Filter 76 UI kit — shared data (plain JS, no JSX).
window.AF_DATA = {
  presets: [
    "ANALOG LOWPASS",
    "VOCAL FORMANT",
    "TAPE HIGHPASS",
    "SLOW SWEEP",
    "NOTCH NARROW",
    "WARM SHELF",
    "ACID 303",
    "MOON DUB BP",
  ],
  filterTypes: ["LP", "HP", "BP", "NOTCH", "PEAK", "SHELF"],
  slopes: ["12", "24", "48"],
  modes: ["ANALOG", "CLEAN"],
  lfoSources: ["OFF", "LFO A", "LFO B", "ENV"],
  destinations: ["FREQUENCY", "RESONANCE", "DRIVE"],
  // v0.7.x: waveform selector is text keys, not glyphs
  waveforms: [
    { id: "sine", label: "SINE" },
    { id: "triangle", label: "TRI" },
    { id: "square", label: "SQR" },
    { id: "random", label: "RND" },
  ],
  version: "v0.7.6",
};
