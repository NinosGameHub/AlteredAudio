# Session Log — June 11, 2026 (v0.2.4 → v0.7.0)

Working notes from the Claude Code session. Resume the full conversation with
`claude --continue` in this folder.

## EQ overhaul
- Removed draggable nodes from the EQ curve; added 8 numbered band-select buttons.
- Slope (6/12/18/24/48 dB/oct) now REAL in audio: Butterworth cascades in `EQModule`
  (was UI-only before). Q drives the resonant section.
- Curve display now draws the EXACT transfer function (`EQModule::bandResponseDb`,
  shared math with the audio path via `BiquadFilter::calcCoefficients`/`magnitudeDb`).
- BandPass/Notch/AllPass fixed; gain knob only shows for Peak/shelves; slope only for LP/HP.

## Engine improvements
- Host latency reporting (PDC): `EffectModule::getLatencySamples()`, summed by
  `ModuleChain`, updated dynamically (oversampling + compressor lookahead).
- Waveshaper & Clipper: dry path aligned through fractional `dsp::DelayLine`
  (no comb filtering on partial mixes); drive/wet-dry smoothed.
- Delay: tempo sync (14 note divisions, host BPM), independent L/R times,
  spread param removed, ping-pong button moved (no overlap), 50 ms time glide.

## Plugin packaging
- All 16 plugins (rack + 15 single-module) install to
  `C:\Program Files\Common Files\VST3\AlteredAudio\` via `tools\install-vst3.ps1`
  (self-elevating, rename-replace for DAW-locked files). Old root-level rack removed.

## Aurora Filter (standalone AlteredAudio Filter)
Custom editor per `TEMP\Filter\Aurora Filter Design Specification.pdf`
(user note: 44.1/48 kHz only, not 96):
- 1400x900 resizable (min 1100x700), exact spec palette, cream knobs + black indicator.
- Display: live FFT spectrum + exact response curve + draggable node
  (drag = freq/res, wheel = drive, double-click = reset). Ghost curve shows the
  LIVE modulated response when LFO/ENV active.
- DSP added to `FilterModule`: drive 0–24 dB (tanh, clean at 0), 12/24 dB slopes,
  mix, output trim. Control-rate smoothing (16-sample coefficient updates,
  multiplicative freq glide) — fixed loud low-freq cracks from coefficient stepping.
- Modulation: source OFF/LFO A/LFO B/ENV → dest FREQ/RES/DRIVE with amount.
  Amount semantics: +amount = up only, −amount = down only, 0 = bipolar (depth sets range).
  ENV SENS knob shows 0–10, maps to 0–4 actual.
- Tempo sync: right-click RATE / ATTACK / RELEASE knobs → FREE / TEMPO SYNC
  (note divisions 1/1…1/32 D/T, host BPM).
- Node/mouse mapping fixed (node sits exactly under cursor; closed-form Q=10^(dB/20)).
- All readouts one decimal.
- Working: 6 factory presets, A/B compare, power button, CPU/sample-rate footer.

## Known gaps / next steps
- Aurora: no oversampling DSP (footer shows 1x), MONO/MID-SIDE not implemented,
  fonts approximated (Eurostile/IBM Plex not bundled).
- Other 14 single-module plugins still use the generic editor.
- LFO is block-rate (fine ≤ ~10 Hz; could move to control-rate inside module).

## Build & install workflow (standing rule)
1. Bump version in `CMakeLists.txt` (both `project()` and `PLUGIN_VERSION`).
2. `cmake --build build --config Release --target <target>_VST3`
3. `powershell -ExecutionPolicy Bypass -File tools\install-vst3.ps1`
