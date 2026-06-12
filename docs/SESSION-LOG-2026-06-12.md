# Session Log — June 12, 2026 (v0.7.0 → v0.7.9)

Working notes from the Claude Code session. Resume the full conversation with
`claude --continue` in this folder.

## Repo / branches
- Pushed the 3 pending v0.7.0 commits to origin/master.
- Created and pushed one branch per module — `filter`, `chorus`, `clipper`,
  `compressor`, `delay`, `eq`, `flanger`, `gain`, `gate`, `limiter`, `phaser`,
  `reverb`, `spatial`, `transient-shaper`, `waveshaper` — plus `strip` for the
  full rack. All Filter 76 work happens on `filter`.
- NOTE: the git remote URL embeds a GitHub PAT in plaintext — consider moving
  to a credential manager.

## Rename: Aurora Filter → Filter 76 (v0.7.1)
- DAW product name is now "AlteredAudio Filter 76" (plugin code AFlt unchanged
  so old sessions still load). Header reads FILTER 76 / MODEL AF-76 MK III.
- Internal class/file names still say Aurora (`AuroraFilterEditor.*`).
- `tools/install-vst3.ps1` now removes the old "AlteredAudio Filter.vst3"
  install (renamed-products cleanup list).
- Gotcha: the install script is ANSI-parsed by PS 5.1 — em-dashes inside
  double-quoted strings break parsing; keep new strings ASCII.

## Filter 76 design system pulled (claude.ai/design)
- Project "Filter 76 Design System" (id 83b75076-44a8-41a3-8c95-6789d7d779d6)
  mirrored into `filter76-design/` via DesignSync: readme spec, SKILL.md,
  tokens, all 13 component JSX + prompt.md, full plugin UI kit.
- Skipped generated artifacts (`_ds_bundle.js`, `.d.ts`, card.html previews,
  uploads) — see `filter76-design/SYNC-NOTES.md`.
- Look: cream ABS #E6E0D2, single amber accent #D08A2E, amber CRT #F0B547 on
  #171717, Michroma (Eurostile substitute) + IBM Plex Mono, type-only wordmark.

## Design implemented in the JUCE editor (v0.7.2)
- Knobs: flat matte-cream puck, floating tick ring (majors per quarter, bold
  top), bold value tick + glowing amber dot; 5 hero knobs print value on the
  face, unit folded into the small label (FREQ · HZ), no text box.
- Display: Pro-Q idiom — radial dark glass, amber log grid, gradient-filled
  analyzer, glowing response curve, round draggable node.
- FILTER TYPE / SLOPE as vertical LED option lists; segmented amber meters;
  cream header with ALTERED AUDIO / FILTER 76 wordmark, numbered preset
  readout (001 INIT), round PowerKey; warm beige footer with SYSTEM LED.

## Feature work (v0.7.3 – v0.7.7)
- v0.7.3:
  - 48 dB/oct slope — 8th-order Butterworth (4 biquads, Qs 0.5098/0.6013/
    0.9000/2.5629, user Q scales the last); `kMaxSections` now 4.
  - `filter_mode` param: ANALOG = tanh drive (default), CLEAN = drive stage
    bypassed entirely.
  - LFO mod amount 0 now means NO modulation (old "0 = bipolar" removed —
    user explicitly wants never-bidirectional at 0).
  - Hero knob face: value 20% smaller, label bigger. GAIN knob moved to the
    right edge of the hero knob panel.
- v0.7.4: high-res analyzer — FFT 2048→8192, continuously-written ring buffer
  snapshotted every UI frame (overlapped), fractional-bin interpolation in the
  lows, 160→400 points, 3-tap spatial smoothing, ~30 dB/s release.
- v0.7.5: MODULATION panel uses styled dropdowns (SOURCE / DESTINATION combo
  boxes per design Select; popup = cream bg + amber highlight); AMOUNT readout
  signed (+37.0 %).
- v0.7.6: disabled-knob state per design (dim ticks, no value tick, grey dot
  #6E685C). Auto-disable: GAIN unless PEAK/shelf; DRIVE in CLEAN mode; AMOUNT
  when source OFF; RATE/DEPTH/PHASE unless LFO; ATTACK/RELEASE/SENS unless ENV.
- v0.7.7:
  - Real 4x/8x oversampling: juce::dsp polyphase IIR halfband around the
    module; module re-prepared at the oversampled rate; integer latency into
    host PDC via new `SingleModuleProcessor::getExtraLatencySamples()` hook.
    Header OVERSAMPLING dropdown (`filter_oversampling`); footer shows live
    oversampling + latency ms.
  - Analyzer gets its own +12..−90 dB scale (response graph keeps ±24).
  - Face label: tighter tracking, narrower, clear of the amber dot.
  - Right-click FREE/TEMPO SYNC menus now use the plugin LookAndFeel
    (`menu.setLookAndFeel(&lnf)` — standalone PopupMenus don't inherit it).

## Bug fixes (v0.7.8 – v0.7.9)
- "Nervous line" at the display bottom = noise floor (−82 dB) jittering inside
  the deeper analyzer scale.
- v0.7.8 per-point silence gate looked WORSE (holes in the curve while
  playing) — reverted.
- v0.7.9 final: whole-spectrum gate — if max(spectrum) < −70 dB draw nothing;
  otherwise draw the full continuous curve exactly as before.

## Known gaps / next steps
- Aurora→Filter 76 internal renaming (classes/files) not done.
- MONO/MID-SIDE signal path, SOLO, 36 dB slope from the design kit: not in C++.
- Fonts still system substitutes (Michroma / IBM Plex Mono not bundled).
- Footer SIGNAL PATH is static "STEREO".
- Commit 65a321d has a stray CJK char in its message (history rewrite blocked).
- Other 14 single-module plugins still use the generic editor.

## Build & install workflow (standing rule)
1. Bump version in `CMakeLists.txt` (both `project()` and `PLUGIN_VERSION`).
2. `cmake --build build --config Release --target AlteredAudio_Filter_VST3`
3. `powershell -ExecutionPolicy Bypass -File tools\install-vst3.ps1`
   (self-elevating; verify the installed binary timestamp afterwards).
