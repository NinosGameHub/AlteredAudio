# ALTERED AUDIO — MASTER JUCE IMPLEMENTATION SPEC
## Gain 76 · Complete Design System for Claude Code
### Compiled from 7 reverse-engineered design documents

---

# INSTRUCTIONS FOR CLAUDE CODE

This document contains the complete visual specification for the Altered Audio Gain 76 plugin UI.
Implement every section exactly as specified. Do not invent values — every colour, dimension, font,
and layer order is defined here. When in doubt, refer to this document.

The aesthetic goal: a precision scientific instrument from 1976, maintained carefully for decades.
Clean, functional, warm, engineered. NOT modern, NOT gaming, NOT neon, NOT chrome.

---

# 1. WINDOW & LAYOUT

```
Plugin Window:     1200 x 1050 px  (compact production version)
Main Panel:        1120 x 620 px
Corner Radius:     12px (outer window), 28px (main panel)
Outer Margin:      22px
Header Height:     95px
Inner Padding:     45px
Grid Unit:         10px
```

Component positions (compact version, scale from 1365px reference):
```
Main Panel:        X=40, Y=120
Input Meter:       X=68, Y=185    W=26, H=345
Output Meter:      X=1106, Y=185  W=26, H=345
Central Knob:      X=420, Y=160   W=280, H=280  (centred)
Stereo Bar:        X=40, Y=540    W=1120, H=53
Bottom Strip:      X=40, Y=605    W=1120, H=66
```

---

# 2. COLOUR PALETTE

```cpp
// Main Surfaces
static constexpr uint32_t COL_PANEL_BG        = 0xFFD5C5AC;
static constexpr uint32_t COL_PANEL_AGED      = 0xFFD3C4AA;
static constexpr uint32_t COL_PANEL_WARM      = 0xFFCFBDA5;
static constexpr uint32_t COL_KNOB_FACE       = 0xFFD4C5AF;
static constexpr uint32_t COL_KNOB_HIGHLIGHT  = 0xFFE0D5C1;
static constexpr uint32_t COL_WEATHERED       = 0xFFC9B9A0;

// Borders & Structure
static constexpr uint32_t COL_BORDER          = 0xFFA8977D;
static constexpr uint32_t COL_DIVIDER         = 0xFF9B8A72;
static constexpr uint32_t COL_KNOB_RIM        = 0xFF76674F;
static constexpr uint32_t COL_RIM_SHADOW      = 0xFF4F4537;
static constexpr uint32_t COL_HW_DARK         = 0xFF564B3C;

// Typography
static constexpr uint32_t COL_TEXT            = 0xFF282722;
static constexpr uint32_t COL_TEXT_MID        = 0xFF5D5649;
static constexpr uint32_t COL_TEXT_MUTED      = 0xFF7A7061;

// Displays
static constexpr uint32_t COL_DISPLAY         = 0xFF22211D;
static constexpr uint32_t COL_METER_HOUSING   = 0xFF2A2923;
static constexpr uint32_t COL_METER_OFF       = 0xFF3B352B;
static constexpr uint32_t COL_METER_SHADOW    = 0xFF171613;

// Amber Instrumentation
static constexpr uint32_t COL_AMBER_BRIGHT    = 0xFFF3B12A;
static constexpr uint32_t COL_AMBER_ACTIVE    = 0xFFE6A422;
static constexpr uint32_t COL_AMBER_DEEP      = 0xFFC9841E;
static constexpr uint32_t COL_AMBER_GLOW      = 0xFFF5C76D;
static constexpr uint32_t COL_AMBER_GOLD      = 0xFFDDA548;

// Wear & Tear
static constexpr uint32_t COL_DIRT            = 0xFFA28D70;
static constexpr uint32_t COL_SCRATCH         = 0xFF8F7C62;
static constexpr uint32_t COL_CORNER_WEAR     = 0xFF7C6A53;
static constexpr uint32_t COL_OXIDIZED        = 0xFF6D5B45;

// Controls
static constexpr uint32_t COL_DROPDOWN_BG     = 0xFFDCCFBC;
static constexpr uint32_t COL_DROPDOWN_BORDER = 0xFF9B8A72;
static constexpr uint32_t COL_TOGGLE_OFF      = 0xFFB8A78F;
static constexpr uint32_t COL_TOGGLE_ON       = 0xFFE6A422;
static constexpr uint32_t COL_STRIP_BG        = 0xFFCDB99D;
```

---

# 3. TYPOGRAPHY

Two fonts only. Download from Google Fonts and bundle as binary data.

```
Primary (headers, titles):   Space Mono Regular
                              Letter spacing: 0.15em
                              Usage: "ALTERED AUDIO", "GAIN 76", plugin names

Secondary (everything else): IBM Plex Mono Regular / Medium
                              Letter spacing: 0.05em
                              Usage: knob labels, values, meter labels, controls
```

Font sizes:
```
Header title:     48px  (Space Mono)
Header subtitle:  16px  (Space Mono)
Knob value:       52px  (IBM Plex Mono Medium)
Knob unit:        18px  (IBM Plex Mono Regular)
Strip labels:     11px  (IBM Plex Mono Regular)
Strip values:     14px  (IBM Plex Mono Regular)
Scale labels:     11px  (IBM Plex Mono Regular)
```

Load fonts in JUCE:
```cpp
// In your LookAndFeel constructor:
auto spaceMono = Typeface::createSystemTypefaceFor(
    BinaryData::SpaceMono_Regular_ttf,
    BinaryData::SpaceMono_Regular_ttfSize);
auto ibmPlexMono = Typeface::createSystemTypefaceFor(
    BinaryData::IBMPlexMono_Regular_ttf,
    BinaryData::IBMPlexMono_Regular_ttfSize);
```

---

# 4. BACKGROUND SURFACE — paintBackground()

Paint in this exact order:

```cpp
void paintBackground(Graphics& g, int w, int h)
{
    // LAYER 1 — Base surface
    g.fillAll(Colour(COL_PANEL_BG));

    // LAYER 2 — Large scale colour variation (200-400px blobs)
    // Use Perlin-style noise at 3-4% opacity, Overlay blend
    // Light areas: #DCCFBC, Dark areas: #CDB99D
    // Draw as large soft ellipses at random positions, opacity 3%
    {
        Random rng(12345);
        for (int i = 0; i < 8; ++i)
        {
            float bx = rng.nextFloat() * w;
            float by = rng.nextFloat() * h;
            float bw = 200.0f + rng.nextFloat() * 200.0f;
            bool bright = rng.nextBool();
            ColourGradient blob(
                Colour(bright ? 0xFFDCCFBC : 0xFFCDB99D).withAlpha(0.03f), bx, by,
                Colours::transparentBlack, bx + bw, by + bw, true);
            g.setGradientFill(blob);
            g.fillEllipse(bx - bw, by - bw, bw * 2.0f, bw * 2.0f);
        }
    }

    // LAYER 3 — Fine material grain (1-2px particles, 4-6% opacity)
    {
        Random rng(99999);
        for (int i = 0; i < (w * h) / 8; ++i)
        {
            float gx = rng.nextFloat() * w;
            float gy = rng.nextFloat() * h;
            bool bright = rng.nextBool();
            g.setColour(Colour(bright ? 0xFFE0D5C1 : 0xFFB8A78F).withAlpha(0.05f));
            g.fillRect(Rectangle<float>(gx, gy, 1.0f + (rng.nextBool() ? 1.0f : 0.0f), 1.0f));
        }
    }

    // LAYER 4 — Edge vignette (rectangular falloff, 60-90px spread)
    {
        float spread = 80.0f;
        float intensity = 0.08f;
        // Top edge
        ColourGradient topGrad(Colour(COL_BORDER).withAlpha(intensity), 0, 0,
                               Colours::transparentBlack, 0, spread, false);
        g.setGradientFill(topGrad);
        g.fillRect(0.0f, 0.0f, (float)w, spread);
        // Bottom edge
        ColourGradient botGrad(Colour(COL_BORDER).withAlpha(intensity), 0, (float)h,
                               Colours::transparentBlack, 0, (float)h - spread, false);
        g.setGradientFill(botGrad);
        g.fillRect(0.0f, (float)h - spread, (float)w, spread);
        // Left edge
        ColourGradient leftGrad(Colour(COL_BORDER).withAlpha(intensity), 0, 0,
                                Colours::transparentBlack, spread, 0, false);
        g.setGradientFill(leftGrad);
        g.fillRect(0.0f, 0.0f, spread, (float)h);
        // Right edge
        ColourGradient rightGrad(Colour(COL_BORDER).withAlpha(intensity), (float)w, 0,
                                 Colours::transparentBlack, (float)w - spread, 0, false);
        g.setGradientFill(rightGrad);
        g.fillRect((float)w - spread, 0.0f, spread, (float)h);
    }

    // LAYER 5 — Corner aging (soft radial at each corner, 3-5% opacity)
    {
        float r = 160.0f;
        float alpha = 0.04f;
        auto paintCorner = [&](float cx, float cy) {
            ColourGradient cg(Colour(COL_SCRATCH).withAlpha(alpha), cx, cy,
                              Colours::transparentBlack, cx + r, cy + r, true);
            g.setGradientFill(cg);
            g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
        };
        paintCorner(0, 0);
        paintCorner((float)w, 0);
        paintCorner(0, (float)h);
        paintCorner((float)w, (float)h);
    }

    // LAYER 6 — Edge wear (2-5px outer perimeter darkening)
    g.setColour(Colour(COL_CORNER_WEAR).withAlpha(0.06f));
    g.drawRect(Rectangle<float>(0, 0, (float)w, (float)h), 3.0f);

    // LAYER 7 — Micro stains (soft blotches, 2-3% opacity)
    {
        Random rng(77777);
        for (int i = 0; i < 6; ++i)
        {
            float sx = rng.nextFloat() * w;
            float sy = rng.nextFloat() * h;
            float sr = 20.0f + rng.nextFloat() * 40.0f;
            ColourGradient stain(Colour(COL_DIRT).withAlpha(0.025f), sx, sy,
                                 Colours::transparentBlack, sx + sr, sy, true);
            g.setGradientFill(stain);
            g.fillEllipse(sx - sr, sy - sr, sr * 2.0f, sr * 2.0f);
        }
    }

    // LAYER 8 — Scratches (40-60, horizontal bias, 6-12% opacity)
    {
        Random rng(55555);
        for (int i = 0; i < 50; ++i)
        {
            float sx = rng.nextFloat() * w;
            float sy = rng.nextFloat() * h;
            float len = 10.0f + rng.nextFloat() * 70.0f;
            float angle = (rng.nextFloat() - 0.5f) * 0.7f; // mostly horizontal
            if (rng.nextFloat() < 0.1f) angle = 0.785f;    // occasional 45deg
            float alpha = 0.06f + rng.nextFloat() * 0.06f;
            g.setColour(Colour(COL_SCRATCH).withAlpha(alpha));
            g.drawLine(sx, sy,
                       sx + std::cos(angle) * len,
                       sy + std::sin(angle) * len, 0.8f);
        }
    }

    // LAYER 9 — Satin reflection (top 35%, 2% white)
    {
        ColourGradient satin(Colour(0xFFFFFFFF).withAlpha(0.02f), 0, 0,
                             Colours::transparentBlack, 0, (float)h * 0.35f, false);
        g.setGradientFill(satin);
        g.fillRect(0.0f, 0.0f, (float)w, (float)h * 0.35f);
    }
}
```

---

# 5. KNOB — drawRotarySlider()

14-layer construction. Paint in this exact order:

```
Knob diameter (compact): 280px
Face diameter:           252px  (90%)
Bevel diameter:          263px  (94%)
Center plate:            126px  (45%)
Indicator dot:           17px   (6%)
Indicator distance:      112px  (40%) from centre
Scale ring diameter:     404px  (144%)
Major ticks:             8 count, 26px long, 2px wide
Minor ticks:             40 count, 14px long, 1px wide
```

```cpp
void drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                      float sliderPos, float startAngle, float endAngle,
                      Slider& slider) override
{
    auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float radius = jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float angle = startAngle + sliderPos * (endAngle - startAngle);

    // LAYER 1 — Scale ring ticks (drawn BEHIND knob)
    float scaleRadius = radius * 1.44f;
    for (int i = 0; i <= 48; ++i)
    {
        float tickAngle = startAngle + ((float)i / 48.0f) * (endAngle - startAngle);
        bool major = (i % 6 == 0);
        float innerR = scaleRadius + 4.0f;
        float outerR = scaleRadius + (major ? 26.0f : 14.0f);
        float ta = tickAngle - MathConstants<float>::halfPi;
        g.setColour(Colour(major ? COL_TEXT_MID : COL_TEXT_MUTED)
                    .withAlpha(major ? 0.8f : 0.7f));
        g.drawLine(cx + std::cos(ta) * innerR, cy + std::sin(ta) * innerR,
                   cx + std::cos(ta) * outerR, cy + std::sin(ta) * outerR,
                   major ? 2.0f : 1.0f);
    }

    // LAYER 2 — Active arc
    {
        float arcRadius = radius * 1.20f;
        float midAngle = startAngle + (endAngle - startAngle) * 0.5f;
        Path arc;
        bool positive = sliderPos > 0.5f;
        arc.addCentredArc(cx, cy, arcRadius, arcRadius, 0,
                          midAngle - MathConstants<float>::halfPi,
                          angle - MathConstants<float>::halfPi, true);
        PathStrokeType stroke(8.0f, PathStrokeType::curved, PathStrokeType::rounded);
        g.setColour(Colour(positive ? COL_AMBER_ACTIVE : COL_SCRATCH).withAlpha(0.85f));
        g.strokePath(arc, stroke);
    }

    // LAYER 3 — Outer shadow
    g.setColour(Colours::black.withAlpha(0.18f));
    g.fillEllipse(cx - radius * 1.01f, cy - radius * 1.01f + 5.0f,
                  radius * 2.02f, radius * 2.02f);

    // LAYER 4 — Outer rim
    g.setColour(Colour(COL_KNOB_RIM));
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    // LAYER 5 — Rim highlight (upper arc only)
    {
        Path rimArc;
        rimArc.addCentredArc(cx, cy, radius * 0.98f, radius * 0.98f,
                             0, MathConstants<float>::pi * 1.1f,
                             MathConstants<float>::twoPi * 1.9f, true);
        g.setColour(Colour(COL_BORDER).withAlpha(0.55f));
        g.strokePath(rimArc, PathStrokeType(2.0f));
    }

    // LAYER 6 — Inner bevel
    {
        float bevelR = radius * 0.94f;
        ColourGradient bevel(Colour(COL_KNOB_HIGHLIGHT), cx - bevelR * 0.3f, cy - bevelR * 0.3f,
                             Colour(0xFFC8B89E), cx + bevelR * 0.3f, cy + bevelR * 0.3f, false);
        g.setGradientFill(bevel);
        g.fillEllipse(cx - bevelR, cy - bevelR, bevelR * 2.0f, bevelR * 2.0f);
    }

    // LAYER 7 — Main knob face
    {
        float faceR = radius * 0.90f;
        ColourGradient face(Colour(0xFFDCCFBC), cx - faceR * 0.4f, cy - faceR * 0.4f,
                            Colour(0xFFC5B499), cx + faceR * 0.3f, cy + faceR * 0.3f, true);
        g.setGradientFill(face);
        g.fillEllipse(cx - faceR, cy - faceR, faceR * 2.0f, faceR * 2.0f);
    }

    // LAYER 8 — Surface grain
    {
        Random rng(42);
        float faceR = radius * 0.90f;
        for (int i = 0; i < 200; ++i)
        {
            float gx = cx + (rng.nextFloat() * 2.0f - 1.0f) * faceR;
            float gy = cy + (rng.nextFloat() * 2.0f - 1.0f) * faceR;
            if ((gx-cx)*(gx-cx) + (gy-cy)*(gy-cy) > faceR*faceR) continue;
            g.setColour(Colour(rng.nextBool() ? 0xFFE0D5C1 : 0xFFB8A78F).withAlpha(0.04f));
            g.fillRect(Rectangle<float>(gx, gy, 1.5f, 1.0f));
        }
    }

    // LAYER 9 — Inner shadow (dish effect)
    {
        float faceR = radius * 0.88f;
        ColourGradient dish(Colours::transparentBlack, cx, cy,
                            Colour(COL_KNOB_RIM).withAlpha(0.12f), cx + faceR, cy, true);
        g.setGradientFill(dish);
        g.fillEllipse(cx - faceR, cy - faceR, faceR * 2.0f, faceR * 2.0f);
    }

    // LAYER 10 — Centre plate shadow
    {
        float plateR = radius * 0.46f;
        g.setColour(Colours::black.withAlpha(0.08f));
        g.fillEllipse(cx - plateR, cy - plateR + 1.0f, plateR * 2.0f, plateR * 2.0f);
    }

    // LAYER 11 — Centre value plate
    {
        float plateR = radius * 0.45f;
        ColourGradient plate(Colour(COL_KNOB_HIGHLIGHT), cx, cy - plateR * 0.3f,
                             Colour(0xFFCDBEA7), cx, cy + plateR * 0.3f, false);
        g.setGradientFill(plate);
        g.fillEllipse(cx - plateR, cy - plateR, plateR * 2.0f, plateR * 2.0f);
    }

    // LAYER 12 — Value text
    {
        float plateR = radius * 0.45f;
        juce::String valText = juce::String(slider.getValue(), 1);
        g.setColour(Colour(COL_TEXT));
        g.setFont(Font(FontOptions().withHeight(plateR * 0.55f).withStyle("Medium")));
        g.drawFittedText(valText,
                         Rectangle<int>((int)(cx - plateR), (int)(cy - plateR * 0.45f),
                                        (int)(plateR * 2.0f), (int)(plateR * 0.7f)),
                         Justification::centred, 1);
        // Unit label
        g.setColour(Colour(COL_TEXT_MID));
        g.setFont(Font(FontOptions().withHeight(plateR * 0.22f)));
        g.drawFittedText("dB",
                         Rectangle<int>((int)(cx - plateR), (int)(cy + plateR * 0.1f),
                                        (int)(plateR * 2.0f), (int)(plateR * 0.3f)),
                         Justification::centred, 1);
    }

    // LAYER 13 — Indicator dot glow
    {
        float dotDist = radius * 0.40f;
        float dotR = radius * 0.06f;
        float a = angle - MathConstants<float>::halfPi;
        float dotX = cx + std::cos(a) * dotDist;
        float dotY = cy + std::sin(a) * dotDist;
        ColourGradient glow(Colour(COL_AMBER_GLOW).withAlpha(0.35f), dotX, dotY,
                            Colours::transparentBlack, dotX + dotR * 4.0f, dotY, true);
        g.setGradientFill(glow);
        g.fillEllipse(dotX - dotR * 4.0f, dotY - dotR * 4.0f,
                      dotR * 8.0f, dotR * 8.0f);

        // LAYER 14 — Indicator dot solid
        g.setColour(Colour(COL_AMBER_BRIGHT));
        g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
        g.setColour(Colour(COL_AMBER_DEEP).withAlpha(0.8f));
        g.drawEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f, 1.0f);
    }
}
```

---

# 6. VU METER — paintVUMeter()

```
Total segments:    48
Segment W:         20px
Segment H:         7px
Gap:               3px
Corner radius:     2px
Housing W:         36px
Housing H:         490px
Housing radius:    8px
```

```cpp
void paintVUMeter(Graphics& g, Rectangle<int> bounds, float level,
                  float peakLevel = 0.0f)
{
    // Housing
    g.setColour(Colour(COL_BORDER));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

    // Inner recess
    auto inner = bounds.reduced(3);
    ColourGradient recess(Colour(0xFF2E2D27), inner.getX(), inner.getY(),
                          Colour(0xFF1F1E1A), inner.getX(), inner.getBottom(), false);
    g.setGradientFill(recess);
    g.fillRoundedRectangle(inner.toFloat(), 6.0f);

    // Segments
    int numSeg = 48;
    int segW = 20;
    int segH = 7;
    int gap = 3;
    int startX = inner.getCentreX() - segW / 2;
    int startY = inner.getBottom() - 4;

    for (int i = 0; i < numSeg; ++i)
    {
        float segLevel = (float)i / (float)numSeg;
        int sy = startY - (i + 1) * (segH + gap);
        Rectangle<float> seg((float)startX, (float)sy, (float)segW, (float)segH);

        bool active = level > segLevel;
        bool peak   = peakLevel > segLevel && !active;
        bool clip   = segLevel > 0.96f;
        bool hi     = segLevel > 0.85f;

        Colour col;
        if (clip && active)
            col = Colour(0xFFC56A2B);
        else if (hi && active)
            col = Colour(COL_AMBER_BRIGHT);
        else if (active)
        {
            ColourGradient grad(Colour(COL_AMBER_BRIGHT), 0, (float)sy,
                                Colour(COL_AMBER_DEEP), 0, (float)(sy + segH), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(seg, 2.0f);
            continue;
        }
        else if (peak)
            col = Colour(COL_AMBER_GLOW).withAlpha(0.6f);
        else
            col = Colour(COL_METER_OFF);

        g.setColour(col);
        g.fillRoundedRectangle(seg, 2.0f);
    }

    // Level triangle marker
    float markerY = (float)startY - level * (float)(numSeg * (segH + gap));
    juce::Path triangle;
    float tx = (float)bounds.getRight() + 2.0f;
    triangle.addTriangle(tx, markerY - 5.0f,
                         tx + 10.0f, markerY,
                         tx, markerY + 5.0f);
    g.setColour(Colour(COL_TEXT));
    g.fillPath(triangle);
}
```

dB scale labels (paint to left of input meter, right of output meter):
```
Labels: +6, 0, -6, -12, -18, -24, -36, -48
Font:   IBM Plex Mono 11px
Color:  COL_TEXT_MID
```

---

# 7. STEREO BAR — paintStereoBar()

```
Two separate bars with gap between them
Container H:    53px
Segment W:      8px
Segment H:      36px
Gap:            2px
Corner radius:  3px (container)
```

```cpp
void paintStereoBar(Graphics& g, Rectangle<int> bounds,
                    float levelL, float levelR)
{
    // Container
    g.setColour(Colour(COL_METER_HOUSING));
    g.fillRoundedRectangle(bounds.toFloat(), 3.0f);
    g.setColour(Colour(COL_BORDER));
    g.drawRoundedRectangle(bounds.toFloat(), 3.0f, 1.0f);

    int halfW = bounds.getWidth() / 2 - 8;
    int segW = 8, segH = bounds.getHeight() - 10, gap = 2;
    int numSeg = halfW / (segW + gap);

    auto paintBar = [&](int startX, float level, bool reversed)
    {
        for (int i = 0; i < numSeg; ++i)
        {
            int idx = reversed ? (numSeg - 1 - i) : i;
            float segLevel = (float)idx / (float)numSeg;
            int sx = startX + i * (segW + gap);
            bool active = level > segLevel;

            Colour col = active
                ? (segLevel > 0.85f ? Colour(COL_AMBER_BRIGHT)
                                    : Colour(COL_AMBER_ACTIVE).brighter(segLevel * 0.3f))
                : Colour(COL_METER_OFF);

            g.setColour(col);
            g.fillRect(Rectangle<float>((float)sx,
                                        (float)bounds.getY() + 5.0f,
                                        (float)segW, (float)segH));
        }
    };

    // L bar (left half)
    paintBar(bounds.getX() + 4, levelL, false);
    // R bar (right half, mirrored)
    paintBar(bounds.getCentreX() + 4, levelR, false);

    // L / R labels
    g.setColour(Colour(COL_TEXT_MID));
    g.setFont(Font(FontOptions().withHeight(11.0f)));
    g.drawText("L", bounds.getX() - 14, bounds.getY(), 12, bounds.getHeight(),
               Justification::centredRight);
    g.drawText("R", bounds.getRight() + 2, bounds.getY(), 12, bounds.getHeight(),
               Justification::centredLeft);
}
```

---

# 8. BOTTOM STRIP — drawBottomStrip()

```
Height:        66px  (compact)
Corner radius: 12px
Border:        1px COL_BORDER
Top highlight: 1px #E0D5C1 at 35%
Bottom shadow: 1px COL_KNOB_RIM at 20%

5 sections: MODE | LINK | PEAK | LUFS | OVERSAMPLING
Dividers: 1px COL_BORDER at 60%, with +1px highlight at 15%

Dropdown:  W=150, H=34, radius=8, bg=COL_DROPDOWN_BG, border=COL_DROPDOWN_BORDER
Toggle:    W=70,  H=34, radius=17
  OFF: bg=COL_TOGGLE_OFF, text=#5D5649
  ON:  bg=COL_TOGGLE_ON,  text=#282722
```

---

# 9. HEADER

```cpp
// In paint():
// Brand name — Space Mono 48px
g.setColour(Colour(COL_TEXT));
g.setFont(spaceMono.withHeight(28.0f)); // scaled for compact
g.drawText("ALTERED AUDIO", 22, 14, 400, 32, Justification::centredLeft);

// Plugin name — Space Mono 16px, amber
g.setColour(Colour(COL_AMBER_ACTIVE));
g.setFont(spaceMono.withHeight(13.0f));
g.drawText("GAIN 76", 22, 46, 200, 16, Justification::centredLeft);

// Subtitle
g.setColour(Colour(COL_TEXT_MUTED));
g.setFont(ibmPlexMono.withHeight(10.0f));
g.drawText("CALIBRATION INSTRUMENT", 22, 62, 300, 12, Justification::centredLeft);

// Power indicator dot (top right)
g.setColour(Colour(COL_AMBER_BRIGHT));
g.fillEllipse((float)getWidth() - 18.0f, 18.0f, 8.0f, 8.0f);
```

---

# 10. CMAKEISTS — BINARY RESOURCES

```cmake
# Add to CMakeLists.txt after juce_add_plugin()
juce_add_binary_data(Gain76Assets SOURCES
    Resources/SpaceMono-Regular.ttf
    Resources/IBMPlexMono-Regular.ttf
    Resources/IBMPlexMono-Medium.ttf
)
target_link_libraries(Gain76 PRIVATE Gain76Assets)
```

Download fonts from Google Fonts:
- https://fonts.google.com/specimen/Space+Mono
- https://fonts.google.com/specimen/IBM+Plex+Mono

Place `.ttf` files in `Gain76/Resources/`

---

# 11. PROCESSOR — LEVEL METERING

Add to PluginProcessor.h (public):
```cpp
float getInputLevel()  { return inputLevel.load();  }
float getOutputLevel() { return outputLevel.load(); }
float getPeakLevel()   { return peakLevel.load();   }
```

Add to PluginProcessor.h (private):
```cpp
std::atomic<float> inputLevel  { 0.0f };
std::atomic<float> outputLevel { 0.0f };
std::atomic<float> peakLevel   { 0.0f };
float inputSmoothed  = 0.0f;
float outputSmoothed = 0.0f;
float peakHold       = 0.0f;
int   peakHoldTimer  = 0;
static constexpr float DECAY = 0.88f;
static constexpr int   PEAK_HOLD_SAMPLES = 44100; // ~1 second
```

Add to processBlock():
```cpp
// Input level
float rawIn = 0.0f;
for (int ch = 0; ch < getTotalNumInputChannels(); ++ch)
    rawIn = jmax(rawIn, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
inputSmoothed = rawIn > inputSmoothed ? rawIn : inputSmoothed * DECAY;
inputLevel.store(inputSmoothed);

// Apply gain
auto* gainParam = apvts.getRawParameterValue("GAIN");
float gainLin = Decibels::decibelsToGain(gainParam->load());
for (int ch = 0; ch < getTotalNumInputChannels(); ++ch)
    buffer.applyGain(ch, 0, buffer.getNumSamples(), gainLin);

// Output level
float rawOut = 0.0f;
for (int ch = 0; ch < getTotalNumInputChannels(); ++ch)
    rawOut = jmax(rawOut, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
outputSmoothed = rawOut > outputSmoothed ? rawOut : outputSmoothed * DECAY;
outputLevel.store(outputSmoothed);

// Peak hold
if (rawOut > peakHold) { peakHold = rawOut; peakHoldTimer = 0; }
else if (++peakHoldTimer > PEAK_HOLD_SAMPLES) peakHold *= DECAY;
peakLevel.store(peakHold);
```

---

# 12. PARAMETER LAYOUT

```cpp
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"GAIN", 1}, "Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    return {params.begin(), params.end()};
}
```

---

# END OF SPEC

This document fully defines the Altered Audio Gain 76 visual system.
Implement every section. Do not substitute colours, fonts, or dimensions.
The result should feel like a precision scientific instrument, not a plugin.
