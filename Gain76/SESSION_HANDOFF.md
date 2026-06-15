# Gain 76 — Session Handoff & Working Principles
_Last updated: 2026-06-14. Read this first next session — it tells you where the GUI is and HOW we work._

---

## 0. TL;DR — where we are right now
The Gain 76 GUI is being rebuilt from Blender-rendered sprites. As of end of this session, the plugin builds clean (VST3 + Standalone) and has:
- Brass **faceplate**, **GAIN 76** + **ALTERED AUDIO** wordmarks (all Blender sprites).
- **Knob**: brass body with a **tick-mark dial scale**, a **rotating LED dot + amber glow** (tracks value), and a **spinning body-grain overlay** (two-pass).
- Two vertical **meters** (housings + amber segment strips, level-coloured).
- **Info line** = NEW **5-cell vintage display panel** (brass bezels + dark recessed screens). The **center screen** shows the live **amber dB readout** (embossed-style amber glyph sprites), with a **clip-triggered glitch** animation. The 4 side screens are empty (reserved for PEAK/LUFS/MODE/OVERSAMPLING).

**Immediate next candidates:** wire readouts into the 4 empty side screens; tune amber screen glow; embed assets as BinaryData; optionally enlarge center readout.

---

## 1. Project facts
- **Code:** `C:\Dev\Gain76` (own JUCE copy, own `build/`, **local-only, never committed there**). Sources: `Source\GainEditor.cpp/.h`, `GainProcessor.cpp`. The LIVE editor class is `GainEditor`; look-and-feel is `Gain76LookAndFeel` (both in GainEditor.*). Dead/ignore: `PluginEditor.h`, `AlteredAudioLookAndFeel.h`.
- **Version** 1.1.0, **canvas 720×720** square. Targets `Gain76_VST3`, `Gain76_Standalone`.
- **Build:** `cmake --build "C:/Dev/Gain76/build" --config Release --target Gain76_VST3 Gain76_Standalone`
  - The Standalone **exe locks while running** → always `Stop-Process -Name 'AlteredAudio Gain 76' -Force` before building.
  - In the Bash tool, backslash paths get mangled (`C:\Dev` → `C:Dev`); **use forward slashes** in quotes.
- **Git rule:** commits go to the **`gain` branch** of `C:\Users\ninov\OneDrive\Documents\AlteredAudio`. The `C:\Dev\Gain76` working copy (sources, JUCE, build) is still never committed, BUT as of 2026-06-15 the **regenerated knob sprites are now mirrored into the repo at `Gain76/Assets/Export/`** (`knob.png`, `knob_grain.png`, `body_detailed.png`, `body_flat.png`) and tracked — copy updated PNGs there before committing.
- **Assets:** `C:\dev\gain76\Assets\Export\*.png`, full-frame **@2x = 1440×1440**. `loadAsset()` now checks **`C:/Program Files/Altered Audio/Gain 76`** first (the installed location), then falls back to `C:/dev/gain76/Assets/Export` (dev tree). **TODO before release: embed as BinaryData.** Backups live in `Assets\Export\_backup_*` (latest `_backup_20260615`).
- **Finished package + installer:** `…\AlteredAudio\Finished_Plugins\Gain_76\Material_UI\` (Source, Assets, Blender, Build, `HANDOFF.md`, `Installer\Gain76_Setup.exe`). The installer (self-contained C# stub, payload embedded) deploys the VST3 → `Program Files\Common Files\VST3\AlteredAudio` and assets → `Program Files\Altered Audio\Gain 76`.
- **⚠️ Security:** the repo's `origin` remote URL has a **GitHub PAT embedded in plaintext** (`ghp_…`) — rotate it and switch to a credential helper/SSH.

---

## 2. The Blender ↔ JUCE pipeline (how rendering works)
- A live Blender instance hosts the design scene and listens on **localhost:9876** (the blender-mcp add-on socket).
- **Talk to it directly** (the MCP server is often NOT loaded in-session): 
  `python C:\Users\ninov\blender_cmd.py <type> '<json-params>'`
  - Types: `get_scene_info`, `get_object_info`, `execute_code`, `get_viewport_screenshot` (needs a `filepath` param).
  - For long scripts: `execute_code` with `exec(open(r'C:/Users/ninov/<script>.py').read())`.
  - The client uses a **600 s socket timeout** because renders block.
- **Camera/render:** ortho cam at (0,0,10) looking −Z, **ortho_scale 7.9**, **1440×1440**, `film_transparent` PNG/RGBA, Cycles ~48 samples (~20–35 s/render).
- **Coordinate math** (essential for placing JUCE overlays on rendered art):
  - **182.3 px per world-unit** in the 1440 frame (1440/7.9). Canvas(720) = frame/2.
  - `x_frame = 720 + x_world*182.3`, `y_frame = 720 − y_world*182.3` (Blender Y up → screen Y down). `canvas = frame/2`.
- **Isolation render** (the core method): to render one asset, set `hide_render=True` on **all** MESH+FONT objects except the target set, keep the lights as-is, render, then restore. Always-hidden helpers: `Emboss_Left.002`, `Indicator_Dot` (now used), `Fill`, `LED_Light` (LED off for non-indicator renders so its hotspot isn't baked in).
- **Full-frame sprites:** every object renders in its true position; JUCE either blits the whole frame (faceplate/infoline/text) or crops a region by component bounds (knob/meters). Don't move objects to center them.

---

## 3. Current asset → Blender object mapping
| Asset PNG | Blender object(s) |
|---|---|
| faceplate.png | `Faceplate` |
| knob.png | `Groove_Ring` + `Knob_Body` + `Knob_Rim` + `Knob_Dial_Ticks` (drawn STATIC) |
| knob_grain.png | offline-built from `body_detailed` render (see §4) |
| indicator.png | `Indicator_Dot` rendered **with `LED_Light` ON** |
| meter_housing_left/right.png | `Left_Meter_Housing.001` / `Right_Meter_Housing.002` |
| meter_strip_left/right.png | `MeterStrip.002` / `MeterStrip.003` (live ones; `.001/.004` are old hidden copies) |
| infoline_housing.png | **all visible meshes with bbox-centre y < −1.8** = 6 housings (`Botom_InfoLine_Housing.001/.002/.003/.004/.005/.006`) + 6 screen plates (`Screen_plate`, `.001`–`.005`) → 5-cell panel |
| digits/d0–d9, dot, plus, minus | amber-emissive glyphs (see §5) |
| text_altered_audio.png / text_gain76.png | `Text_AlteredAudio` / `Text_Gain76` |

Blender "numbers" **collection** holds embossed-brass glyph objects `Num_0…Num_9, Num_Dot, Num_Plus, Num_Minus` (all `hide_render=True`, organisational).

**Knob_Body material** = `Dull_Brass.003` — a hybrid: PBR image set (`dull-brass_albedo/metallic/roughness/normal/ao`) + procedural overlays. The camera-facing face pattern comes from: blotchy **color mottling** (`Noise Texture.002` scale 5 → `Color Ramp` → `Mix (Legacy).003` fac 0.12 → Base Color) and a fine **roughness speckle** (`Voronoi Texture` scale 18 + `Noise Texture.001` scale 40 → `Mix (Legacy).001/.002` fac 0.15 → Roughness); brushed grain = `Normal Map` strength 1.0. All nodes now carry descriptive **`.label`s** (e.g. `BASECOLOR mottle mix (amount)`, `MOTTLE noise (size)`, `ROUGHNESS speckle mix (amount)`, `BRUSHED bump (strength)`) so they're findable in the shader editor. **Note:** the cloudy face mottle is largely baked into the *source images* (`dull-brass_albedo`/`roughness`), so zeroing the procedural mixes alone does NOT remove it. To flatten the face, two `MixRGB` nodes were added — **`FLATTEN basecolor (amount)`** (into Base Color) and **`FLATTEN roughness (amount)`** (Color2 = `0.4` grey, into Roughness, Factor 0.6). The body cap was then **colour-matched to the outer ring**: ring renders sRGB ~(137,108,63), so FLATTEN basecolor Color2 = solid brass linear, then brightened (15%,15%,10%,15%) to **`(0.359,0.213,0.062)`** at **Factor 1.0** → body renders ~(169,132,78), a lit cap brighter than ring (137,108,63). Latest state: **base-colour noise mottle on** (`Mix (Legacy).003` 0.10, `FLATTEN basecolor` Factor 0.80, Color2 = linear **`(0.287,0.170,0.050)`**) + a **faint roughness blotch** (`Mix (Legacy).002` 0.019, `FLATTEN roughness` 0.75). Body renders ~(166,131,80) — back near the ring's matched tone (137,108,63 ring / cap intentionally a bit brighter). Net = near-smooth brass cap with subtle base-colour noise + faint sheen micro-variation. Brushed `Normal Map` strength 0.21; speckle scale `Noise Texture.001` 140 / `Voronoi` 36. (Voronoi has no `Smoothness` input in its current feature mode — don't try to set it.) **Not yet propagated to the production `knob.png`/`knob_grain.png` assets or rebuilt** — re-render + rebuild to see it in the plugin.

---

## 4. Component state & key JUCE code (in `drawRotarySlider` unless noted)
- **Knob layers (draw order):** (1) static base `knob.png`; (2) `knob_grain.png` grain overlay rotated by `rot`, clipped to body cap (r≈0.245·w), opacity 0.88; (3) `indicator.png` LED dot rotated by `rot` with a procedural amber glow under it (two radial blobs: halo 0.105·w @0.34, core 0.052·w @0.60, at orbit radius 0.203·w — kept TIGHT so it doesn't pool on the dark groove).
  - `rot = (startAngle + sliderPos·(endAngle−startAngle)) − π/2` (glyphs/indicator rendered at 3-o'clock baseline). 0 dB → up, −24 → lower-left, +24 → lower-right, matching the tick dial.
- **Tick dial** = `Knob_Dial_Ticks` (bmesh, 9 major + 8 minor dark `Dial_Ink` bars on the flat skirt, matches JUCE default rotary sweep 1.2π–2.8π).
- **Two-pass grain spin** (`knob_grain.png`): render `Knob_Body` detailed (`body_detailed.png`) + a flat tone-matched base (`knob.png` body smoothed), then `make_grain2.py` makes the overlay = full detailed body, feathered at the cap edge, alpha = body silhouette. Strength = the 0.88 opacity. (Subtle high-pass version `make_grain.py` looked flat — replaced.)
- **dB readout** (`valueDisplay_`, a `PaintDelegate` child of `content`): canvas bounds **(321, 594, 78, 38)** = the info-line center dark screen. Draws the signed value `gainKnob.getTextFromValue(getValue())` using **amber glyph sprites** in a fixed 5-cell monospace grid (`cellW = (W−8)/5 * 0.95` — the ×0.95 gives side margin so wide values like "−15.0" don't crowd). One uniform src cell `(688,664,64×63)` crops each glyph (all centred x=720, baseline y≈721). Repainted each timer tick.
- **Glitch** (clip-triggered, medium): in `timerCallback`, when output peak `pk ≥ 1.0` (0 dB) fire `glitchT_ = kGlitchDur (0.30s)` on rising edge (+15% chance to re-fire while sustained). In `valueDisplay_.onPaint`, while `glitchT_>0`: render glyphs to an offscreen layer, then RGB-split ghosts (red +dx / cyan −dx via alpha-mask tint), torn horizontal bands, flicker, occasional scanline. State: `glitchT_`, `wasClipping_`, `glitchRng_`.
- **Meters** (`VerticalMeter`): housing + amber strip used as an exact alpha mask; recoloured by dB zone (amber < −9, orange −9..0, red ≥0). `glowLayer_` casts soft colour onto the brass.

---

## 5. Helper scripts (all in `C:\Users\ninov\`)
- `blender_cmd.py` — socket client to Blender (THE workhorse).
- `blender_render.py` — re-render all 9 base assets isolated.
- `blender_digits.py` / `blender_digits_amber.py` / `blender_digits_brass.py` — render the 0-9(+ . + −) glyph set via a temp origin proto (Michroma, extrude 0.0175, bevel 0.004/res2, align_x CENTER + **align_y BOTTOM_BASELINE**, size 0.62). Amber = material `LED_Amber` (emission (1,0.4,0.05)); brass = `Dull_Brass.002`. **Currently the digits/ folder holds the AMBER set** (for the dark screen).
- `blender_letters_amber.py` — renders the amber **A–Z** glyph set (same proto/material/size/baseline as `blender_digits_amber.py`) to `Assets/Export/letters/A.png…Z.png`. Lets the plugin spell words (STEREO/MONO/…) from sprites instead of a font. Editor loads them into `letterImg_[26]`, measures each glyph's tight L/R bounds within the shared baseline band (y 664–727), and `drawAmberWord()` lays them out proportionally, centred, width-fitted to the plate. Used by the STEREO/MONO mode screen.
- `blender_numbers_folder.py` — builds the "numbers" collection of brass glyph objects.
- `blender_letters_folder.py` — builds the "letters" collection of brass A–Z glyph objects (organisational, `hide_render=True`), analogous to "numbers".
- `make_grain2.py` — builds `knob_grain.png` (the live grain overlay).
- `feather_knob.py` — post-process on `knob.png` that feathers the outer alpha (full to r=240, →0 by r=262) so `Knob_Rim`'s bright beveled edge stops reading as a pale "white-washed" ring against the faceplate. **Run AFTER `blender_render.py`** (which regenerates `knob.png`). Full knob pipeline: `blender_render.py` → `feather_knob.py` → `make_grain2.py`.
- `blender_screen_glow.py` — builds the amber glowing-screen material on `Screen_plate` (`SCR_GlowFalloff` ramp = glow size, `SCR_Strength` = brightness, `SCR_Grain`, `SCR_Scan` scanlines; emission color on the Principled BSDF). NOTE: nodes were given `.label`s so they're findable in the editor.
- `blender_screen.py` / `blender_revert_screen.py` — the inset-screen experiment on `.004` and its revert (both reverted; not in use).

---

## 6. WORKING PRINCIPLES (how we do things — keep doing these)
1. **Verify everything visually.** After any Blender render or JUCE change, render/crop with PIL and `Read` the image, and screenshot the running plugin. Never claim it works without looking.
2. **Screenshot the running plugin** via PowerShell: find process `AlteredAudio Gain 76`, `SetWindowPos` topmost (flags `0x0041`), `GetWindowRect`, `CopyFromScreen`, save PNG; then crop with PIL. (Define the `Win` P/Invoke type fresh in each PowerShell call — shell state doesn't persist.)
3. **Standalone mutes audio**, so meters/glitch/value won't move on their own. To verify a specific value/state, **temporarily inject** it (e.g. `gainKnob.setValue(-15.0, dontSendNotification);` or force `glitchT_`), screenshot, then **remove the temp line and final-build**. Mark temp lines `// TEMP`.
4. **Back up before overwriting** any asset PNG (`Assets\Export\_backup_*`).
5. **Measure, don't guess** positions: render the target object isolated, get its alpha bbox, convert frame→canvas with the 182.3 px/unit math, and use that rect for the JUCE component.
6. **sRGB → linear** when setting Blender node colours (`s2l(c)= c/12.92 if c<=0.04045 else ((c+0.055)/1.055)**2.4`).
7. **Render isolation** = toggle `hide_render`, restore afterward. Keep `LED_Light` OFF except for the indicator render.
8. **Long renders go to the background** (`run_in_background`) — multi-glyph/all-asset renders are ~4 min.
9. **Flag design decisions** you made on the user's behalf (e.g. "moved the number off the knob", "made digits amber") and offer the alternative — the user iterates fast and reverts freely.
10. **Revert cleanly and leave the build consistent.** If you injected a temp value, always rebuild after removing it so the binary matches source.
11. **Update memory** (`gain76-gui-design.md` + this file) after meaningful changes.

---

## 7. GOTCHAS (bit us this session)
- **Washed ring ON the body cap (live only, not in Blender):** the cap = static base (`knob.png`) + grain overlay (`body_detailed`) at 0.88, feathered near the edge. If the static base body is a different (lighter) tone than the grain, the base shows through the feathered annulus = a washed ring absent from the uniform Blender `body_detailed`. **Fix: render `knob.png`'s body with the SAME full material as `body_detailed`** (done — removed the old `detach()` flatten for the `knob` render; base==grain ⇒ no ring). Separately, the washed ring OUTSIDE the knob is `Knob_Rim`'s bevel edge — handled by `feather_knob.py`.
- **⚠️ Knob material: iso renders mislead — judge against the LIVE composite.** A bare `Knob_Body` isolation render (esp. with `LED_Light` ON) looks very different from the plugin, because the plugin shows `knob_grain.png` (= `body_detailed`, rendered LED **off**) at 0.88 over `knob.png`'s **hardcoded flat base** (`detach('Base Color', …)` in `blender_render.py`). When tuning body colour/brightness, verify with `make_grain2.py`'s `_grain2_preview.png` (the actual composite) and a plugin screenshot — not the iso crop. To shift the live body you must change BOTH the material (`FLATTEN basecolor` Color2 → drives the grain) AND the flat base value in `blender_render.py` (drives the static base).
- **Non-uniform object scale**: `Botom_InfoLine_Housing.004` is scaled (1.377, 0.248, 0.47); a "spherical" gradient/inset reads oddly. For face-only material effects, **flatten Z** (Mapping Scale Z=0) so the gradient lives on the viewed face.
- **`.004` has leftover inset geometry** (12 verts / 10 faces vs a clean box 8/6) from a reverted dark-screen experiment — coplanar so invisible in render, but shows as stray edges in the viewport. `dissolve_limit` at 1° did NOT merge them (faces not perfectly coplanar). Not yet cleaned.
- **Blender scene compositor unavailable**: `scene.use_nodes=True` leaves `scene.node_tree = None`, so a **Glare/Bloom node can't be added** — do screen bloom in **JUCE** instead. Also: leaving `use_nodes=True` with a null tree risks black renders → keep it **False**.
- **Node `.name` ≠ visible label**: set `node.label` for names to show in the shader editor header.
- Pre-existing build warnings (Font ctor deprecation, GainProcessor C4305/C4996) are harmless — ignore.
- `get_viewport_screenshot` needs a `filepath` param and captures whatever the viewport currently frames (often the whole scene) — prefer a Cycles render crop for component close-ups.

---

## 8. Open threads / TODO
- **Info-line screens (5 cells):** far-left = **input peak (dB)**, 2nd = **STEREO/MONO** click-toggle (amber letter sprites, `drawAmberWord`, refWord "STEREO" ×0.65 so words match size), centre = **gain**, 4th = **momentary LUFS** (`analysis.lufs`; hover tooltip "LUFS" via `Gain76LookAndFeel::drawTooltip` = near-black bg + amber), far-right = **output peak (dB)**. In/out screens are peak-hold, reset on input silence. Remaining empty: none — next could be OVERSAMPLING.
- **Mode pop fixed:** Stereo/Mono/Side now crossfade via a smoothed channel matrix (`mtxA_..D_`, 20 ms) in `GainProcessor::processWet` instead of an instant swap.
- Amber **letter sprites** (`letters/A.png…Z.png`) + the "letters" Blender collection now exist — any label can be written with sprites.
- **Amber screen glow** on `Screen_plate` is essentially off (`SCR_Strength` was being tuned, last seen ≈ −0.168 = no glow). Decide final glow level; bloom to be added in JUCE.
- Clean `.004`'s leftover inset edges (or rebuild it as a clean box).
- Center readout digits are small in the 78×38 screen — may want to enlarge.
- Embed assets as **BinaryData** (remove absolute-path dependency) before release.
- Unused leftover materials in the scene: `Screen_Dark`, `LED_Amber` (digits use LED_Amber), `Dial_Ink` (used).
- Re-enable knob value/extra overlays only if desired; power button + combos still hidden.
