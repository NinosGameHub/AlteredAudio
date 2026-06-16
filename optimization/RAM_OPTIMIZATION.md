# Filter 76 (Prime) — RAM Optimization

**Goal:** reduce plugin RAM footprint from ~90 MB toward ~25 MB.
**Date:** 2026-06-16
**Active editor:** `Prime76Editor` (`Source/FilterPlugin.cpp:200`). `AuroraFilterEditor` is
compiled but never instantiated.

---

## 1. Where the RAM goes

All images are baked into the binary via `juce_add_binary_data` (`CMakeLists.txt:41-48`).
Decoded size = `width × height × 4` bytes (RGBA), which is what lives in RAM once the
editor opens.

| Asset | Dimensions | Disk | Decoded RGBA | In binary? | Used by Prime76Editor? |
|-------|-----------|------|--------------|-----------|------------------------|
| `wear_overlay.png`   | 4096×4096 | 21.6 MB | **64 MB** | yes | **NO — never loaded** |
| `prime_faceplate.png`| 2862×1801 | 4.8 MB  | **19.7 MB** | yes | yes (hero cost) |
| `knob.png`           | 1024×1024 | 1.0 MB  | 4 MB | yes | **NO — never loaded** |
| `prime_knob.png`     | 259×259   | 0.04 MB | 0.3 MB | yes | yes |
| `prime_indicator.png`| 259×259   | ~0      | 0.3 MB | yes | yes |
| glyphs (×39)         | small     | ~0.16 MB| small | yes | yes |

### Key findings
1. **~22.6 MB of dead assets** (`wear_overlay.png` + `knob.png`) are embedded in the DLL
   but the active editor never references them. Confirmed by grepping `BinaryData::`
   usage — only `prime_faceplate`, `prime_knob`, `prime_indicator`, and glyphs are loaded
   (`Source/Prime76Editor.cpp:15-22, 160-165`).
2. **The faceplate decodes to ~19.7 MB** on editor open. It is a `@2x` asset (window is
   1405 px wide; image used out to 2810 px — `Source/Prime76Editor.cpp:527`).
3. The remaining ~30–40 MB is JUCE baseline (`juce_dsp`, `juce_audio_utils`,
   oversampling buffers, C++ runtime).

---

## 2. Action plan

### Fix 1 — Remove dead assets  *(zero visual change, ~22.6 MB off the binary)*
Delete the two unused entries from `CMakeLists.txt:42-43`:

```cmake
juce_add_binary_data(Filter76BinaryData SOURCES
    Resources/prime_faceplate.png
    Resources/prime_knob.png
    Resources/prime_indicator.png
    ${PRIME_GLYPHS}
)
```
(Removed: `Resources/knob.png`, `Resources/wear_overlay.png`.)

**Status:** [ ] pending

### Fix 2 — Downscale the faceplate  *(~15 MB off runtime)*
Re-export `prime_faceplate.png` at `@1x` (≈1405×901).
- Decoded drops **19.7 MB → ~5 MB**; embedded PNG shrinks too.
- Cost: slightly softer plate on HiDPI displays.
- Alternative: keep `@2x` for crisp retina, but then ~25 MB total is not reachable.

**Status:** [ ] pending (requires manual PNG re-export)

### Fix 3 — Release decode when editor closes  *(matters if RAM measured with GUI shut)*
In `~Prime76Editor()` (`Source/Prime76Editor.cpp:292`):
```cpp
juce::ImageCache::releaseUnusedImages();
```
and optionally `juce::ImageCache::setCacheTimeout(1000);` at editor construction.

**Status:** [ ] pending

---

## 3. Expected result

| Stage | Approx. resident RAM |
|-------|----------------------|
| Current | ~90 MB |
| After Fix 1 | ~65–70 MB |
| After Fix 1 + Fix 2 | ~30–35 MB |
| + Fix 3 (editor closed) | ~25 MB |

---

## 4. Notes / follow-ups
- Faceplate is fully opaque, so the alpha channel is wasted RAM — but JUCE always
  decodes PNG to ARGB, so this can't be trimmed without a custom image format.
- If `AuroraFilterEditor` is permanently retired, consider removing it from
  `target_sources` (`CMakeLists.txt:50-57`) to shrink the binary further.
- Re-measure with the DAW's per-plugin meter (or Task Manager on the Standalone build)
  after each fix to confirm the deltas above.
