#include "GainEditor.h"

// ============================================================
//  Layout constants
// ============================================================
namespace
{
    const juce::Rectangle<int> kMainPanel { 16,  80, 788, 540 };
    const juce::Rectangle<int> kFooter   { 16, 636, 788,  52 };

    constexpr float kMeterLo = -24.0f, kMeterHi = 6.0f;
    constexpr float kGlitchDur = 0.30f;   // screen-glitch duration (seconds)
    inline float meterFrac(float lin)
    {
        const float db = juce::Decibels::gainToDecibels(lin, kMeterLo);
        return juce::jlimit(0.0f, 1.0f, (db - kMeterLo) / (kMeterHi - kMeterLo));
    }

    // Blender-rendered sprites live in Assets/Export (full-frame @2x = 1640x1440).
    // TODO: switch to embedded BinaryData before release so the plugin is portable.
    inline juce::Image loadAsset(const char* file)
    {
        return juce::ImageFileFormat::loadFrom(
            juce::File("C:/dev/gain76/Assets/Export").getChildFile(file));
    }
}

// ============================================================
//  LayoutStore
// ============================================================
juce::File LayoutStore::storageFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
           .getChildFile("AlteredAudio/Gain76/layout.json");
}

void LayoutStore::load()
{
    auto f = storageFile();
    if (!f.existsAsFile()) return;
    auto parsed = juce::JSON::parse(f.loadFileAsString());
    if (!parsed.isObject()) return;
    auto* obj = parsed.getDynamicObject();
    if (!obj) return;

    auto getRect = [&](const juce::String& key, juce::Rectangle<int> def) -> juce::Rectangle<int>
    {
        auto v = obj->getProperty(key);
        if (auto* o = v.getDynamicObject())
            return { (int)o->getProperty("x"), (int)o->getProperty("y"),
                     (int)o->getProperty("w"), (int)o->getProperty("h") };
        return def;
    };

    knob     = getRect("knob",     knob);
    inMeter  = getRect("inMeter",  inMeter);
    outMeter = getRect("outMeter", outMeter);
    power    = getRect("power",    power);
    footer   = getRect("footer",   footer);
    panel    = getRect("panel",    panel);
}

void LayoutStore::save() const
{
    auto toObj = [](juce::Rectangle<int> r) -> juce::var
    {
        auto* o = new juce::DynamicObject();
        o->setProperty("x", r.getX()); o->setProperty("y", r.getY());
        o->setProperty("w", r.getWidth()); o->setProperty("h", r.getHeight());
        return juce::var(o);
    };
    auto* root = new juce::DynamicObject();
    root->setProperty("knob",     toObj(knob));
    root->setProperty("inMeter",  toObj(inMeter));
    root->setProperty("outMeter", toObj(outMeter));
    root->setProperty("power",    toObj(power));
    root->setProperty("footer",   toObj(footer));
    root->setProperty("panel",    toObj(panel));
    auto f = storageFile();
    f.getParentDirectory().createDirectory();
    f.replaceWithText(juce::JSON::toString(juce::var(root), true));
}

// ============================================================
//  EditOverlay
// ============================================================
void GainEditor::EditOverlay::paint(juce::Graphics& g)
{
    // semi-transparent black veil so handles pop
    g.setColour(juce::Colour(0x55000000));
    g.fillAll();

    // Permanent centre crosshair — always visible in edit mode
    g.setColour(juce::Colour(0x44FFFFFF));
    g.drawHorizontalLine(kH / 2, 0.0f, (float)kW);
    g.drawVerticalLine(kW / 2, 0.0f, (float)kH);
    g.setColour(juce::Colour(0xCCFFFF00));
    g.fillEllipse((float)(kW / 2 - 3), (float)(kH / 2 - 3), 6.0f, 6.0f);

    for (int i = 0; i < entries.size(); ++i)
    {
        auto* c   = entries[i].comp;
        auto  r   = c->getBounds().toFloat();
        bool  act = (i == activeIdx);

        // border
        g.setColour(act ? juce::Colour(0xFFFF8C00) : juce::Colour(0xFF00CFFF));
        g.drawRect(r, act ? 2.5f : 1.5f);

        // resize handle — bottom-right corner square
        constexpr float hs = 14.0f;
        g.fillRect(r.getRight() - hs, r.getBottom() - hs, hs, hs);

        // info label above the component
        const juce::String info = entries[i].name
            + "   x:" + juce::String(c->getX())
            + " y:"   + juce::String(c->getY())
            + "  "    + juce::String(c->getWidth())
            + "x"     + juce::String(c->getHeight());

        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRect((int)r.getX(), (int)r.getY() - 16, 260, 14);
        g.setColour(act ? juce::Colour(0xFFFF8C00) : juce::Colours::white);
        g.setFont(juce::Font(10.5f));
        g.drawText(info, (int)r.getX() + 3, (int)r.getY() - 15, 256, 13,
                   juce::Justification::centredLeft, false);
    }

    // bottom hint bar
    g.setColour(juce::Colour(0xCC000000));
    g.fillRect(0, kH - 22, kW, 22);
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(juce::Font(10.0f));
    g.drawText("DRAG to move   CORNER to resize   Right-click > Save Layout / Exit",
               8, kH - 20, kW - 16, 18, juce::Justification::centredLeft, false);
}

bool GainEditor::EditOverlay::hitTest(int x, int y)
{
    for (auto& e : entries)
        if (e.comp->getBounds().contains(x, y))
            return true;
    return false;
}

void GainEditor::EditOverlay::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (onRightClick) onRightClick(e);
        return;
    }
    activeIdx = -1;
    resizing  = false;
    for (int i = 0; i < entries.size(); ++i)
    {
        auto r = entries[i].comp->getBounds();
        if (!r.contains(e.getPosition())) continue;
        activeIdx   = i;
        startBounds = r;
        dragStart   = e.getPosition();
        constexpr int hs = 14;
        resizing = juce::Rectangle<int>(r.getRight() - hs, r.getBottom() - hs, hs, hs)
                       .contains(e.getPosition());
        break;
    }
    repaint();
}

void GainEditor::EditOverlay::mouseDrag(const juce::MouseEvent& e)
{
    if (activeIdx < 0) return;
    auto* c     = entries[activeIdx].comp;
    auto  delta = e.getPosition() - dragStart;
    constexpr int kSnap = 8;

    if (resizing)
    {
        int newW = juce::jmax(20, startBounds.getWidth()  + delta.x);
        int newH = juce::jmax(20, startBounds.getHeight() + delta.y);
        for (int i = 0; i < entries.size(); ++i)
        {
            if (i == activeIdx) continue;
            auto ob = entries[i].comp->getBounds();
            for (int sx : { ob.getX(), ob.getRight() })
                if (std::abs(startBounds.getX() + newW - sx) <= kSnap)
                    newW = sx - startBounds.getX();
            for (int sy : { ob.getY(), ob.getBottom() })
                if (std::abs(startBounds.getY() + newH - sy) <= kSnap)
                    newH = sy - startBounds.getY();
        }
        c->setBounds(startBounds.withSize(juce::jmax(20, newW), juce::jmax(20, newH)));
    }
    else
    {
        int x = startBounds.getX() + delta.x;
        int y = startBounds.getY() + delta.y;
        const int w = startBounds.getWidth(), h = startBounds.getHeight();
        for (int i = 0; i < entries.size(); ++i)
        {
            if (i == activeIdx) continue;
            auto ob = entries[i].comp->getBounds();
            for (int sx : { ob.getX(), ob.getRight(), ob.getCentreX() })
            {
                if (std::abs(x         - sx) <= kSnap) x = sx;
                if (std::abs(x + w     - sx) <= kSnap) x = sx - w;
                if (std::abs(x + w / 2 - sx) <= kSnap) x = sx - w / 2;
            }
            for (int sy : { ob.getY(), ob.getBottom(), ob.getCentreY() })
            {
                if (std::abs(y         - sy) <= kSnap) y = sy;
                if (std::abs(y + h     - sy) <= kSnap) y = sy - h;
                if (std::abs(y + h / 2 - sy) <= kSnap) y = sy - h / 2;
            }
        }
        // Snap to canvas centre axes
        auto snapToAxis = [kSnap](int v, int size, int axis) -> int {
            if (std::abs(v           - axis) <= kSnap) return axis;
            if (std::abs(v + size    - axis) <= kSnap) return axis - size;
            if (std::abs(v + size/2  - axis) <= kSnap) return axis - size / 2;
            return v;
        };
        x = snapToAxis(x, w, kW / 2);
        y = snapToAxis(y, h, kH / 2);

        c->setTopLeftPosition(x, y);
    }
    if (onChange) onChange();
    repaint();
}

void GainEditor::EditOverlay::mouseUp(const juce::MouseEvent&) { repaint(); }

// ============================================================
//  Gain76LookAndFeel
// ============================================================

Gain76LookAndFeel::Gain76LookAndFeel()
{
    setColour(juce::Label::textColourId, gain76::textPrimary);
    setColour(juce::PopupMenu::backgroundColourId,            gain76::cream);
    setColour(juce::PopupMenu::textColourId,                  gain76::textPrimary);
    setColour(juce::PopupMenu::headerTextColourId,            gain76::textSecond);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, gain76::amber);
    setColour(juce::PopupMenu::highlightedTextColourId,       gain76::textOnAccent);
}

// Knob: static Blender base disc + a spinning body-grain overlay + a rotating
// LED position dot. The base carries the fixed form/highlights (lamp stays put);
// only the grain texture and the LED dot rotate with the value, so the knob
// reads as turning without the baked highlights sliding around.
void Gain76LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float sliderPos, float startAngle, float endAngle,
                                         juce::Slider& s)
{
    const auto wb = s.getBounds();   // window coords within the kW x kH canvas

    // Helper: blit a full-frame @2x sprite's knob region into (x,y,w,h).
    auto blitKnob = [&](const juce::Image& img)
    {
        if (img.isValid())
            g.drawImage(img, x, y, w, h,
                        wb.getX() * 2, wb.getY() * 2, wb.getWidth() * 2, wb.getHeight() * 2);
    };

    // 1) Static base disc (flat body + rim + groove + tick scale).
    blitKnob(knobImage);

    // Rotation shared by grain + indicator. Both were rendered at the 3-o'clock
    // baseline (pi/2 clockwise from top), so rotate by current angle - pi/2.
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const float rot   = angle - juce::MathConstants<float>::halfPi;
    const float cx    = (float) x + (float) w * 0.5f;   // knob centre (local)
    const float cy    = (float) y + (float) h * 0.5f;

    // 2) Spinning body-grain overlay, clipped to the body cap so it never spills
    //    onto the groove/rim. Body cap radius ~= 0.245 * w in local coords.
    if (grainImage.isValid())
    {
        g.saveState();
        juce::Path cap;
        const float cr = 0.245f * (float) w;
        cap.addEllipse(cx - cr, cy - cr, cr * 2.0f, cr * 2.0f);
        g.reduceClipRegion(cap);
        g.addTransform(juce::AffineTransform::rotation(rot, cx, cy));
        g.setOpacity(0.88f);
        blitKnob(grainImage);
        g.restoreState();
    }

    // 3) LED position dot — tracks the value (orbits the knob). Rendered at the
    //    3-o'clock baseline, rotated by `rot` so 0 dB points up, -24 lower-left,
    //    +24 lower-right (matches the tick dial). A warm radial glow is drawn
    //    under the dot and rides the same rotation.
    if (indicatorImage.isValid())
    {
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(rot, cx, cy));

        // Glow halo at the dot's baseline position (cx + dotR, cy); the same
        // transform carries it to the live angle. Soft outer wash + hot core.
        const float gx   = cx + 0.203f * (float) w;   // dot orbit radius
        const float gy   = cy;
        auto blob = [&](float radius, float alpha)
        {
            juce::ColourGradient gr(gain76::led.withAlpha(alpha), gx, gy,
                                    gain76::led.withAlpha(0.0f), gx + radius, gy, true);
            g.setGradientFill(gr);
            g.fillEllipse(gx - radius, gy - radius, radius * 2.0f, radius * 2.0f);
        };
        blob(0.105f * (float) w, 0.34f);   // soft halo (tight, hugs the dot)
        blob(0.052f * (float) w, 0.60f);   // hot core

        blitKnob(indicatorImage);
        g.restoreState();
    }
    // (Value readout moved to the info-line centre cube — see valueDisplay_.)
}

// Flat combo box (Select.jsx): cream fill, hairline border, thin chevron
void Gain76LookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height).reduced(0.5f);
    g.setColour(isButtonDown ? gain76::sand
                             : box.isMouseOver() ? gain76::cream.brighter(0.04f) : gain76::cream);
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(gain76::border);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);

    const float chx = (float)width - 15.0f, chy = (float)height * 0.5f;
    juce::Path ch;
    ch.startNewSubPath(chx - 4.0f, chy - 2.0f);
    ch.lineTo(chx, chy + 2.5f);
    ch.lineTo(chx + 4.0f, chy - 2.0f);
    g.setColour(gain76::textSecond);
    g.strokePath(ch, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
}

juce::Font Gain76LookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return gain76::mono(11.0f).withExtraKerningFactor(0.06f);
}

void Gain76LookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(8, 1, box.getWidth() - 30, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
    label.setColour(juce::Label::textColourId, gain76::textPrimary);
}

// ============================================================
//  Gain76PowerKey
// ============================================================

void Gain76PowerKey::paintButton(juce::Graphics& g, bool over, bool down)
{
    const bool on = getToggleState();
    const auto r  = getLocalBounds().toFloat();
    const float d = juce::jmin(r.getWidth(), r.getHeight() - 14.0f);
    juce::Rectangle<float> key(r.getCentreX() - d * 0.5f, r.getY(), d, d);

    g.setColour(juce::Colour(0x402A251C));
    g.fillEllipse(key.translated(0.0f, 1.8f));
    juce::ColourGradient grad(juce::Colour(0xFFF3EEE0),
                              key.getX() + d * 0.38f, key.getY() + d * 0.30f,
                              juce::Colour(0xFFDCD4C2),
                              key.getX() + d * 0.5f,  key.getBottom(), true);
    grad.addColour(0.42, gain76::cream);
    g.setGradientFill(grad);
    g.fillEllipse(key.reduced(down ? 1.2f : 0.0f));
    g.setColour(gain76::border.withAlpha(over ? 0.95f : 0.65f));
    g.drawEllipse(key.reduced(0.5f), 1.0f);

    const float cx = key.getCentreX(), cy = key.getCentreY(), gr = d * 0.22f;
    g.setColour(on ? gain76::textPrimary : gain76::inkFaint);
    g.drawLine(cx, cy - gr * 1.25f, cx, cy - gr * 0.15f, 2.2f);
    juce::Path arc;
    arc.addCentredArc(cx, cy, gr, gr, 0.0f,
                      juce::MathConstants<float>::pi * 0.30f,
                      juce::MathConstants<float>::pi * 1.70f, true);
    g.strokePath(arc, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));

    const float lr = 3.0f, ly = r.getBottom() - lr * 2.2f;
    if (on)
    {
        g.setColour(gain76::led.withAlpha(0.35f));
        g.fillEllipse(cx - lr * 2.0f, ly - lr * 2.0f + lr, lr * 4.0f, lr * 4.0f);
        g.setColour(gain76::led);
    }
    else
        g.setColour(gain76::border.withAlpha(0.5f));
    g.fillEllipse(cx - lr, ly, lr * 2.0f, lr * 2.0f);
}

// ============================================================
//  VerticalMeter
// ============================================================

void VerticalMeter::paint(juce::Graphics& g)
{
    const auto wb = getBounds();          // window coords within the kW x kH canvas
    const int  W  = getWidth(), H = getHeight();

    auto cropDraw = [&](const juce::Image& img) {
        if (img.isValid())
            g.drawImage(img, 0, 0, W, H,
                        wb.getX() * 2, wb.getY() * 2, wb.getWidth() * 2, wb.getHeight() * 2);
    };

    // 1) Brass housing (background)
    cropDraw(housingImage);
    if (!stripImage.isValid())
        return;

    // Strip art vertical extent in local coords (measured from the export):
    // top = +6 dB end, bottom = -24 dB end.
    const float topY = 10.0f;
    const float botY = 340.0f;
    const float span = botY - topY;

    auto dbToY = [&](float db) {
        const float f = juce::jlimit(0.0f, 1.0f, (db - kMeterLo) / (kMeterHi - kMeterLo));
        return botY - f * span;          // higher dB -> smaller y (toward top)
    };

    const float lvlDb   = juce::Decibels::gainToDecibels(juce::jmax(disp, 1.0e-6f), kMeterLo);
    const float yLevel  = dbToY(lvlDb);  // everything below this y (louder) is lit
    const float yZero   = dbToY(0.0f);
    const float yMinus9 = dbToY(-9.0f);

    // Transform mapping the full strip image into this component's locked region,
    // used as an exact alpha mask so recolouring lines up with the segments.
    const auto mask = juce::AffineTransform::scale(0.5f)
                          .translated(-(float)wb.getX(), -(float)wb.getY());

    // Inactive base: every segment is permanently visible in a warm dark brass
    // (was a cold grey #7A7468 that read blue against the faceplate).
    {
        g.saveState();
        g.reduceClipRegion(stripImage, mask);
        g.setColour(juce::Colour(0xFF6E6048));
        g.fillRect(0, 0, W, H);
        g.restoreState();
    }

    // Recolour a zone's lit portion using the strip's own alpha (exact overlay).
    auto fillZone = [&](float yTop, float yBot, juce::Colour col) {
        const float y0 = juce::jmax(yLevel, yTop);
        if (y0 >= yBot) return;
        const juce::Rectangle<int> r(0, juce::roundToInt(y0), W, juce::roundToInt(yBot - y0));
        g.saveState();
        g.reduceClipRegion(r);
        g.reduceClipRegion(stripImage, mask);
        g.setColour(col);
        g.fillRect(r);
        g.restoreState();
    };

    fillZone(yMinus9, botY,   juce::Colour(0xFFE8A23A));   // < -9 dB     amber (rich)
    fillZone(yZero,   yMinus9, juce::Colour(0xFFC56A2B));  // -9 .. 0 dB  orange
    fillZone(topY,    yZero,   juce::Colour(0xFFD23B1A));  //  0 .. +6 dB red

    // Emissive hot core — brighten the lit segments so they read as glowing.
    if (yLevel < botY)
    {
        g.saveState();
        g.reduceClipRegion(juce::Rectangle<int>(0, juce::roundToInt(yLevel),
                                                W, juce::roundToInt(botY - yLevel)));
        g.reduceClipRegion(stripImage, mask);
        g.setColour(juce::Colours::white.withAlpha(0.16f));
        g.fillRect(0, 0, W, H);
        g.restoreState();
    }
}

// ============================================================
//  HoldMeter
// ============================================================

void HoldMeter::paint(juce::Graphics& g)
{
    // Horizontal PeakMeter: same molded-segment recipe as VerticalMeter.
    // Bars grow from the outer edge inward; R channel is mirrored.
    const float w = (float)getWidth(), h = (float)getHeight();
    constexpr int   nSeg   = 46;
    constexpr float segGap = 2.0f;

    const float frac     = meterFrac(disp);
    const float holdFrac = meterFrac(holdLin);
    const int   holdIdx  = (holdLin > 0.000001f)
        ? juce::jlimit(0, nSeg - 1,
              (int)std::round(holdFrac * (float)nSeg - 0.5f))
        : -1;

    const float segW = (w - (float)(nSeg - 1) * segGap) / (float)nSeg;

    for (int i = 0; i < nSeg; ++i)
    {
        const float segFrac_i = ((float)i + 0.5f) / (float)nSeg;
        const float db_i = kMeterLo + segFrac_i * (kMeterHi - kMeterLo);
        const bool  lit  = (segFrac_i <= frac) || (i == holdIdx);
        const bool  warn = lit && (db_i >= 0.0f);

        const float sx = mirrored
            ? w - (float)(i + 1) * segW - (float)i * segGap
            : (float)i * (segW + segGap);

        const juce::Rectangle<float> seg(sx, 0.0f, segW, h);

        // 3-stop horizontal gradient (left → right)
        juce::ColourGradient grad;
        if (lit && warn)
        {
            grad = juce::ColourGradient(juce::Colour(0xFFE08A3C), sx, 0.0f,
                                        juce::Colour(0xFFA5541D), sx + segW, 0.0f, false);
            grad.addColour(0.55, juce::Colour(0xFFC56A2B));
        }
        else if (lit)
        {
            grad = juce::ColourGradient(juce::Colour(0xFFF4BE55), sx, 0.0f,
                                        juce::Colour(0xFFBC7E20), sx + segW, 0.0f, false);
            grad.addColour(0.48, juce::Colour(0xFFDE9F35));
        }
        else
        {
            grad = juce::ColourGradient(juce::Colour(0xFF7B7365), sx, 0.0f,
                                        juce::Colour(0xFF575145), sx + segW, 0.0f, false);
            grad.addColour(0.50, juce::Colour(0xFF6A6356));
        }
        g.setGradientFill(grad);
        g.fillRoundedRectangle(seg, 1.5f);

        // Emboss: inset top highlight
        g.setColour(juce::Colours::white.withAlpha(lit ? (warn ? 0.40f : 0.50f) : 0.18f));
        g.drawLine(sx + 1.5f, 1.0f, sx + segW - 1.5f, 1.0f, 1.0f);

        // Emboss: inset bottom shadow
        g.setColour(juce::Colours::black.withAlpha(lit ? (warn ? 0.30f : 0.28f) : 0.22f));
        g.drawLine(sx + 1.5f, h - 1.0f, sx + segW - 1.5f, h - 1.0f, 1.0f);

        // Emboss: 1px outer ring
        g.setColour(lit ? (warn ? juce::Colour(0x665E391C) : juce::Colour(0x595E4D2A))
                        : juce::Colour(0x4D3C362C));
        g.drawRoundedRectangle(seg, 1.5f, 1.0f);
    }
}

// ============================================================
//  GainEditor
// ============================================================

GainEditor::GainEditor(juce::AudioProcessor& proc,
                       juce::AudioProcessorValueTreeState& vts,
                       GainAnalysisSource& src)
    : AudioProcessorEditor(&proc), apvts(vts), analysis(src)
{
    // ---- Load Blender-rendered design sprites ----
    brassTexture_ = loadAsset("faceplate.png");
    meterLImg_    = loadAsset("meter_housing_left.png");
    meterRImg_    = loadAsset("meter_housing_right.png");
    infolineImg_  = loadAsset("infoline_housing.png");
    logoImg_      = loadAsset("text_altered_audio.png");
    gainTextImg_  = loadAsset("text_gain76.png");
    lnf.knobImage      = loadAsset("knob.png");
    lnf.grainImage     = loadAsset("knob_grain.png");
    lnf.indicatorImage = loadAsset("indicator.png");

    // Embossed brass glyphs (0-9, '.', '+', '-') for the dB readout.
    {
        const char* gn[13] = { "d0","d1","d2","d3","d4","d5","d6","d7","d8","d9",
                               "dot","plus","minus" };
        for (int i = 0; i < 13; ++i)
            glyphImg_[i] = loadAsset((juce::String("digits/") + gn[i] + ".png").toRawUTF8());
    }

    // Attach the meter housings + amber strips to their components (locked)
    inMeter.housingImage  = meterLImg_;
    outMeter.housingImage = meterRImg_;
    inMeter.stripImage    = loadAsset("meter_strip_left.png");
    outMeter.stripImage   = loadAsset("meter_strip_right.png");

    layout_.load();

    setLookAndFeel(&lnf);
    addAndMakeVisible(content);
    content.setBounds(0, 0, kW, kH);

    // ---- Main panel — kept as an edit-mode/right-click target only.
    //      The recessed panel is now baked into faceplate.png, so no painting. ----
    content.addAndMakeVisible(mainPanel_);

    gainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainKnob.setDoubleClickReturnValue(true, 0.0);
    content.addAndMakeVisible(gainKnob);
    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::gainDb, gainKnob);
    // AFTER the attachment — SliderAttachment installs the parameter's own
    // 2-decimal formatter and would clobber this if set earlier
    gainKnob.textFromValueFunction = [](double v) {
        return (v >= 0.05 ? "+" : "") + juce::String(v, 1); };
    gainKnob.updateText();

    // Combo boxes + power button: attachments stay wired, but added hidden
    // (addChildComponent) so no procedural chrome shows. Re-enable one by one.
    modeBox.addItemList({ "STEREO", "MONO", "SIDE" }, 1);
    footerComp_.addChildComponent(modeBox);
    modeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, ParamID::gainMode, modeBox);

    osBox.addItemList({ "1x", "4x", "8x" }, 1);
    footerComp_.addChildComponent(osBox);
    osAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, ParamID::gainOversamp, osBox);

    powerBtn.setClickingTogglesState(true);
    powerBtn.onClick = [this]() {
        if (auto* p = apvts.getParameter(ParamID::gainBypass))
            p->setValueNotifyingHost(powerBtn.getToggleState() ? 0.0f : 1.0f);
    };
    content.addChildComponent(powerBtn);

    // ---- Glow layer: casts the lit meter colours onto the surrounding brass.
    //      Added BEFORE the meters so it sits behind them (on top of faceplate);
    //      the spill beyond each housing lands on the open brass. ----
    glowLayer_.setInterceptsMouseClicks(false, false);
    glowLayer_.setBounds(0, 0, kW, kH);
    glowLayer_.onPaint = [this](juce::Graphics& g) {
        // Build the glow in a downscaled offscreen buffer, Gaussian-blur it, then
        // upscale. Stacked low-alpha *radial* gradients used to leave vertical
        // banding ("reflecting lines"); a blurred low-res buffer is perfectly
        // smooth and gives a big, soft feather.
        const int S  = 4;                                   // downscale factor
        const int gw = (kW + S - 1) / S, gh = (kH + S - 1) / S;
        juce::Image glow(juce::Image::ARGB, gw, gh, true);
        {
            juce::Graphics gg(glow);
            gg.addTransform(juce::AffineTransform::scale(1.0f / (float)S));

            auto drawGlow = [&gg](VerticalMeter& m) {
                const auto  b   = m.getBounds();
                const float lvl = m.level();
                if (lvl < 1.0e-5f) return;

                const float lvlDb = juce::Decibels::gainToDecibels(lvl, -24.0f);
                const float topY = 10.0f, botY = 340.0f, span = botY - topY;
                auto dbToY = [&](float db) {
                    return botY - juce::jlimit(0.0f, 1.0f, (db + 24.0f) / 30.0f) * span;
                };
                const float yLevel = (float)b.getY() + dbToY(lvlDb);
                const float yZero  = (float)b.getY() + dbToY(0.0f);
                const float yM9    = (float)b.getY() + dbToY(-9.0f);
                const float yTop   = (float)b.getY() + topY;
                const float yBot   = (float)b.getY() + botY;
                const float cx     = (float)b.getX() + 31.5f;   // strip centre
                const float halfW  = 22.0f;                     // bar half-width (blur spreads it)

                auto zone = [&](float yA, float yB, juce::Colour col) {
                    const float y0 = juce::jmax(yLevel, yA);
                    if (y0 >= yB) return;
                    gg.setColour(col.withAlpha(0.6f));
                    gg.fillRect(juce::Rectangle<float>(cx - halfW, y0, halfW * 2.0f, yB - y0));
                };
                zone(yM9,   yBot,  juce::Colour(0xFFE8A23A));   // < -9   amber
                zone(yZero, yM9,   juce::Colour(0xFFC56A2B));   // -9..0  orange
                zone(yTop,  yZero, juce::Colour(0xFFD23B1A));   // >= 0   red
            };
            drawGlow(inMeter);
            drawGlow(outMeter);
        }

        juce::ImageConvolutionKernel k(11);
        k.createGaussianBlur(4.5f);                         // big, soft feather
        k.applyToImage(glow, glow, glow.getBounds());

        g.setOpacity(0.38f);                                // overall glow intensity (subtle)
        g.drawImage(glow, 0, 0, kW, kH, 0, 0, gw, gh);
    };
    content.addAndMakeVisible(glowLayer_);

    content.addAndMakeVisible(inMeter);
    content.addAndMakeVisible(outMeter);

    peakReset.fn = [this]() { heldPeakDb = -120.0f; };
    footerComp_.addAndMakeVisible(peakReset);

    // ---- Footer panel component ----
    // Strip is the infoline_housing.png sprite. All procedural labels/readouts
    // (MODE / PEAK / LUFS / OVERSAMPLING) removed — to be re-added one by one.
    content.addAndMakeVisible(footerComp_);

    // ---- dB readout in the info-line centre cube (canvas-fixed position) ----
    valueDisplay_.setInterceptsMouseClicks(false, false);
    valueDisplay_.setBounds(321, 594, 78, 38);   // centre screen of the new info line
    valueDisplay_.onPaint = [this](juce::Graphics& g)
    {
        const int   W   = valueDisplay_.getWidth();
        const int   H   = valueDisplay_.getHeight();
        const juce::String num = gainKnob.getTextFromValue(gainKnob.getValue()); // "+15.0"

        // Normal readout
        if (glitchT_ <= 0.0f) { drawAmberReadout(g, W, H, num); return; }

        // ---- Clip-triggered glitch (medium): tear + RGB split + flicker ----
        const float t  = juce::jlimit(0.0f, 1.0f, glitchT_ / kGlitchDur);  // 1 -> 0
        const int   Wi = W, Hi = H;

        juce::Image layer(juce::Image::ARGB, Wi, Hi, true);
        { juce::Graphics lg(layer); drawAmberReadout(lg, Wi, Hi, num); }

        const float flick = 0.55f + 0.45f * glitchRng_.nextFloat();
        const int   dx    = juce::roundToInt((2.0f + 5.0f * t) * (glitchRng_.nextFloat() * 2.0f - 1.0f));

        // RGB split — red ghost at +dx, cyan ghost at -dx (tint via alpha mask)
        auto ghost = [&](int ox, juce::Colour col)
        {
            g.saveState();
            g.reduceClipRegion(layer, juce::AffineTransform::translation((float) ox, 0.0f));
            g.setColour(col.withAlpha(0.45f * flick));
            g.fillRect(0, 0, Wi, Hi);
            g.restoreState();
        };
        ghost(dx,  juce::Colour(0xFFFF3030));
        ghost(-dx, juce::Colour(0xFF30FFFF));

        // Torn horizontal bands, each offset randomly
        const int bands = 6;
        for (int i = 0; i < bands; ++i)
        {
            const int by = i * Hi / bands;
            const int bh = (i + 1) * Hi / bands - by;
            const int ox = (glitchRng_.nextFloat() < 0.55f)
                ? juce::roundToInt((glitchRng_.nextFloat() * 2.0f - 1.0f) * 7.0f * t) : 0;
            g.setOpacity(flick);
            g.drawImage(layer, ox, by, Wi, bh, 0, by, Wi, bh);
        }
        g.setOpacity(1.0f);

        // occasional bright scanline
        if (glitchRng_.nextFloat() < 0.6f)
        {
            const int sy = glitchRng_.nextInt(juce::jmax(1, Hi));
            g.setColour(juce::Colours::white.withAlpha(0.22f * t));
            g.fillRect(0, sy, Wi, 2);
        }
    };
    content.addAndMakeVisible(valueDisplay_);

    // ---- Far-left / far-right info-line screens: input / output level (dB) ----
    //      Driven by the same smoothed level the meters read, floored at -60 dB.
    inDisplay_.setInterceptsMouseClicks(false, false);
    inDisplay_.setBounds(70, 593, 83, 38);   // matches the far-left screen plate recess
    inDisplay_.onPaint = [this](juce::Graphics& g) {
        // Held peak input level (dB) with +/- sign; reset on input silence.
        const float db = inPeakDb_;
        const juce::String s = (db >= 0.05f ? "+" : "") + juce::String(db, 1);
        drawAmberReadout(g, inDisplay_.getWidth(), inDisplay_.getHeight(), s, 0.75f);
    };
    content.addAndMakeVisible(inDisplay_);

    outDisplay_.setInterceptsMouseClicks(false, false);
    outDisplay_.setBounds(566, 593, 83, 38);   // matches the far-right screen plate recess
    outDisplay_.onPaint = [this](juce::Graphics& g) {
        // Held peak output level (dB) with +/- sign; reset on input silence.
        const float db = outPeakDb_;
        const juce::String s = (db >= 0.05f ? "+" : "") + juce::String(db, 1);
        drawAmberReadout(g, outDisplay_.getWidth(), outDisplay_.getHeight(), s, 0.75f);
    };
    content.addAndMakeVisible(outDisplay_);

    // ---- 2nd screen plate (between IN and centre GAIN): STEREO/MONO toggle ----
    //      Click to switch the channel mode (Stereo <-> Mono). Drives the same
    //      gainMode parameter as the hidden modeBox. Amber mono font (the glyph
    //      sprites are digits-only, so letters use the font).
    modeScreen_.setBounds(188, 593, 82, 38);
    modeScreen_.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    modeScreen_.onPaint = [this](juce::Graphics& g) {
        auto* p = apvts.getParameter(ParamID::gainMode);
        const juce::String txt = p ? p->getCurrentValueAsText().toUpperCase()
                                   : juce::String("STEREO");
        g.setColour(juce::Colour(0xFFE8A23A));
        g.setFont(gain76::mono(13.0f).withExtraKerningFactor(0.05f));
        g.drawText(txt, modeScreen_.getLocalBounds(), juce::Justification::centred, false);
    };
    modeScreen_.onLeftClick = [this]() {
        if (auto* p = apvts.getParameter(ParamID::gainMode)) {
            const int idx = juce::roundToInt(p->getValue() * 2.0f);  // 0=Stereo 1=Mono 2=Side
            p->setValueNotifyingHost(idx == 0 ? 0.5f : 0.0f);        // Stereo <-> Mono
            modeScreen_.repaint();
        }
    };
    content.addAndMakeVisible(modeScreen_);

    // ---- Static faceplate ----
    content.onPaint = [this](juce::Graphics& g) {
        // Full-frame sprite layer: source is @2x (1640x1440), drawn to kW x kH.
        auto blit = [&g](const juce::Image& img) {
            if (img.isValid())
                g.drawImage(img, 0, 0, kW, kH, 0, 0, img.getWidth(), img.getHeight());
        };

        // --- Static design composite (back to front) ---
        // Only the Blender sprites. Captions, knob scale labels, version, etc.
        // were procedural and are intentionally removed — re-added one by one.
        if (brassTexture_.isValid()) blit(brassTexture_);   // faceplate
        else                         g.fillAll(gain76::baseSurface);
        // meter housings are now drawn by the meter components (locked), not here
        blit(infolineImg_);                                  // footer strip
        blit(logoImg_);                                      // ALTERED AUDIO
        blit(gainTextImg_);                                  // GAIN 76
    };

    // ---- Right-click context menu (faceplate, footer, edit overlay) ----
    auto rightClickHandler = [this](const juce::MouseEvent& e) {
        juce::PopupMenu menu;
        if (!editMode_)
            menu.addItem(1, "Edit Layout");
        else
        {
            menu.addItem(2, "Save Layout");
            menu.addItem(3, "Exit Edit Mode");
        }
        menu.showMenuAsync(
            juce::PopupMenu::Options()
                .withTargetComponent(&content)
                .withTargetScreenArea({ e.getScreenX(), e.getScreenY(), 1, 1 }),
            [this](int result) {
                if      (result == 1) enterEditMode();
                else if (result == 2) layout_.save();
                else if (result == 3) exitEditMode();
            });
    };
    content.onRightClick      = rightClickHandler;
    footerComp_.onRightClick  = rightClickHandler;
    mainPanel_.onRightClick   = rightClickHandler;
    editOverlay_.onRightClick = rightClickHandler;

    // ---- Edit overlay (invisible until Edit Layout is chosen) ----
    editOverlay_.entries.add({ &gainKnob,    "KNOB"   });
    editOverlay_.entries.add({ &inMeter,     "IN MET" });
    editOverlay_.entries.add({ &outMeter,    "OUT MET"});
    editOverlay_.entries.add({ &powerBtn,    "POWER"  });
    editOverlay_.entries.add({ &footerComp_, "FOOTER" });
    editOverlay_.entries.add({ &mainPanel_,  "PANEL"  });
    editOverlay_.onChange = [this]() {
        layout_.knob     = gainKnob.getBounds();
        layout_.inMeter  = inMeter .getBounds();
        layout_.outMeter = outMeter.getBounds();
        layout_.power    = powerBtn.getBounds();
        layout_.footer   = footerComp_.getBounds();
        layout_.panel    = mainPanel_.getBounds();
        content.repaint();
        footerComp_.repaint();
    };
    content.addChildComponent(editOverlay_);

    setResizable(true, true);
    setResizeLimits(480, 480, 1440, 1440);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio((double)kW / (double)kH);   // 1:1
    setSize(kW, kH);

    startTimerHz(30);
}

GainEditor::~GainEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void GainEditor::applyLayout()
{
    gainKnob.setBounds(layout_.knob);
    inMeter .setBounds(layout_.inMeter);
    outMeter.setBounds(layout_.outMeter);
    powerBtn.setBounds(layout_.power);
    footerComp_.setBounds(layout_.footer);
    mainPanel_.setBounds(layout_.panel);

    const int cw   = footerComp_.getWidth() / 4;
    const int midY = footerComp_.getHeight() / 2;
    modeBox  .setBounds(58,           midY - 12, 96, 24);
    osBox    .setBounds(3 * cw + 110, midY - 12, 64, 24);
    peakReset.setBounds(cw,           0,         cw, footerComp_.getHeight());
}

void GainEditor::enterEditMode()
{
    editMode_ = true;
    editOverlay_.setVisible(true);
    editOverlay_.toFront(false);
}

void GainEditor::exitEditMode()
{
    editMode_ = false;
    editOverlay_.setVisible(false);
    content.repaint();
}

void GainEditor::drawAmberReadout(juce::Graphics& g, int W, int H, const juce::String& num, float scale)
{
    // Embossed amber glyphs are full-frame @2x sprites, each centred on x=720
    // with its baseline at y~721 — one uniform source cell captures every glyph
    // with a shared baseline. Lay them out in equal monospace cells.
    auto idxFor = [](juce::juce_wchar c) -> int
    {
        if (c >= '0' && c <= '9') return (int) (c - '0');
        if (c == '.') return 10;
        if (c == '+') return 11;
        if (c == '-') return 12;
        return -1;
    };
    constexpr int kSrcX = 688, kSrcY = 664, kSrcW = 64, kSrcH = 63;

    const float pad   = 4.0f;
    const float cellW = ((float) W - 2.0f * pad) / 5.0f * 0.95f * scale;   // fixed 5-cell grid (× scale)
    const float cellH = cellW * (float) kSrcH / (float) kSrcW;
    const int   N     = num.length();
    const float x0    = ((float) W - (float) N * cellW) * 0.5f;
    const float y0    = ((float) H - cellH) * 0.5f;

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    for (int i = 0; i < N; ++i)
    {
        const int idx = idxFor(num[i]);
        if (idx < 0 || ! glyphImg_[idx].isValid()) continue;
        g.drawImage(glyphImg_[idx],
                    juce::roundToInt(x0 + (float) i * cellW), juce::roundToInt(y0),
                    juce::roundToInt(cellW), juce::roundToInt(cellH),
                    kSrcX, kSrcY, kSrcW, kSrcH);
    }
}

void GainEditor::timerCallback()
{
    const float iL = analysis.inL.exchange(0.0f);
    const float iR = analysis.inR.exchange(0.0f);
    const float oL = analysis.outL.exchange(0.0f);
    const float oR = analysis.outR.exchange(0.0f);

    const float inLin  = juce::jmax(iL, iR);
    const float outLin = juce::jmax(oL, oR);
    inMeter .setLevel(inLin);
    outMeter.setLevel(outLin);

    // In/out numeric screens hold the running peak (dB) and restart when the
    // INPUT goes silent (~below -60 dB) so each new signal starts fresh.
    if (inLin < 0.001f)
    {
        inPeakDb_  = -60.0f;
        outPeakDb_ = -60.0f;
    }
    else
    {
        inPeakDb_  = juce::jmax(inPeakDb_,  juce::Decibels::gainToDecibels(inLin,  -60.0f));
        outPeakDb_ = juce::jmax(outPeakDb_, juce::Decibels::gainToDecibels(outLin, -60.0f));
    }

    glowLayer_.repaint();
    inDisplay_ .repaint();
    outDisplay_.repaint();
    modeScreen_.repaint();

    // infinite-hold peak readout (click the PEAK cell to reset)
    const float pk = juce::jmax(oL, oR);
    if (pk > 0.000001f)
        heldPeakDb = juce::jmax(heldPeakDb,
                                juce::Decibels::gainToDecibels(pk, -120.0f));

    // Screen glitch: fire when the output hits/exceeds 0 dB (clip). Trigger on
    // the rising edge, plus a chance to re-fire while clipping sustains.
    const bool clipping = (pk >= 1.0f);
    if ((clipping && ! wasClipping_)
        || (clipping && glitchT_ <= 0.0f && glitchRng_.nextFloat() < 0.15f))
        glitchT_ = kGlitchDur;
    wasClipping_ = clipping;
    if (glitchT_ > 0.0f)
        glitchT_ = juce::jmax(0.0f, glitchT_ - 1.0f / 30.0f);

    // sync power LED with the bypass parameter
    const auto* pB = apvts.getRawParameterValue(ParamID::gainBypass);
    const bool on = pB ? (pB->load() < 0.5f) : true;
    if (powerBtn.getToggleState() != on)
        powerBtn.setToggleState(on, juce::dontSendNotification);

    // refresh dynamic painted regions (footer readouts + dB display)
    footerComp_.repaint();
    valueDisplay_.repaint();
}

void GainEditor::paint(juce::Graphics& g)
{
    g.fillAll(gain76::graphBg);   // letterbox behind scaled content
}

void GainEditor::resized()
{
    const float scale = juce::jmin((float)getWidth() / (float)kW,
                                   (float)getHeight() / (float)kH);
    const float ox = ((float)getWidth()  - kW * scale) * 0.5f;
    const float oy = ((float)getHeight() - kH * scale) * 0.5f;
    content.setTransform(juce::AffineTransform::scale(scale).translated(ox, oy));

    if (!editMode_)
        applyLayout();

    editOverlay_.setBounds(0, 0, kW, kH);
}
