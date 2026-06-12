# Session Log — June 12, 2026 PM (v0.8.0 → v0.8.5) — Filter 76 wrap-up

Working notes from the Claude Code session. Filter 76 is paused here;
next session moves to **Gain 76**. Resume with `claude --continue`.

## Shipped versions
- **v0.8.0** — Preset manager: save/browse/load user presets as
  `~/Documents/AlteredAudio/Filter 76/Presets/*.f76preset`, inline SAVE
  overlay, popup browser with FACTORY/USER sections, combined prev/next
  navigation across factory + user.
- **v0.8.1** — Dark knob readouts; full disabled-state for inactive
  modulation params (dst dropdown, wave buttons, all dependent knobs).
- **v0.8.2** — Design sync vs filter76-design: header 64px, panel radii
  10px, OptionList density, face-knob fonts, scope sizes. Tester zip
  created in `dist/Filter76-v0.8.2-win64/` (README.md + INSTALL.txt +
  install.ps1 + zip) — **zip still contains the v0.8.2 DLL**.
- **v0.8.3** — MIX header readout is click-drag (HeaderDragger component,
  double-click resets), DRIVE·DB face label narrowed, PopupMenu header +
  TextEditor colors pinned dark.
- **v0.8.4** — Layout bugfixes (OVERSAMPLING overlap, SHELF overflow,
  MODE/48dB overlap). Version had been stuck at 0.8.0 in CMakeLists —
  bumped so the footer can prove which binary the DAW loaded.
- **v0.8.5** — Final display design (below), bigger face labels,
  OVERSAMPLING/MIX on the control row with captions underneath.

## Display design — FINAL after many iterations (do not regress)
- Even dark background (`#171511 → #11100D` linear). NO fixed radial glow,
  NO CRT vignette — all static lighting explicitly rejected by the user.
- **Node spotlight**: warm amber radial glow (curveGlow 0.20 → 0, radius
  0.85 × h) centered on the node, follows it while dragging. The rule that
  settled it: *light may come from the node only, never from a fixed point
  on the glass*.
- Response scale **+12 … −30 dB**, grid + labels at 6 dB steps to −24.
- Analyzer: honest dBFS scale (0 top / −78 bottom — removed a hidden
  +18 dB visual boost that painted music above the 0 dB line), fill
  quieted to 0.28/0.11, stroke brushed with a vertical gradient that stays
  strong to ~93% height and dissolves only at the very bottom (HF content
  stays visible; no jittery floor line), slow exponential release
  (`(db+100)*0.025`/frame min 0.10), whole layer fades with the signal.

## Design system synced both ways
- Local `filter76-design/` AND the claude.ai project (id
  `83b75076-44a8-41a3-8c95-6789d7d779d6`) updated via DesignSync:
  SpectrumDisplay.jsx (scale, background, spotlight, analyzer stroke),
  Knob.jsx (face label 6.2 units / 0.3 tracking). Changelog in
  `filter76-design/SYNC-NOTES.md`.

## Process rules established (user-mandated, saved to memory)
1. **Every build gets a version bump** (both lines in CMakeLists.txt).
2. **Build only the Filter 76 target** (`--target AlteredAudio_Filter_VST3`)
   — not the whole stack — until told otherwise. Same applies per-module
   (e.g. `AlteredAudio_Gain_VST3` for Gain 76 work).
3. Always build + install after every change without being asked; verify
   installed DLL timestamp matches the build artefact.
4. DAWs cache loaded VST3s — full DAW restart needed; footer version is
   the proof of which binary is loaded.

## Gotchas hit this session
- `Copy-Item` to Program Files fails silently/partially without elevation;
  the install script's `Start-Process -Verb RunAs` works from the
  interactive session (sandbox disabled) but NOT as a background task.
- MSBuild sometimes skips recompiles after rapid successive edits —
  deleting `AuroraFilterEditor.obj` forces it.
- When the user says "make it look like before" about ONE element, do not
  revert neighbouring elements with it.

## Where Gain 76 starts
- Existing minimal `AlteredAudio_Gain` target (generic editor) in CMakeLists.
- `TEMP/Gain/` exists untracked in the repo root — inspect first.
- `gain` branch exists on origin (created June 12 AM).
