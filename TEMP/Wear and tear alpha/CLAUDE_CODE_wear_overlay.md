# Claude Code Instructions — Integrate Wear & Tear Overlay

## Goal
Integrate a transparent PNG wear-and-tear overlay texture into the Gain76 plugin
background. The PNG provides organic scratches, dust smudges, and corner aging that
sit on top of the procedural panel surface.

File to add: `wear_overlay.png` → place in `Resources/`

---

## Step 1 — Add the PNG as binary data

In `CMakeLists.txt`:

```cmake
juce_add_binary_data(Gain76Assets SOURCES
    Resources/wear_overlay.png
)
target_link_libraries(Gain76 PRIVATE Gain76Assets)
```

If `Gain76Assets` already exists, add the PNG to the existing SOURCES list instead
of creating a duplicate target.

---

## Step 2 — Disable procedural scratches in PanelBackground.h

The PNG now handles scratches and stains, so turn off the code-drawn versions to
avoid doubling up. In the `paint()` method of `PanelBackground.h`, comment out or
remove these two calls:

```cpp
// paintScratches   (g, w, h);   // handled by wear_overlay.png now
// paintMicroStains (g, w, h);   // handled by wear_overlay.png now
```

Keep ALL of these:
- paintBase
- paintColourVariation
- paintGrain
- paintEdgeVignette
- paintCornerAging
- paintEdgeWear
- paintSatinSheen

---

## Step 3 — Draw the overlay on top of the background

In the `paint()` method of `PluginEditor.h` (or PluginEditor.cpp):

```cpp
void paint(juce::Graphics& g) override
{
    // 1. Procedural panel surface (base colour, grain, vignette)
    PanelBackground::paint(g, getWidth(), getHeight());

    // 2. Wear overlay PNG on top
    auto wear = juce::ImageCache::getFromMemory(
        BinaryData::wear_overlay_png,
        BinaryData::wear_overlay_pngSize);

    if (wear.isValid())
    {
        g.setOpacity(0.5f);   // tune 0.3 (subtle) to 0.7 (heavy)
        g.drawImage(wear, getLocalBounds().toFloat(),
                    juce::RectanglePlacement::stretchToFit);
        g.setOpacity(1.0f);
    }

    // 3. Everything else — header text, panel borders, version string, etc.
    //    (keep all existing paint code here, AFTER the overlay)
}
```

---

## Step 4 — Build and verify

JUCE generates the BinaryData symbol from the filename:
`wear_overlay.png` → `BinaryData::wear_overlay_png` + `BinaryData::wear_overlay_pngSize`

If the generated symbol name differs (check the generated `BinaryData.h`), fix the
reference in the code to match.

Build:
```bash
cmake --build build --config Release
```

Fix any compile errors.

---

## Critical ordering rule

Draw order in `paint()` must be:

```
1. Procedural background  (bottom)
2. Wear overlay PNG
3. Header text / borders / labels  (top)
```

And the KNOB and METERS (child Components) render above all of this automatically.

This is physically correct: the wear sits on the flat panel surface, but the raised
controls (knob, meters) sit above the wear — so scratches never appear on top of the
knob face. Do NOT draw the overlay after the child components or over the knob area.

---

## Tuning after build

The single control is the opacity value in Step 3:
- `0.3f` — premium, barely-there wear
- `0.5f` — moderate (recommended starting point)
- `0.7f` — heavily used vintage gear

Adjust to taste once you see it rendered in the DAW.

---

## Note on stretching

The overlay is stretched to fit the window with `stretchToFit`. Since the wear is
mostly random scratches and dust, mild stretching is not noticeable. If the window
is far from square and stretching becomes visible, regenerate the PNG at the window's
exact aspect ratio, or switch to `RectanglePlacement::fillDestination` to crop
instead of stretch.
