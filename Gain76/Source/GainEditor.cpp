#include "GainEditor.h"

// ============================================================
//  Layout constants (820 x 820 design space)
// ============================================================
namespace
{
    constexpr int kHeaderH = 64;
    constexpr int kKnobCX  = 410, kKnobCY = 365, kKnobD = 340;

    const juce::Rectangle<int> kMainPanel  { 16,  80, 788, 540 };
    const juce::Rectangle<int> kStripPanel { 16, 636, 788, 74 };
    const juce::Rectangle<int> kFooter     { 16, 726, 788, 52 };

    // dB range shared by all meters
    constexpr float kMeterLo = -60.0f, kMeterHi = 6.0f;
    inline float meterFrac(float lin)
    {
        const float db = juce::Decibels::gainToDecibels(lin, kMeterLo);
        return juce::jlimit(0.0f, 1.0f, (db - kMeterLo) / (kMeterHi - kMeterLo));
    }
}

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

// Filter 76 hero face knob, scaled up: matte-cream puck, floating tick
// ring, amber dot — and the live value printed big on the disc with the
// GAIN · DB label folded beneath it.
void Gain76LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float sliderPos, float startAngle, float endAngle,
                                         juce::Slider& s)
{
    const bool enabled = s.isEnabled();

    const auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(1.0f);
    const float size  = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const auto  area  = bounds.withSizeKeepingCentre(size, size);
    const float cx    = area.getCentreX(), cy = area.getCentreY();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    const float rDisc   = size * 0.30f;
    const float rTickIn = size * 0.375f;
    const float rMinor  = size * 0.43f;
    const float rMajor  = size * 0.47f;
    const float rTop    = size * 0.49f;

    auto px = [cx](float a, float r) { return cx + r * std::sin(a); };
    auto py = [cy](float a, float r) { return cy - r * std::cos(a); };

    // --- tick ring: fine minors all around, skip near the value tick ---
    const int nTicks = 48;
    const int majorEvery = nTicks / 4;
    for (int i = 0; i < nTicks; ++i)
    {
        const float a = (float)i / (float)nTicks * juce::MathConstants<float>::twoPi;
        const bool top   = (i == 0);
        const bool major = (i % majorEvery == 0);
        if (!top && std::abs(std::remainder(a - angle, juce::MathConstants<float>::twoPi)) < 0.12f)
            continue;
        const float rOut = top ? rTop : (major ? rMajor : rMinor);
        g.setColour(gain76::textPrimary.withAlpha(enabled ? 0.8f : 0.3f));
        g.drawLine(px(a, rTickIn), py(a, rTickIn), px(a, rOut), py(a, rOut),
                   top ? 1.8f : (major ? 1.2f : 0.85f));
    }

    // bold tick tracking the value
    if (enabled)
    {
        g.setColour(gain76::textPrimary);
        g.drawLine(px(angle, rTickIn), py(angle, rTickIn), px(angle, rTop), py(angle, rTop), 2.0f);
    }

    // --- matte cream puck: drop shadow, vertical sheen, thin dark edge ---
    g.setColour(juce::Colour(0x612A251C));
    g.fillEllipse(cx - rDisc, cy - rDisc + 3.0f, rDisc * 2.0f, rDisc * 2.0f);
    {
        juce::ColourGradient grad(juce::Colour(0xFFE9E1CC), cx, cy - rDisc,
                                  juce::Colour(0xFFD7CDB5), cx, cy + rDisc, false);
        grad.addColour(0.55, juce::Colour(0xFFE1D8C1));
        g.setGradientFill(grad);
        g.fillEllipse(cx - rDisc, cy - rDisc, rDisc * 2.0f, rDisc * 2.0f);
    }
    g.setColour(juce::Colour(0x802D261A));
    g.drawEllipse(cx - rDisc, cy - rDisc, rDisc * 2.0f, rDisc * 2.0f, 1.1f);
    {
        juce::Path arc;
        arc.addCentredArc(cx, cy, rDisc - 1.2f, rDisc - 1.2f, 0.0f,
                          juce::degreesToRadians(-62.0f), juce::degreesToRadians(62.0f), true);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.strokePath(arc, juce::PathStrokeType(1.1f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    // --- face dot: glowing amber when live ---
    {
        const float dr = juce::jmax(2.2f, size * 0.021f);
        const float dx = px(angle, rDisc * 0.8f), dy = py(angle, rDisc * 0.8f);
        if (enabled)
        {
            g.setColour(gain76::graphLine.withAlpha(0.35f));
            g.fillEllipse(dx - dr * 1.9f, dy - dr * 1.9f, dr * 3.8f, dr * 3.8f);
            g.setColour(juce::Colour(0xFFEFA42F));
            g.fillEllipse(dx - dr, dy - dr, dr * 2.0f, dr * 2.0f);
            g.setColour(juce::Colour(0xFFFFE2A6));
            g.fillEllipse(dx - dr * 0.65f, dy - dr * 0.65f, dr * 0.8f, dr * 0.8f);
        }
        else
        {
            g.setColour(gain76::ledOff);
            g.fillEllipse(dx - dr * 0.9f, dy - dr * 0.9f, dr * 1.8f, dr * 1.8f);
        }
    }

    // --- on-face value + label (Filter 76 hero idiom) ---
    {
        const juce::String value = s.getTextFromValue(s.getValue());
        g.setColour(juce::Colour(0xFF2A2620).withAlpha(enabled ? 1.0f : 0.45f));
        g.setFont(gain76::mono(size * 0.148f));
        g.drawText(value, (int)(cx - rDisc), (int)(cy - size * 0.11f),
                   (int)(rDisc * 2.0f), (int)(size * 0.15f), juce::Justification::centred);

        g.setColour(juce::Colour(0xFF6B6353).withAlpha(enabled ? 1.0f : 0.5f));
        g.setFont(gain76::mono(size * 0.062f));
        g.drawFittedText(juce::String::fromUTF8("GAIN \xc2\xb7 DB"),
                         (int)(cx - rDisc * 0.85f), (int)(cy + size * 0.048f),
                         (int)(rDisc * 1.7f), (int)(size * 0.080f),
                         juce::Justification::centred, 1, 0.9f);
    }
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
    const float w = (float)getWidth(), h = (float)getHeight();
    constexpr int nSeg = 34;
    const float gap = 2.0f;

    const juce::Rectangle<float> well(0.0f, 0.0f, w - 26.0f, h);
    g.setColour(gain76::graphBg);
    g.fillRoundedRectangle(well, 2.0f);

    const float frac = meterFrac(disp);

    const auto inner = well.reduced(2.5f);
    const float segH = (inner.getHeight() - (nSeg - 1) * gap) / (float)nSeg;
    for (int i = 0; i < nSeg; ++i)
    {
        const float segFrac = 1.0f - ((float)i + 0.5f) / (float)nSeg;
        const bool  lit     = segFrac <= frac;
        const juce::Colour c = segFrac > 0.9f ? gain76::warnOrange : gain76::graphLine;
        g.setColour(lit ? c.withAlpha(0.55f + segFrac * 0.45f)
                        : gain76::graphLine.withAlpha(0.10f));
        g.fillRect(inner.getX(), inner.getY() + (float)i * (segH + gap),
                   inner.getWidth(), segH);
    }

    // dB scale labels beside the column
    g.setColour(gain76::textSecond);
    g.setFont(gain76::mono(8.5f));
    for (float lDb : { 6.0f, 0.0f, -12.0f, -24.0f, -48.0f })
    {
        const float lf = (lDb - kMeterLo) / (kMeterHi - kMeterLo);
        const float ly = well.getBottom() - lf * well.getHeight();
        g.drawText(juce::String((int)lDb), (int)well.getRight() + 4, (int)ly - 5, 22, 10,
                   juce::Justification::centredLeft);
    }
}

// ============================================================
//  HoldMeter
// ============================================================

void HoldMeter::paint(juce::Graphics& g)
{
    const float w = (float)getWidth(), h = (float)getHeight();
    constexpr int nSeg = 46;
    const float gap = 2.0f;

    g.setColour(gain76::graphBg);
    g.fillRoundedRectangle(0.0f, 0.0f, w, h, 2.0f);

    const float frac     = meterFrac(disp);
    const float holdFrac = meterFrac(holdLin);

    const auto inner = juce::Rectangle<float>(0.0f, 0.0f, w, h).reduced(2.5f);
    const float segW = (inner.getWidth() - (nSeg - 1) * gap) / (float)nSeg;

    for (int i = 0; i < nSeg; ++i)
    {
        // bars grow from the OUTER edge toward the centre (R is mirrored)
        const float segFrac = ((float)i + 0.5f) / (float)nSeg;
        const bool  lit     = segFrac <= frac;
        const float sx = mirrored
            ? inner.getRight() - (float)(i + 1) * segW - (float)i * gap
            : inner.getX() + (float)i * (segW + gap);
        const juce::Colour c = segFrac > 0.9f ? gain76::warnOrange : gain76::graphLine;
        g.setColour(lit ? c.withAlpha(0.55f + segFrac * 0.45f)
                        : gain76::graphLine.withAlpha(0.10f));
        g.fillRect(sx, inner.getY(), segW, inner.getHeight());
    }

    // peak-hold marker — bright single bar that lingers
    if (holdFrac > 0.005f)
    {
        const float hx = mirrored
            ? inner.getRight() - holdFrac * inner.getWidth()
            : inner.getX() + holdFrac * inner.getWidth();
        g.setColour(juce::Colour(0xFFFFE2A6));
        g.fillRect(hx - 1.25f, inner.getY(), 2.5f, inner.getHeight());
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
    setLookAndFeel(&lnf);
    addAndMakeVisible(content);
    content.setBounds(0, 0, kW, kH);

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

    modeBox.addItemList({ "STEREO", "MONO", "SIDE" }, 1);
    content.addAndMakeVisible(modeBox);
    modeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, ParamID::gainMode, modeBox);

    osBox.addItemList({ "1x", "4x", "8x" }, 1);
    content.addAndMakeVisible(osBox);
    osAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, ParamID::gainOversamp, osBox);

    powerBtn.setClickingTogglesState(true);
    powerBtn.onClick = [this]() {
        if (auto* p = apvts.getParameter(ParamID::gainBypass))
            p->setValueNotifyingHost(powerBtn.getToggleState() ? 0.0f : 1.0f);
    };
    content.addAndMakeVisible(powerBtn);

    content.addAndMakeVisible(inMeter);
    content.addAndMakeVisible(outMeter);
    content.addAndMakeVisible(stripL);
    content.addAndMakeVisible(stripR);

    peakReset.fn = [this]() { heldPeakDb = -120.0f; };
    content.addAndMakeVisible(peakReset);

    // ---- Static faceplate ----
    content.onPaint = [this](juce::Graphics& g) {
        g.fillAll(gain76::baseSurface);

        {
            juce::ColourGradient sheen(juce::Colour(0x80FFFDF7), 0.0f, 0.0f,
                                       juce::Colour(0x00FFFDF7), 0.0f, 120.0f, false);
            g.setGradientFill(sheen);
            g.fillRect(0, 0, kW, 120);
        }

        // header wordmark
        g.setColour(gain76::textPrimary);
        g.setFont(gain76::heading(24.0f).withExtraKerningFactor(0.22f));
        g.drawText("ALTERED AUDIO", 24, 10, 470, 26, juce::Justification::centredLeft);
        g.setColour(gain76::amberLo);
        g.setFont(gain76::heading(11.0f).withExtraKerningFactor(0.14f));
        g.drawText("GAIN 76", 26, 38, 300, 14, juce::Justification::centredLeft);

        // panels
        auto panel = [&g](juce::Rectangle<int> r) {
            const auto rf = r.toFloat();
            g.setColour(gain76::baseSurface.brighter(0.015f));
            g.fillRoundedRectangle(rf, 10.0f);
            g.setColour(juce::Colour(0x66FFFCF4));
            g.fillRect(rf.getX() + 2.0f, rf.getY() + 1.0f, rf.getWidth() - 4.0f, 1.5f);
            g.setColour(gain76::border);
            g.drawRoundedRectangle(rf, 10.0f, 1.0f);
        };
        panel(kMainPanel);
        panel(kStripPanel);

        const auto label = [&g](int lx, int ly, int lw, const char* t,
                                juce::Justification j = juce::Justification::centredLeft) {
            g.setColour(gain76::textSecond);
            g.setFont(gain76::mono(9.0f).withExtraKerningFactor(0.08f));
            g.drawText(t, lx, ly, lw, 11, j);
        };

        // meter captions
        label(inMeter.getX(),  inMeter.getY() - 18, 60, "INPUT");
        label(outMeter.getX() - 14, outMeter.getY() - 18, 70, "OUTPUT",
              juce::Justification::centredRight);

        // knob scale labels -24 / 0 / +24 (270° sweep, gap at the bottom)
        {
            const float cx = (float)kKnobCX, cy = (float)kKnobCY;
            const float r  = kKnobD * 0.565f;
            g.setColour(gain76::textSecond);
            g.setFont(gain76::mono(12.0f));
            auto lab = [&](float aDeg, const char* t) {
                const float a = juce::degreesToRadians(aDeg);
                g.drawText(t, (int)(cx + r * std::sin(a)) - 22, (int)(cy - r * std::cos(a)) - 8,
                           44, 16, juce::Justification::centred);
            };
            lab(-135.0f, "-24");
            lab(0.0f,    "0");
            lab(135.0f,  "+24");
        }

        // strip channel tags
        label(kStripPanel.getX() + 12, kStripPanel.getCentreY() - 5, 14, "L");
        label(kStripPanel.getRight() - 22, kStripPanel.getCentreY() - 5, 14, "R");

        // ---- footer: 4 cells — MODE | PEAK | LUFS | OVERSAMPLING ----
        {
            const auto rf = kFooter.toFloat();
            g.setColour(gain76::warmBeige);
            g.fillRoundedRectangle(rf, 6.0f);
            g.setColour(gain76::border);
            g.drawRoundedRectangle(rf, 6.0f, 1.0f);

            const int cellW = kFooter.getWidth() / 4;
            const int cy    = kFooter.getCentreY();
            for (int i = 1; i < 4; ++i)
            {
                g.setColour(gain76::border.withAlpha(0.5f));
                g.drawVerticalLine(kFooter.getX() + i * cellW, rf.getY() + 10.0f, rf.getBottom() - 10.0f);
            }

            auto cellLabel = [&](int cell, const char* t) {
                label(kFooter.getX() + cell * cellW + 16, cy - 5, 90, t);
            };
            auto cellValue = [&](int cell, const juce::String& v) {
                g.setColour(gain76::textPrimary);
                g.setFont(gain76::mono(12.0f));
                g.drawText(v, kFooter.getX() + cell * cellW + 62, cy - 7, cellW - 70, 14,
                           juce::Justification::centredLeft);
            };

            cellLabel(0, "MODE");
            cellLabel(1, "PEAK");
            cellValue(1, (heldPeakDb <= -119.0f
                            ? juce::String("-\xe2\x88\x9e dB")
                            : (heldPeakDb >= 0.005f ? "+" : "")
                              + juce::String(heldPeakDb, 2) + " dB"));
            cellLabel(2, "LUFS");
            {
                const float lu = analysis.lufs.load();
                cellValue(2, lu <= -99.0f ? juce::String("-")
                                          : juce::String(lu, 1));
            }
            cellLabel(3, "OVERSAMPLING");
        }

        // version
        g.setColour(gain76::textSecond);
        g.setFont(gain76::mono(10.0f));
        g.drawText("v" + juce::String(JucePlugin_VersionString),
                   kW - 120, kH - 28, 96, 14, juce::Justification::centredRight);
    };

    setResizable(true, true);
    setResizeLimits(560, 560, 1640, 1640);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio(1.0);
    setSize(kW, kH);

    startTimerHz(30);
}

GainEditor::~GainEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void GainEditor::timerCallback()
{
    const float iL = analysis.inL.exchange(0.0f);
    const float iR = analysis.inR.exchange(0.0f);
    const float oL = analysis.outL.exchange(0.0f);
    const float oR = analysis.outR.exchange(0.0f);

    inMeter .setLevel(juce::jmax(iL, iR));
    outMeter.setLevel(juce::jmax(oL, oR));
    stripL.setLevel(oL);
    stripR.setLevel(oR);

    // infinite-hold peak readout (click the PEAK cell to reset)
    const float pk = juce::jmax(oL, oR);
    if (pk > 0.000001f)
        heldPeakDb = juce::jmax(heldPeakDb,
                                juce::Decibels::gainToDecibels(pk, -120.0f));

    // sync power LED with the bypass parameter
    const auto* pB = apvts.getRawParameterValue(ParamID::gainBypass);
    const bool on = pB ? (pB->load() < 0.5f) : true;
    if (powerBtn.getToggleState() != on)
        powerBtn.setToggleState(on, juce::dontSendNotification);

    // refresh dynamic painted regions (footer readouts)
    content.repaint(kFooter);
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

    powerBtn.setBounds(kW - 82, 6, 44, 58);

    gainKnob.setBounds(kKnobCX - kKnobD / 2, kKnobCY - kKnobD / 2, kKnobD, kKnobD);

    inMeter .setBounds(kMainPanel.getX() + 28,        kMainPanel.getY() + 40, 64, kMainPanel.getHeight() - 80);
    outMeter.setBounds(kMainPanel.getRight() - 92,    kMainPanel.getY() + 40, 64, kMainPanel.getHeight() - 80);

    const int stripW = (kStripPanel.getWidth() - 80) / 2;
    stripL.setBounds(kStripPanel.getX() + 30,              kStripPanel.getY() + 18, stripW, kStripPanel.getHeight() - 36);
    stripR.setBounds(kStripPanel.getRight() - 30 - stripW, kStripPanel.getY() + 18, stripW, kStripPanel.getHeight() - 36);

    const int cellW = kFooter.getWidth() / 4;
    modeBox.setBounds(kFooter.getX() + 58,              kFooter.getCentreY() - 12, 96, 24);
    osBox  .setBounds(kFooter.getX() + 3 * cellW + 110, kFooter.getCentreY() - 12, 64, 24);

    peakReset.setBounds(kFooter.getX() + cellW, kFooter.getY(), cellW, kFooter.getHeight());
}
