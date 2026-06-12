#include "AuroraFilterEditor.h"

// ============================================================
//  Layout constants (1400 x 900 design space)
// ============================================================
namespace
{
    constexpr int kHeaderH   = 96;
    constexpr int kDisplayY  = 106, kDisplayH = 330;            // the spectrum dominates
    constexpr int kFilterY   = 448, kFilterH  = 200;
    constexpr int kBottomY   = 660, kBottomH  = 186;
    constexpr int kFooterY   = 856;

    const juce::Rectangle<int> kModPanel  { 16,   kBottomY,  430,  kBottomH };
    const juce::Rectangle<int> kLfoPanel  { 458,  kBottomY,  470,  kBottomH };
    const juce::Rectangle<int> kEnvPanel  { 944,  kBottomY,  440,  kBottomH };
    const juce::Rectangle<int> kTypePanel { 16,   kFilterY,  150,  kFilterH };
    const juce::Rectangle<int> kKnobPanel { 180,  kFilterY,  1040, kFilterH };
    const juce::Rectangle<int> kSlopePanel{ 1234, kFilterY,  150,  kFilterH };
    const juce::Rectangle<int> kMeterPanel{ 1216, kDisplayY, 168,  kDisplayH };
    const juce::Rectangle<int> kFooter    { 16,   kFooterY,  1368, 40 };

    // UI row → filterType parameter index (LP HP BP NOTCH PEAK SHELF)
    constexpr int kTypeMap[6] = { 0, 1, 2, 3, 5, 6 };
    const char*   kTypeNames[6] = { "LP", "HP", "BP", "NOTCH", "PEAK", "SHELF" };

    // ---- Factory presets: name + (paramID, value) pairs ----
    struct PresetDef
    {
        const char* name;
        std::vector<std::pair<const char*, float>> values;
    };

    const std::vector<PresetDef>& presets()
    {
        static const std::vector<PresetDef> p = {
          { "INIT",
            { { ParamID::filterType, 0 }, { ParamID::filterFreq, 1000 }, { ParamID::filterQ, 0.707f },
              { ParamID::filterGain, 0 }, { ParamID::filterSlope, 0 },   { ParamID::filterDrive, 0 },
              { ParamID::filterMix, 1 },  { ParamID::filterOutput, 0 },  { ParamID::fltModSource, 0 },
              { ParamID::fltModAmount, 0 } } },
          { "ANALOG LOWPASS",
            { { ParamID::filterType, 0 }, { ParamID::filterFreq, 824 },  { ParamID::filterQ, 3.2f },
              { ParamID::filterSlope, 1 },{ ParamID::filterDrive, 4.1f },{ ParamID::filterMix, 1 },
              { ParamID::filterOutput, 0 }, { ParamID::fltModSource, 0 } } },
          { "ACID SQUELCH",
            { { ParamID::filterType, 0 }, { ParamID::filterFreq, 320 },  { ParamID::filterQ, 14 },
              { ParamID::filterSlope, 1 },{ ParamID::filterDrive, 12 },  { ParamID::filterMix, 1 },
              { ParamID::fltModSource, 3 }, { ParamID::fltModDest, 0 },  { ParamID::fltModAmount, 0.8f },
              { ParamID::fltEnvAttack, 2 }, { ParamID::fltEnvRelease, 120 }, { ParamID::fltEnvSens, 1.4f } } },
          { "SLOW SWEEP",
            { { ParamID::filterType, 0 }, { ParamID::filterFreq, 800 },  { ParamID::filterQ, 6 },
              { ParamID::filterSlope, 1 },{ ParamID::filterDrive, 3 },   { ParamID::fltModSource, 1 },
              { ParamID::fltModDest, 0 }, { ParamID::fltModAmount, 0.6f },
              { ParamID::fltLfoARate, 0.15f }, { ParamID::fltLfoAWave, 0 }, { ParamID::fltLfoADepth, 1 } } },
          { "TELEPHONE",
            { { ParamID::filterType, 2 }, { ParamID::filterFreq, 1700 }, { ParamID::filterQ, 4 },
              { ParamID::filterDrive, 6 },{ ParamID::filterMix, 1 },     { ParamID::fltModSource, 0 } } },
          { "NOTCH WOBBLE",
            { { ParamID::filterType, 3 }, { ParamID::filterFreq, 1200 }, { ParamID::filterQ, 2 },
              { ParamID::fltModSource, 2 }, { ParamID::fltModDest, 0 },  { ParamID::fltModAmount, 0.5f },
              { ParamID::fltLfoBRate, 0.8f }, { ParamID::fltLfoBWave, 1 }, { ParamID::fltLfoBDepth, 1 } } },
        };
        return p;
    }
}

// ============================================================
//  AuroraLookAndFeel
// ============================================================

AuroraLookAndFeel::AuroraLookAndFeel()
{
    // Slider text boxes are dark amber readout wells
    setColour(juce::Slider::textBoxTextColourId,       aurora::graphLine);
    setColour(juce::Slider::textBoxBackgroundColourId, aurora::graphBg);
    setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId,               aurora::textPrimary);
    setColour(juce::TextButton::textColourOffId,       aurora::textPrimary);
    setColour(juce::TextButton::textColourOnId,        aurora::textPrimary);
    setColour(juce::PopupMenu::backgroundColourId,     aurora::cream);
    setColour(juce::PopupMenu::textColourId,           aurora::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, aurora::amber);
    setColour(juce::PopupMenu::highlightedTextColourId, aurora::textOnAccent);
}

// Filter 76 knob: flat matte-cream puck, floating tick ring with an air
// gap (majors every quarter, bold mark at top), a bold tick tracking the
// value aligned with a glowing amber dot on the face. Hero ("face") knobs
// print the value big on the disc with the unit folded into a small label.
void AuroraLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float sliderPos, float startAngle, float endAngle,
                                         juce::Slider& s)
{
    const bool face    = (bool)s.getProperties().getWithDefault("face", false);
    const bool enabled = s.isEnabled();

    const auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(1.0f);
    const float size  = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const auto  area  = bounds.withSizeKeepingCentre(size, size);
    const float cx    = area.getCentreX(), cy = area.getCentreY();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // geometry mirrors Knob.jsx (viewBox 100: disc r30, ticks 37.5..49)
    const float rDisc   = size * 0.30f;
    const float rTickIn = size * 0.375f;
    const float rMinor  = size * 0.43f;
    const float rMajor  = size * 0.47f;
    const float rTop    = size * 0.49f;

    auto px = [cx](float a, float r) { return cx + r * std::sin(a); };
    auto py = [cy](float a, float r) { return cy - r * std::cos(a); };

    // --- tick ring: fine minors all around, skip near the value tick ---
    const int nTicks = size > 100.0f ? 40 : 28;
    const int majorEvery = juce::jmax(1, nTicks / 4);
    for (int i = 0; i < nTicks; ++i)
    {
        const float a = (float)i / (float)nTicks * juce::MathConstants<float>::twoPi;
        const bool top   = (i == 0);
        const bool major = (i % majorEvery == 0);
        if (!top && std::abs(std::remainder(a - angle, juce::MathConstants<float>::twoPi)) < 0.16f)
            continue;
        const float rOut = top ? rTop : (major ? rMajor : rMinor);
        g.setColour(aurora::textPrimary.withAlpha(enabled ? 0.8f : 0.3f));
        g.drawLine(px(a, rTickIn), py(a, rTickIn), px(a, rOut), py(a, rOut),
                   top ? 1.6f : (major ? 1.1f : 0.8f));
    }

    // bold tick tracking the value (hidden while disabled)
    if (enabled)
    {
        g.setColour(aurora::textPrimary);
        g.drawLine(px(angle, rTickIn), py(angle, rTickIn), px(angle, rTop), py(angle, rTop), 1.8f);
    }

    // --- matte cream puck: drop shadow, vertical sheen, thin dark edge ---
    g.setColour(juce::Colour(0x612A251C));
    g.fillEllipse(cx - rDisc, cy - rDisc + 2.2f, rDisc * 2.0f, rDisc * 2.0f);
    {
        juce::ColourGradient grad(juce::Colour(0xFFE9E1CC), cx, cy - rDisc,
                                  juce::Colour(0xFFD7CDB5), cx, cy + rDisc, false);
        grad.addColour(0.55, juce::Colour(0xFFE1D8C1));
        g.setGradientFill(grad);
        g.fillEllipse(cx - rDisc, cy - rDisc, rDisc * 2.0f, rDisc * 2.0f);
    }
    g.setColour(juce::Colour(0x802D261A));
    g.drawEllipse(cx - rDisc, cy - rDisc, rDisc * 2.0f, rDisc * 2.0f, 0.9f);
    {
        // faint top-light crescent
        juce::Path arc;
        arc.addCentredArc(cx, cy, rDisc - 1.0f, rDisc - 1.0f, 0.0f,
                          juce::degreesToRadians(-62.0f), juce::degreesToRadians(62.0f), true);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.strokePath(arc, juce::PathStrokeType(0.9f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    // --- face dot: glowing amber when live, flat grey when disabled ---
    {
        const float dr = juce::jmax(1.6f, size * 0.021f);
        const float dx = px(angle, rDisc * 0.8f), dy = py(angle, rDisc * 0.8f);
        if (enabled)
        {
            g.setColour(aurora::graphLine.withAlpha(0.35f));
            g.fillEllipse(dx - dr * 1.9f, dy - dr * 1.9f, dr * 3.8f, dr * 3.8f);
            g.setColour(juce::Colour(0xFFEFA42F));
            g.fillEllipse(dx - dr, dy - dr, dr * 2.0f, dr * 2.0f);
            g.setColour(juce::Colour(0xFFFFE2A6));
            g.fillEllipse(dx - dr * 0.65f, dy - dr * 0.65f, dr * 0.8f, dr * 0.8f);
        }
        else
        {
            g.setColour(aurora::ledOff);
            g.fillEllipse(dx - dr * 0.9f, dy - dr * 0.9f, dr * 1.8f, dr * 1.8f);
        }
    }

    // --- on-face value + label (hero knobs) ---
    if (face)
    {
        const juce::String value = s.getTextFromValue(s.getValue());
        const juce::String label = s.getProperties().getWithDefault("faceLabel", "").toString();

        g.setColour(juce::Colour(0xFF2A2620).withAlpha(enabled ? 1.0f : 0.45f));
        g.setFont(aurora::mono(size * 0.12f));
        g.drawText(value, (int)(cx - rDisc), (int)(cy - size * 0.10f),
                   (int)(rDisc * 2.0f), (int)(size * 0.14f), juce::Justification::centred);

        // tight tracking + narrow band, kept above the amber dot's travel arc
        g.setColour(juce::Colour(0xFF6B6353).withAlpha(enabled ? 1.0f : 0.5f));
        g.setFont(aurora::mono(size * 0.060f).withExtraKerningFactor(0.02f));
        g.drawFittedText(label,
                         (int)(cx - rDisc * 0.85f), (int)(cy + size * 0.045f),
                         (int)(rDisc * 1.7f), (int)(size * 0.075f),
                         juce::Justification::centred, 1, 0.9f);
    }
}

juce::Label* AuroraLookAndFeel::createSliderTextBox(juce::Slider& s)
{
    auto* l = LookAndFeel_V4::createSliderTextBox(s);
    l->setFont(aurora::mono(11.0f));
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::backgroundColourId, aurora::graphBg);
    l->setColour(juce::Label::textColourId,       aurora::graphLine);
    l->setColour(juce::Label::outlineColourId,    juce::Colours::transparentBlack);
    return l;
}

void AuroraLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                             const juce::Colour& bg, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(0.5f);
    const bool optionRow = (bool)b.getProperties().getWithDefault("optionRow", false);
    const bool sel = (bg == aurora::amber);   // updateButtonStates marks selection with amber

    if (optionRow)
    {
        // vertical option row: flat with hairline; selected = raised cream + lit LED
        if (sel || highlighted)
        {
            g.setColour(sel ? aurora::cream : aurora::cream.withAlpha(0.45f));
            g.fillRoundedRectangle(r, 4.0f);
        }
        g.setColour(sel ? aurora::border : aurora::border.withAlpha(0.42f));
        g.drawRoundedRectangle(r, 4.0f, 1.0f);

        const float lx = r.getX() + 12.0f, ly = r.getCentreY();
        if (sel)
        {
            g.setColour(aurora::led.withAlpha(0.35f));
            g.fillEllipse(lx - 5.5f, ly - 5.5f, 11.0f, 11.0f);
            g.setColour(aurora::led);
            g.fillEllipse(lx - 3.5f, ly - 3.5f, 7.0f, 7.0f);
            g.setColour(aurora::amberLo);
        }
        else
            g.setColour(aurora::inkFaint);
        g.drawEllipse(lx - 3.5f, ly - 3.5f, 7.0f, 7.0f, 1.0f);
        return;
    }

    const juce::Colour fill =
        sel ? (down ? aurora::amberLo : highlighted ? aurora::amberHi : aurora::amber)
            : (down ? aurora::sand    : highlighted ? aurora::cream.brighter(0.04f) : aurora::cream);
    g.setColour(fill);
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(sel ? aurora::amberLo : aurora::border);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);
}

void AuroraLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    const bool optionRow = (bool)b.getProperties().getWithDefault("optionRow", false);
    const bool sel = (b.findColour(juce::TextButton::buttonColourId) == aurora::amber);

    g.setColour(!optionRow && sel ? aurora::textOnAccent : aurora::textPrimary);
    g.setFont(aurora::mono(11.0f).withExtraKerningFactor(0.06f));

    auto r = b.getLocalBounds();
    if (optionRow)
        g.drawText(b.getButtonText(), r.withTrimmedLeft(26).withTrimmedRight(4),
                   juce::Justification::centredLeft);
    else
        g.drawText(b.getButtonText(), r, juce::Justification::centred);
}

juce::Font AuroraLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    return aurora::mono(11.0f);
}

// Flat combo box (Select.jsx): cream fill, hairline border, thin chevron
void AuroraLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height).reduced(0.5f);
    g.setColour(isButtonDown ? aurora::sand
                             : box.isMouseOver() ? aurora::cream.brighter(0.04f) : aurora::cream);
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(aurora::border);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);

    const float chx = (float)width - 17.0f, chy = (float)height * 0.5f;
    juce::Path ch;
    ch.startNewSubPath(chx - 4.0f, chy - 2.0f);
    ch.lineTo(chx, chy + 2.5f);
    ch.lineTo(chx + 4.0f, chy - 2.0f);
    g.setColour(aurora::textSecond);
    g.strokePath(ch, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
}

juce::Font AuroraLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return aurora::mono(12.0f).withExtraKerningFactor(0.06f);
}

void AuroraLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(10, 1, box.getWidth() - 36, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
    label.setColour(juce::Label::textColourId, aurora::textPrimary);
}

// ============================================================
//  ResponseDisplay
// ============================================================

ResponseDisplay::ResponseDisplay(juce::AudioProcessorValueTreeState& vts,
                                 FilterAnalysisSource& src)
    : apvts(vts), analysis(src)
{
    for (auto& v : specDisp) v = kDbBot - 24.0f;
}

float ResponseDisplay::xForFreq(float hz, float w) const
{
    return (std::log10(juce::jlimit(kMinF, kMaxF, hz)) - std::log10(kMinF))
         / (std::log10(kMaxF) - std::log10(kMinF)) * w;
}

float ResponseDisplay::freqForX(float x, float w) const
{
    return std::pow(10.0f, std::log10(kMinF)
                  + (x / w) * (std::log10(kMaxF) - std::log10(kMinF)));
}

float ResponseDisplay::yForDb(float db, float h) const
{
    return (kDbTop - db) / (kDbTop - kDbBot) * h;
}

void ResponseDisplay::refresh()
{
    constexpr int kSize = FilterAnalysisSource::kFFTSize;
    constexpr int kHalf = kSize / 2;

    // Snapshot the most recent kFFTSize samples in time order (oldest first)
    const int wp = analysis.writePos.load(std::memory_order_acquire);
    for (int i = 0; i < kSize; ++i)
        fftWork[i] = analysis.ring[(wp + i) & (kSize - 1)];
    std::memset(fftWork + kSize, 0, sizeof(float) * kSize);

    window.multiplyWithWindowingTable(fftWork, kSize);
    fft.performFrequencyOnlyForwardTransform(fftWork);

    const float sr   = juce::jmax(8000.0f, analysis.sampleRate.load());
    const float norm = 4.0f / (float)kSize;   // Hann coherent-gain compensated
    const float w    = (float)juce::jmax(1, getWidth());

    for (int p = 0; p < kSpecPoints; ++p)
    {
        const float fLo = freqForX((float)p       / kSpecPoints * w, w);
        const float fHi = freqForX((float)(p + 1) / kSpecPoints * w, w);
        const float bLo = fLo / (sr * 0.5f) * (float)kHalf;
        const float bHi = fHi / (sr * 0.5f) * (float)kHalf;

        float mag;
        if (bHi - bLo <= 1.0f)
        {
            // narrower than one bin — interpolate between neighbours so the
            // low end draws a continuous curve instead of a staircase
            const float bc = juce::jlimit(1.0f, (float)(kHalf - 2), 0.5f * (bLo + bHi));
            const int   b0 = (int)bc;
            const float t  = bc - (float)b0;
            mag = fftWork[b0] * (1.0f - t) + fftWork[b0 + 1] * t;
        }
        else
        {
            mag = 0.0f;
            const int b0 = juce::jlimit(1, kHalf - 1, (int)std::ceil(bLo));
            const int b1 = juce::jlimit(b0, kHalf - 1, (int)bHi);
            for (int b = b0; b <= b1; ++b)
                mag = juce::jmax(mag, fftWork[b]);
        }

        specWork[p] = juce::Decibels::gainToDecibels(mag * norm, -100.0f) + 18.0f;
    }

    // light spatial smoothing — kills single-bin jaggies, keeps peaks
    for (int p = 0; p < kSpecPoints; ++p)
    {
        const float prev = specWork[juce::jmax(0, p - 1)];
        const float next = specWork[juce::jmin(kSpecPoints - 1, p + 1)];
        const float sm   = 0.25f * prev + 0.5f * specWork[p] + 0.25f * next;

        // ballistics: instant attack, ~30 dB/s release at 30 fps
        specDisp[p] = (sm > specDisp[p]) ? sm : specDisp[p] - 1.0f;
    }

    repaint();
}

// dB the node sits at for a given type — the exact response at the cutoff.
// LP/HP/BP peak at 20*log10(Q); PEAK at its gain; shelves at half gain at
// the corner; NOTCH/ALLPASS have no meaningful node height (0 dB line).
static float nodeDbFor(int type, float q, float gainDb)
{
    switch (type)
    {
        case 0: case 1: case 2:
            return juce::Decibels::gainToDecibels(juce::jmax(0.05f, q));
        case 5:
            return gainDb;
        case 6: case 7:
            return gainDb * 0.5f;
        default:
            return 0.0f;
    }
}

void ResponseDisplay::mouseDown(const juce::MouseEvent& e) { mouseDrag(e); }

void ResponseDisplay::mouseDrag(const juce::MouseEvent& e)
{
    const float w = (float)getWidth(), h = (float)getHeight();

    if (auto* pf = apvts.getParameter(ParamID::filterFreq))
        pf->setValueNotifyingHost(pf->getNormalisableRange().convertTo0to1(
            juce::jlimit(kMinF, kMaxF, freqForX((float)e.x, w))));

    // Invert the node mapping so the node lands exactly under the mouse
    const float targetDb = kDbTop - (float)e.y / h * (kDbTop - kDbBot);
    const auto* pType = apvts.getRawParameterValue(ParamID::filterType);
    const int   type  = pType ? (int)pType->load() : 0;

    switch (type)
    {
        case 0: case 1: case 2:   // LP / HP / BP — dB → Q
            if (auto* pq = apvts.getParameter(ParamID::filterQ))
            {
                const float q = juce::jlimit(0.1f, 24.0f,
                                    std::pow(10.0f, targetDb / 20.0f));
                pq->setValueNotifyingHost(pq->getNormalisableRange().convertTo0to1(q));
            }
            break;
        case 5:                    // PEAK — dB → gain
            if (auto* pg = apvts.getParameter(ParamID::filterGain))
                pg->setValueNotifyingHost(pg->getNormalisableRange().convertTo0to1(
                    juce::jlimit(-24.0f, 24.0f, targetDb)));
            break;
        case 6: case 7:            // SHELF — corner sits at half gain
            if (auto* pg = apvts.getParameter(ParamID::filterGain))
                pg->setValueNotifyingHost(pg->getNormalisableRange().convertTo0to1(
                    juce::jlimit(-24.0f, 24.0f, targetDb * 2.0f)));
            break;
        default:                   // NOTCH / ALLPASS — vertical has no node meaning
            break;
    }
}

void ResponseDisplay::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (auto* pd = apvts.getParameter(ParamID::filterDrive))
    {
        const float cur = apvts.getRawParameterValue(ParamID::filterDrive)->load();
        const float nxt = juce::jlimit(0.0f, 24.0f, cur + wheel.deltaY * 8.0f);
        pd->setValueNotifyingHost(pd->getNormalisableRange().convertTo0to1(nxt));
    }
}

void ResponseDisplay::mouseDoubleClick(const juce::MouseEvent&)
{
    if (auto* pf = apvts.getParameter(ParamID::filterFreq))
        pf->setValueNotifyingHost(pf->getNormalisableRange().convertTo0to1(1000.0f));
    if (auto* pq = apvts.getParameter(ParamID::filterQ))
        pq->setValueNotifyingHost(pq->getNormalisableRange().convertTo0to1(0.707f));
}

void ResponseDisplay::paint(juce::Graphics& g)
{
    const float w = (float)getWidth(), h = (float)getHeight();

    // dark precision glass — radial falloff to the corners
    {
        juce::ColourGradient bg(juce::Colour(0xFF201E1A), w * 0.5f, h * 0.3f,
                                juce::Colour(0xFF0B0A08), w * 0.5f, h * 1.6f, true);
        bg.addColour(0.52, juce::Colour(0xFF15140F));
        g.setGradientFill(bg);
        g.fillRoundedRectangle(0.0f, 0.0f, w, h, 6.0f);
    }

    // subtle amber log grid
    g.setColour(aurora::graphLine.withAlpha(0.07f));
    for (float f : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f })
        g.drawVerticalLine((int)xForFreq(f, w), 4.0f, h - 18.0f);
    g.setColour(aurora::graphLine.withAlpha(0.06f));
    for (float db : { 12.f, -12.f, -24.f })
        g.drawHorizontalLine((int)yForDb(db, h), 4.0f, w - 4.0f);
    g.setColour(aurora::graphLine.withAlpha(0.16f));
    g.drawHorizontalLine((int)yForDb(0.0f, h), 4.0f, w - 4.0f);

    // Live spectrum — translucent gradient-filled analyzer + crisp top line.
    // The analyzer uses its own, deeper dB scale (+12..-90) than the response
    // graph, so the body of the spectrum fills the display instead of only
    // the peaks poking up from the bottom edge.
    {
        static constexpr float kSpecTop = 12.0f, kSpecBot = -90.0f;
        auto ySpec = [h](float db) {
            return (kSpecTop - juce::jlimit(kSpecBot, kSpecTop, db))
                 / (kSpecTop - kSpecBot) * h; };

        // Only draw while there is actual signal — digital silence parks
        // every point at the -82 dB noise floor, which otherwise renders
        // as a jittery line along the bottom edge.
        float maxDb = -200.0f;
        for (int p = 0; p < kSpecPoints; ++p)
            maxDb = juce::jmax(maxDb, specDisp[p]);

        if (maxDb > -70.0f)
        {
            juce::Path spec;
            spec.startNewSubPath(0.0f, h);
            for (int p = 0; p < kSpecPoints; ++p)
                spec.lineTo((float)p / (kSpecPoints - 1) * w, ySpec(specDisp[p]));
            spec.lineTo(w, h);
            spec.closeSubPath();

            juce::ColourGradient fillGrad(aurora::graphLine.withAlpha(0.45f), 0.0f, 0.0f,
                                          aurora::graphLine.withAlpha(0.0f),  0.0f, h, false);
            fillGrad.addColour(0.4, aurora::graphLine.withAlpha(0.18f));
            g.setGradientFill(fillGrad);
            g.fillPath(spec);

            juce::Path specLine;
            for (int p = 0; p < kSpecPoints; ++p)
            {
                const float sx = (float)p / (kSpecPoints - 1) * w;
                const float sy = ySpec(specDisp[p]);
                if (p == 0) specLine.startNewSubPath(sx, sy);
                else        specLine.lineTo(sx, sy);
            }
            g.setColour(aurora::graphLine.withAlpha(0.85f));
            g.strokePath(specLine, juce::PathStrokeType(1.25f, juce::PathStrokeType::curved));
        }
    }

    // Exact response curve
    const auto* pType  = apvts.getRawParameterValue(ParamID::filterType);
    const auto* pFreq  = apvts.getRawParameterValue(ParamID::filterFreq);
    const auto* pQ     = apvts.getRawParameterValue(ParamID::filterQ);
    const auto* pGain  = apvts.getRawParameterValue(ParamID::filterGain);
    const auto* pSlope = apvts.getRawParameterValue(ParamID::filterSlope);

    const auto  type  = static_cast<FilterModule::FilterType>(pType ? (int)pType->load() : 0);
    const float freq  = pFreq  ? pFreq->load()  : 1000.0f;
    const float q     = pQ     ? pQ->load()     : 0.707f;
    const float gain  = pGain  ? pGain->load()  : 0.0f;
    const int   slope = pSlope ? (int)pSlope->load() : 0;
    const double sr   = (double)analysis.sampleRate.load();

    // Ghost curve — the LIVE modulated filter (LFO / env), translucent,
    // so you can see what the modulation is actually doing to the cutoff.
    if (analysis.modActive.load())
    {
        const float mFreq = analysis.modFreq.load();
        const float mQ    = analysis.modQ.load();
        juce::Path ghost;
        for (int i = 0; i <= 160; ++i)
        {
            const float px = (float)i / 160.0f * w;
            const float db = FilterModule::responseDb(type, slope, mFreq, mQ, gain,
                                                      freqForX(px, w), sr);
            const float py = juce::jlimit(-10.0f, h + 10.0f,
                              yForDb(juce::jlimit(kDbBot - 8.0f, kDbTop + 8.0f, db), h));
            if (i == 0) ghost.startNewSubPath(px, py);
            else        ghost.lineTo(px, py);
        }
        g.setColour(aurora::graphLine.withAlpha(0.40f));
        g.strokePath(ghost, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved));

        // Small dot at the live cutoff position
        const float gx = xForFreq(mFreq, w);
        const float gy = yForDb(juce::jlimit(kDbBot, kDbTop,
                            nodeDbFor((int)type, mQ, gain)), h);
        g.fillEllipse(gx - 3.5f, gy - 3.5f, 7.0f, 7.0f);
    }

    juce::Path curve;
    for (int i = 0; i <= 200; ++i)
    {
        const float px = (float)i / 200.0f * w;
        const float db = FilterModule::responseDb(type, slope, freq, q, gain,
                                                  freqForX(px, w), sr);
        const float py = juce::jlimit(-10.0f, h + 10.0f,
                          yForDb(juce::jlimit(kDbBot - 8.0f, kDbTop + 8.0f, db), h));
        if (i == 0) curve.startNewSubPath(px, py);
        else        curve.lineTo(px, py);
    }
    // response curve — soft glow underneath, crisp bright line on top
    g.setColour(aurora::curveGlow.withAlpha(0.40f));
    g.strokePath(curve, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
    g.setColour(aurora::curveLine);
    g.strokePath(curve, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    // Filter node — round draggable handle on a faint cutoff line
    {
        const float nx = xForFreq(freq, w);
        const float ny = yForDb(juce::jlimit(kDbBot, kDbTop,
                          nodeDbFor((int)type, q, gain)), h);
        g.setColour(aurora::curveGlow.withAlpha(0.35f));
        g.drawVerticalLine((int)nx, 4.0f, h - 18.0f);
        g.setColour(aurora::curveGlow.withAlpha(0.12f));
        g.fillEllipse(nx - 11.0f, ny - 11.0f, 22.0f, 22.0f);
        g.setColour(aurora::curveGlow);
        g.drawEllipse(nx - 11.0f, ny - 11.0f, 22.0f, 22.0f, 1.5f);
        g.setColour(juce::Colour(0xFFFFE6B0));
        g.fillEllipse(nx - 3.4f, ny - 3.4f, 6.8f, 6.8f);
    }

    // axis labels — dim amber mono
    g.setColour(aurora::graphLine.withAlpha(0.42f));
    g.setFont(aurora::mono(10.0f));
    for (auto [f, lbl] : std::initializer_list<std::pair<float, const char*>>{
            { 20.f, "20" }, { 50.f, "50" }, { 100.f, "100" }, { 200.f, "200" },
            { 500.f, "500" }, { 1000.f, "1k" }, { 2000.f, "2k" }, { 5000.f, "5k" },
            { 10000.f, "10k" }, { 20000.f, "20k" } })
    {
        const float lx = juce::jlimit(2.0f, w - 38.0f, xForFreq(f, w) - 18.0f);
        g.drawText(lbl, (int)lx, (int)h - 16, 40, 12, juce::Justification::centred);
    }
    for (float db : { 12.f, 0.f, -12.f, -24.f })
        g.drawText(juce::String((int)db), 6, (int)yForDb(db, h) - 12, 28, 11,
                   juce::Justification::centredLeft);
    g.setColour(aurora::graphLine.withAlpha(0.55f));
    g.setFont(aurora::mono(9.5f));
    g.drawText("dB", 6, 4, 24, 11, juce::Justification::centredLeft);
    g.drawText("Hz", (int)w - 28, (int)h - 16, 22, 12, juce::Justification::centredRight);
}

// ============================================================
//  MeterColumn
// ============================================================

void MeterColumn::setLevels(float l, float r)
{
    dispL = juce::jmax(l, dispL * 0.88f);
    dispR = juce::jmax(r, dispR * 0.88f);
    repaint();
}

void MeterColumn::paint(juce::Graphics& g)
{
    // segmented amber channels in dark inset wells (Meter.jsx)
    const float w = (float)getWidth(), h = (float)getHeight();
    const float labelH = 14.0f;
    const float barW = (w - 10.0f) / 2.0f;
    constexpr int nSeg = 22;
    const float gap  = 2.0f;

    g.setFont(aurora::mono(9.0f));

    for (int ch = 0; ch < 2; ++ch)
    {
        const float bx  = ch == 0 ? 0.0f : barW + 10.0f;
        const float lvl = ch == 0 ? dispL : dispR;

        g.setColour(aurora::inkFaint);
        g.drawText(ch == 0 ? "L" : "R", (int)bx, 0, (int)barW, (int)labelH,
                   juce::Justification::centred);

        const juce::Rectangle<float> well(bx, labelH, barW, h - labelH);
        g.setColour(aurora::graphBg);
        g.fillRoundedRectangle(well, 2.0f);

        const float db   = juce::Decibels::gainToDecibels(lvl, -60.0f);
        const float frac = juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 66.0f);

        const auto inner = well.reduced(2.5f);
        const float segH = (inner.getHeight() - (nSeg - 1) * gap) / (float)nSeg;
        for (int i = 0; i < nSeg; ++i)
        {
            const float segFrac = 1.0f - ((float)i + 0.5f) / (float)nSeg;   // i=0 is top
            const bool  lit     = segFrac <= frac;
            const juce::Colour c = segFrac > 0.9f ? aurora::warnOrange : aurora::graphLine;
            g.setColour(lit ? c.withAlpha(0.55f + segFrac * 0.45f)
                            : aurora::graphLine.withAlpha(0.10f));
            g.fillRect(inner.getX(), inner.getY() + (float)i * (segH + gap),
                       inner.getWidth(), segH);
        }
    }
}

// ============================================================
//  LfoScope
// ============================================================

LfoScope::LfoScope(juce::AudioProcessorValueTreeState& vts, FilterAnalysisSource& src)
    : apvts(vts), analysis(src) {}

void LfoScope::paint(juce::Graphics& g)
{
    const float w = (float)getWidth(), h = (float)getHeight();
    g.setColour(aurora::graphBg);
    g.fillRoundedRectangle(0.0f, 0.0f, w, h, 3.0f);
    g.setColour(aurora::graphLine.withAlpha(0.16f));
    g.drawHorizontalLine((int)(h * 0.5f), 2.0f, w - 2.0f);

    const auto waveId = (lfoIndex == 0) ? ParamID::fltLfoAWave : ParamID::fltLfoBWave;
    const auto* pW = apvts.getRawParameterValue(waveId);
    const int wave = pW ? (int)pW->load() : 0;

    // Two cycles of the selected waveform
    juce::Path p;
    juce::Random seeded(42);   // stable preview for RANDOM
    float held = seeded.nextFloat() * 2.0f - 1.0f;
    for (int i = 0; i <= 140; ++i)
    {
        const float t  = (float)i / 140.0f * 2.0f;
        const float fr = t - std::floor(t);
        float v = 0.0f;
        switch (wave)
        {
            case 0: v = std::sin(juce::MathConstants<float>::twoPi * fr); break;
            case 1: v = 4.0f * std::abs(fr - 0.5f) - 1.0f;                break;
            case 2: v = fr < 0.5f ? 1.0f : -1.0f;                          break;
            case 3:
                if (i > 0 && fr < (float)(i - 1) / 140.0f * 2.0f
                              - std::floor((float)(i - 1) / 140.0f * 2.0f))
                    held = seeded.nextFloat() * 2.0f - 1.0f;
                v = held;
                break;
        }
        const float px = (float)i / 140.0f * w;
        const float py = h * 0.5f - v * h * 0.38f;
        if (i == 0) p.startNewSubPath(px, py);
        else        p.lineTo(px, py);
    }
    g.setColour(aurora::graphLine);
    g.strokePath(p, juce::PathStrokeType(1.6f));

    // Phase cursor
    const float ph = analysis.lfoPhase.load();
    g.setColour(aurora::led.withAlpha(0.8f));
    g.drawVerticalLine((int)(ph * 0.5f * w), 2.0f, h - 2.0f);
}

// ============================================================
//  EnvScope
// ============================================================

void EnvScope::push(float v)
{
    hist[head] = v;
    head = (head + 1) % kHist;
    repaint();
}

void EnvScope::paint(juce::Graphics& g)
{
    const float w = (float)getWidth(), h = (float)getHeight();
    g.setColour(aurora::graphBg);
    g.fillRoundedRectangle(0.0f, 0.0f, w, h, 3.0f);

    juce::Path line;
    for (int i = 0; i < kHist; ++i)
    {
        const float v  = hist[(head + i) % kHist];
        const float px = (float)i / (kHist - 1) * w;
        const float py = h - juce::jlimit(0.0f, 1.0f, v) * (h - 4.0f);
        if (i == 0) line.startNewSubPath(px, py);
        else        line.lineTo(px, py);
    }

    juce::Path fill(line);
    fill.lineTo(w, h);
    fill.lineTo(0.0f, h);
    fill.closeSubPath();
    g.setColour(aurora::graphLine.withAlpha(0.30f));
    g.fillPath(fill);
    g.setColour(aurora::graphLine);
    g.strokePath(line, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved));
}

// ============================================================
//  AuroraFilterEditor
// ============================================================

AuroraFilterEditor::AuroraFilterEditor(juce::AudioProcessor& proc,
                                       juce::AudioProcessorValueTreeState& vts,
                                       FilterAnalysisSource& src)
    : AudioProcessorEditor(&proc), apvts(vts), analysis(src),
      display(vts, src), lfoScope(vts, src)
{
    setLookAndFeel(&lnf);
    addAndMakeVisible(content);
    content.setBounds(0, 0, kW, kH);

    content.addAndMakeVisible(display);
    content.addAndMakeVisible(meters);
    content.addAndMakeVisible(lfoScope);
    content.addAndMakeVisible(envScope);

    // ---- Filter type — vertical LED option list ----
    for (int i = 0; i < 6; ++i)
    {
        typeBtns[i].setButtonText(kTypeNames[i]);
        typeBtns[i].getProperties().set("optionRow", true);
        typeBtns[i].onClick = [this, i]() { setParam(ParamID::filterType, (float)kTypeMap[i]); };
        content.addAndMakeVisible(typeBtns[i]);
    }

    // ---- Knobs ----
    makeKnob(freqKnob,   ParamID::filterFreq,   content);
    makeKnob(resKnob,    ParamID::filterQ,      content);
    makeKnob(driveKnob,  ParamID::filterDrive,  content);
    makeKnob(mixKnob,    ParamID::filterMix,    content);
    makeKnob(outKnob,    ParamID::filterOutput, content);
    makeKnob(gainKnob,   ParamID::filterGain,   content);
    makeKnob(amountKnob, ParamID::fltModAmount, content);
    makeKnob(sensKnob,   ParamID::fltEnvSens,   content);
    rebindEnvKnobs();   // binds atk/rel to ms or note division depending on sync

    // Hero face knobs: value prints big on the disc (number only), the
    // unit lives in the small tracked label beneath it (Knob.jsx `face`).
    auto setFace = [](AuroraKnob& k, const juce::String& faceLabel) {
        k.slider.getProperties().set("face", true);
        k.slider.getProperties().set("faceLabel", faceLabel);
        k.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    };
    setFace(freqKnob,  juce::String::fromUTF8("FREQ \xc2\xb7 HZ"));
    setFace(resKnob,   juce::String::fromUTF8("RES \xc2\xb7 Q"));
    setFace(driveKnob, juce::String::fromUTF8("DRIVE \xc2\xb7 DB"));
    setFace(mixKnob,   juce::String::fromUTF8("MIX \xc2\xb7 %"));
    setFace(outKnob,   juce::String::fromUTF8("OUT \xc2\xb7 DB"));

    freqKnob  .slider.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v)); };
    resKnob   .slider.textFromValueFunction = [](double v) { return juce::String(v, 1); };
    driveKnob .slider.textFromValueFunction = [](double v) { return juce::String(v, 1); };
    mixKnob   .slider.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v * 100.0)); };
    outKnob   .slider.textFromValueFunction = [](double v) { return juce::String(v, 1); };
    gainKnob  .slider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    amountKnob.slider.textFromValueFunction = [](double v) {
        return (v >= 0.0 ? "+" : "") + juce::String(v * 100.0, 1) + " %"; };
    sensKnob  .slider.textFromValueFunction = [](double v) { return juce::String(v, 1); };
    for (auto* k : { &freqKnob, &resKnob, &driveKnob, &mixKnob, &outKnob,
                     &gainKnob, &amountKnob, &sensKnob })
        k->slider.updateText();

    // Right-click on RATE / ATTACK / RELEASE → FREE / TEMPO SYNC menu
    rateKnob.slider.addMouseListener(&rateRC, false);
    atkKnob .slider.addMouseListener(&atkRC,  false);
    relKnob .slider.addMouseListener(&relRC,  false);
    rateRC.fn = [this]() {
        showSyncMenu(boundLfo == 1 ? ParamID::fltLfoBSync : ParamID::fltLfoASync); };
    atkRC.fn = [this]() { showSyncMenu(ParamID::fltEnvAtkSync); };
    relRC.fn = [this]() { showSyncMenu(ParamID::fltEnvRelSync); };

    // ---- Slope + mode — LED option rows ----
    slope12.setButtonText("12 dB");
    slope24.setButtonText("24 dB");
    slope48.setButtonText("48 dB");
    slope12.onClick = [this]() { setParam(ParamID::filterSlope, 0.0f); };
    slope24.onClick = [this]() { setParam(ParamID::filterSlope, 1.0f); };
    slope48.onClick = [this]() { setParam(ParamID::filterSlope, 2.0f); };
    modeAnalog.onClick = [this]() { setParam(ParamID::filterMode, 0.0f); };
    modeClean .onClick = [this]() { setParam(ParamID::filterMode, 1.0f); };
    for (auto* b : { &slope12, &slope24, &slope48, &modeAnalog, &modeClean })
    {
        b->getProperties().set("optionRow", true);
        content.addAndMakeVisible(*b);
    }

    // ---- Header: presets, A/B, power ----
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setFont(aurora::mono(13.0f));
    presetLabel.setColour(juce::Label::textColourId, aurora::graphLine);
    presetLabel.setColour(juce::Label::backgroundColourId, aurora::graphBg);
    presetLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    updatePresetLabel();
    content.addAndMakeVisible(presetLabel);
    presetLabel.addMouseListener(&presetLabelClick, false);
    presetLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    presetLabelClick.fn = [this]() { browsePresets(); };

    prevPreset.onClick = [this]() { loadPresetAtIdx(currentPresetIdx - 1); };
    nextPreset.onClick = [this]() { loadPresetAtIdx(currentPresetIdx + 1); };
    content.addAndMakeVisible(prevPreset);
    content.addAndMakeVisible(nextPreset);

    content.addAndMakeVisible(saveBtn);
    saveBtn.onClick = [this]() {
        if (saveOverlay != nullptr) return;
        auto* ov = new SavePresetOverlay(currentPresetName);
        ov->setBounds(0, 0, kW, kH);
        juce::Component::SafePointer<AuroraFilterEditor> safe(this);
        ov->onSave    = [this](const juce::String& n) { doSavePreset(n); };
        ov->onDismiss = [safe]() {
            juce::MessageManager::callAsync([safe]() {
                if (safe) safe->saveOverlay.reset();
            });
        };
        saveOverlay.reset(ov);
        content.addAndMakeVisible(ov);
    };

    slotA = apvts.copyState().createCopy();
    slotB = apvts.copyState().createCopy();
    btnA.onClick = [this]() { switchAB(0); };
    btnB.onClick = [this]() { switchAB(1); };
    content.addAndMakeVisible(btnA);
    content.addAndMakeVisible(btnB);

    powerBtn.setClickingTogglesState(true);
    powerBtn.onClick = [this]() {
        if (auto* p = apvts.getParameter(ParamID::filterBypass))
            p->setValueNotifyingHost(powerBtn.getToggleState() ? 0.0f : 1.0f);
    };
    content.addAndMakeVisible(powerBtn);

    // ---- Modulation routing dropdowns ----
    srcBox.addItemList({ "OFF", "LFO A", "LFO B", "ENV" }, 1);
    dstBox.addItemList({ "FREQUENCY", "RESONANCE", "DRIVE" }, 1);
    content.addAndMakeVisible(srcBox);
    content.addAndMakeVisible(dstBox);
    srcAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, ParamID::fltModSource, srcBox);
    dstAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, ParamID::fltModDest, dstBox);

    // ---- Oversampling selector (header) ----
    osBox.addItemList({ "1x", "4x", "8x" }, 1);
    content.addAndMakeVisible(osBox);
    osAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, ParamID::filterOversamp, osBox);

    // ---- LFO engine ----
    const char* waveNames[4] = { "SINE", "TRI", "SQR", "RND" };
    for (int i = 0; i < 4; ++i)
    {
        waveBtns[i].setButtonText(waveNames[i]);
        waveBtns[i].onClick = [this, i]() {
            setParam(boundLfo == 1 ? ParamID::fltLfoBWave : ParamID::fltLfoAWave, (float)i); };
        content.addAndMakeVisible(waveBtns[i]);
    }
    rebindLfoKnobs(0);

    // ---- Static panel art (Filter 76 faceplate) ----
    content.onPaint = [this](juce::Graphics& g) {
        g.fillAll(aurora::baseSurface);

        // barely-there top sheen on the plastic
        {
            juce::ColourGradient sheen(juce::Colour(0x80FFFDF7), 0.0f, 0.0f,
                                       juce::Colour(0x00FFFDF7), 0.0f, 120.0f, false);
            g.setGradientFill(sheen);
            g.fillRect(0, 0, kW, 120);
        }

        // ---- header: type-only wordmark ----
        g.setColour(aurora::textPrimary);
        g.setFont(aurora::heading(26.0f).withExtraKerningFactor(0.22f));
        g.drawText("ALTERED AUDIO", 24, 22, 470, 28, juce::Justification::centredLeft);
        g.setColour(aurora::amberLo);
        g.setFont(aurora::heading(12.0f).withExtraKerningFactor(0.14f));
        g.drawText("FILTER 76", 26, 54, 300, 14, juce::Justification::centredLeft);

        const auto label = [&g](int lx, int ly, int lw, const char* t,
                                juce::Justification j = juce::Justification::centredLeft) {
            g.setColour(aurora::textSecond);
            g.setFont(aurora::mono(9.0f).withExtraKerningFactor(0.08f));
            g.drawText(t, lx, ly, lw, 11, j);
        };

        // header captions + readouts
        label(prevPreset.getX(), 16, 300, "PRESET", juce::Justification::centredLeft);

        auto readout = [&g, &label](int rx, const char* name, const juce::String& val, int boxW) {
            label(rx, 24, boxW + 20, name, juce::Justification::centred);
            const juce::Rectangle<float> well((float)rx, 40.0f, (float)boxW, 22.0f);
            g.setColour(aurora::graphBg);
            g.fillRoundedRectangle(well, 2.0f);
            g.setColour(aurora::graphLine);
            g.setFont(aurora::mono(11.0f));
            g.drawText(val, well.toNearestInt(), juce::Justification::centred);
        };
        const auto* pMix = apvts.getRawParameterValue(ParamID::filterMix);
        label(1036, 24, 80, "OVERSAMPLING", juce::Justification::centred);
        readout(1128, "MIX",
                juce::String(juce::roundToInt((pMix ? pMix->load() : 1.0f) * 100.0f)) + " %", 52);

        // ---- module panels: flat plastic, hairline, top highlight ----
        auto panel = [&g](juce::Rectangle<int> r, const char* titleTxt, int ledState) {
            const auto rf = r.toFloat();
            g.setColour(aurora::baseSurface.brighter(0.015f));
            g.fillRoundedRectangle(rf, 6.0f);
            g.setColour(juce::Colour(0x66FFFCF4));
            g.fillRect(rf.getX() + 2.0f, rf.getY() + 1.0f, rf.getWidth() - 4.0f, 1.5f);
            g.setColour(aurora::border);
            g.drawRoundedRectangle(rf, 6.0f, 1.0f);

            if (titleTxt != nullptr)
            {
                int tx = r.getX() + 18;
                if (ledState >= 0)
                {
                    const float ly = (float)r.getY() + 14.0f;
                    if (ledState == 1)
                    {
                        g.setColour(aurora::led.withAlpha(0.35f));
                        g.fillEllipse((float)tx - 2.0f, ly - 2.0f, 11.0f, 11.0f);
                        g.setColour(aurora::led);
                    }
                    else
                        g.setColour(aurora::inkFaint);
                    g.fillEllipse((float)tx, ly, 7.0f, 7.0f);
                    tx += 15;
                }
                g.setColour(aurora::textSecond);
                g.setFont(aurora::mono(11.0f).withExtraKerningFactor(0.14f));
                g.drawText(titleTxt, tx, r.getY() + 10, r.getWidth() - 36, 14,
                           juce::Justification::centredLeft);
            }
        };

        const auto* pSrc = apvts.getRawParameterValue(ParamID::fltModSource);
        const int   src  = pSrc ? (int)pSrc->load() : 0;

        panel(kMeterPanel, "OUTPUT",      -1);
        panel(kTypePanel,  "FILTER TYPE", -1);
        panel(kKnobPanel,  nullptr,       -1);
        panel(kSlopePanel, "SLOPE",       -1);
        panel(kModPanel,   "MODULATION",        src != 0 ? 1 : 0);
        panel(kLfoPanel,   "LFO ENGINE",        -1);
        panel(kEnvPanel,   "ENVELOPE FOLLOWER", src == 3 ? 1 : 0);

        // compact-knob titles (hero knobs carry their label on the face)
        auto title = [&g](juce::Component& c, const char* t) {
            g.setColour(aurora::textSecond);
            g.setFont(aurora::mono(10.0f).withExtraKerningFactor(0.08f));
            g.drawText(t, c.getX(), c.getY() - 14, c.getWidth(), 12, juce::Justification::centred);
        };
        title(gainKnob.slider,   "GAIN");
        title(amountKnob.slider, "AMOUNT");
        title(rateKnob.slider,   "RATE");
        title(depthKnob.slider,  "DEPTH");
        title(phaseKnob.slider,  "PHASE");
        title(atkKnob.slider,    "ATTACK");
        title(relKnob.slider,    "RELEASE");
        title(sensKnob.slider,   "SENS");

        // section sub-labels
        label(kModPanel.getX() + 18, kModPanel.getY() + 38,  200, "SOURCE");
        label(kModPanel.getX() + 18, kModPanel.getY() + 102, 200, "DESTINATION");
        label(kSlopePanel.getX() + 18, kSlopePanel.getY() + 112, 100, "MODE");

        // ---- footer: warm strip with SYSTEM LED + inline stats ----
        {
            const auto rf = kFooter.toFloat();
            g.setColour(aurora::warmBeige);
            g.fillRoundedRectangle(rf, 4.0f);
            g.setColour(aurora::border);
            g.drawRoundedRectangle(rf, 4.0f, 1.0f);

            const int cy = kFooter.getCentreY();
            g.setColour(aurora::led.withAlpha(0.35f));
            g.fillEllipse((float)kFooter.getX() + 14.0f, (float)cy - 6.0f, 12.0f, 12.0f);
            g.setColour(aurora::led);
            g.fillEllipse((float)kFooter.getX() + 16.0f, (float)cy - 4.0f, 8.0f, 8.0f);
            g.setColour(aurora::textSecond);
            g.setFont(aurora::mono(9.0f).withExtraKerningFactor(0.08f));
            g.drawText("SYSTEM", kFooter.getX() + 32, cy - 6, 60, 12,
                       juce::Justification::centredLeft);

            auto stat = [&g, cy](int x, const char* name, const juce::String& val) {
                g.setColour(aurora::textSecond);
                g.setFont(aurora::mono(9.0f).withExtraKerningFactor(0.08f));
                g.drawText(name, x, cy - 6, 110, 12, juce::Justification::centredLeft);
                g.setColour(aurora::textPrimary);
                g.setFont(aurora::mono(12.0f));
                g.drawText(val, x + 92, cy - 7, 100, 14, juce::Justification::centredLeft);
                return x;
            };
            const float sr  = juce::jmax(1.0f, analysis.sampleRate.load());
            const auto* pOs = apvts.getRawParameterValue(ParamID::filterOversamp);
            static const char* osNames[3] = { "1x", "4x", "8x" };
            const int osI = pOs ? juce::jlimit(0, 2, (int)pOs->load()) : 0;
            const float latMs = (float)getAudioProcessor()->getLatencySamples() / sr * 1000.0f;

            stat(130, "SAMPLE RATE",  juce::String(sr / 1000.0f, 1) + " kHz");
            stat(360, "OVERSAMPLING", osNames[osI]);
            stat(590, "LATENCY",      juce::String(latMs, 2) + " ms");
            stat(800, "CPU",          juce::String(analysis.cpuPct.load(), 1) + " %");
            stat(990, "SIGNAL PATH",  "STEREO");

            g.setColour(aurora::textSecond);
            g.setFont(aurora::mono(10.0f));
            g.drawText("v" + juce::String(JucePlugin_VersionString),
                       kFooter.getRight() - 110, cy - 7, 96, 14,
                       juce::Justification::centredRight);
        }
    };

    setResizable(true, true);
    setResizeLimits(1100, 700, 2800, 1800);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio((double)kW / (double)kH);
    setSize(kW, kH);

    startTimerHz(30);
}

AuroraFilterEditor::~AuroraFilterEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void AuroraFilterEditor::makeKnob(AuroraKnob& k, const juce::String& paramId,
                                  juce::Component& parent)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 86, 17);
    parent.addAndMakeVisible(k.slider);
    k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramId, k.slider);
}

void AuroraFilterEditor::setParam(const juce::String& id, float realValue)
{
    if (auto* p = apvts.getParameter(id))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(realValue));
}

void AuroraFilterEditor::updatePresetLabel()
{
    presetLabel.setText(
        juce::String(currentPresetIdx + 1).paddedLeft('0', 3) + "  " + currentPresetName,
        juce::dontSendNotification);
}

void AuroraFilterEditor::applyPreset(int index)
{
    currentPresetIdx  = juce::jlimit(0, (int)presets().size() - 1, index);
    currentIsUser     = false;
    currentPresetName = presets()[(size_t)currentPresetIdx].name;
    for (const auto& [id, v] : presets()[(size_t)currentPresetIdx].values)
        setParam(id, v);
    updatePresetLabel();
}

void AuroraFilterEditor::loadPresetAtIdx(int idx)
{
    const auto& fp = presets();
    const auto userList = PresetManager::userNames();
    const int total = (int)fp.size() + userList.size();
    if (total == 0) return;
    idx = ((idx % total) + total) % total;

    if (idx < (int)fp.size())
    {
        applyPreset(idx);
    }
    else
    {
        const int ui  = idx - (int)fp.size();
        currentPresetIdx  = idx;
        currentIsUser     = true;
        currentPresetName = userList[ui];
        const auto state  = PresetManager::load(currentPresetName);
        if (state.isValid()) apvts.replaceState(state);
        updatePresetLabel();
    }
}

void AuroraFilterEditor::browsePresets()
{
    const auto& fp = presets();
    const auto userList = PresetManager::userNames();

    juce::PopupMenu menu;
    menu.setLookAndFeel(&lnf);

    menu.addSectionHeader("FACTORY");
    for (int i = 0; i < (int)fp.size(); ++i)
    {
        const auto lbl = juce::String(i + 1).paddedLeft('0', 3) + "  " + fp[i].name;
        menu.addItem(i + 1, lbl, true, !currentIsUser && currentPresetIdx == i);
    }

    if (!userList.isEmpty())
    {
        menu.addSeparator();
        menu.addSectionHeader("USER");
        for (int i = 0; i < userList.size(); ++i)
        {
            const int absIdx = (int)fp.size() + i;
            const auto lbl = juce::String(absIdx + 1).paddedLeft('0', 3) + "  " + userList[i];
            menu.addItem(200 + i, lbl, true,
                         currentIsUser && currentPresetName == userList[i]);
        }
    }

    const auto mouse = juce::Desktop::getMousePosition();
    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetScreenArea({ mouse.x, mouse.y, 1, 1 }),
        [this](int result) {
            if (result == 0) return;
            if (result < 200)
                loadPresetAtIdx(result - 1);
            else
            {
                const auto ul = PresetManager::userNames();
                const int ui = result - 200;
                if (ui >= 0 && ui < ul.size())
                    loadPresetAtIdx((int)presets().size() + ui);
            }
        });
}

void AuroraFilterEditor::doSavePreset(const juce::String& name)
{
    if (!PresetManager::save(name, apvts)) return;
    currentIsUser     = true;
    currentPresetName = name;
    const auto ul = PresetManager::userNames();
    const int  ui = ul.indexOf(name);
    currentPresetIdx = (int)presets().size() + juce::jmax(0, ui);
    updatePresetLabel();
}

void AuroraFilterEditor::switchAB(int slot)
{
    if (slot == activeSlot) return;
    // Store current state into the slot we're leaving, then load the target
    (activeSlot == 0 ? slotA : slotB) = apvts.copyState().createCopy();
    const auto& target = (slot == 0 ? slotA : slotB);
    if (target.isValid())
        apvts.replaceState(target.createCopy());
    activeSlot = slot;
}

void AuroraFilterEditor::rebindLfoKnobs(int lfoIndex)
{
    boundLfo = lfoIndex;
    const bool b = (lfoIndex == 1);

    const auto* pSync = apvts.getRawParameterValue(b ? ParamID::fltLfoBSync : ParamID::fltLfoASync);
    const bool sync = pSync && *pSync > 0.5f;
    boundLfoSync = sync ? 1 : 0;

    rateKnob.attach.reset();  depthKnob.attach.reset();  phaseKnob.attach.reset();
    if (sync)
    {
        makeKnob(rateKnob, b ? ParamID::fltLfoBDiv : ParamID::fltLfoADiv, content);
        rateKnob.slider.textFromValueFunction = [](double v) {
            return aurora::divNames()[juce::jlimit(0, 13, (int)std::round(v))]; };
    }
    else
    {
        makeKnob(rateKnob, b ? ParamID::fltLfoBRate : ParamID::fltLfoARate, content);
        rateKnob.slider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " Hz"; };
    }
    makeKnob(depthKnob, b ? ParamID::fltLfoBDepth : ParamID::fltLfoADepth, content);
    makeKnob(phaseKnob, b ? ParamID::fltLfoBPhase : ParamID::fltLfoAPhase, content);
    depthKnob.slider.textFromValueFunction = [](double v) { return juce::String(v * 100.0, 1) + " %"; };
    phaseKnob.slider.textFromValueFunction = [](double v) { return juce::String(v, 1) + juce::String::fromUTF8("\xc2\xb0"); };
    for (auto* k : { &rateKnob, &depthKnob, &phaseKnob })
        k->slider.updateText();
    lfoScope.setLfoIndex(lfoIndex);
    resized();   // re-place the freshly created sliders
}

void AuroraFilterEditor::rebindEnvKnobs()
{
    const auto* pAtkSync = apvts.getRawParameterValue(ParamID::fltEnvAtkSync);
    const auto* pRelSync = apvts.getRawParameterValue(ParamID::fltEnvRelSync);
    const bool atkSync = pAtkSync && *pAtkSync > 0.5f;
    const bool relSync = pRelSync && *pRelSync > 0.5f;
    boundAtkSync = atkSync ? 1 : 0;
    boundRelSync = relSync ? 1 : 0;

    auto divText = [](double v) {
        return aurora::divNames()[juce::jlimit(0, 13, (int)std::round(v))]; };

    atkKnob.attach.reset();
    makeKnob(atkKnob, atkSync ? ParamID::fltEnvAtkDiv : ParamID::fltEnvAttack, content);
    atkKnob.slider.textFromValueFunction = atkSync
        ? std::function<juce::String(double)>(divText)
        : [](double v) { return juce::String(v, 1) + " ms"; };

    relKnob.attach.reset();
    makeKnob(relKnob, relSync ? ParamID::fltEnvRelDiv : ParamID::fltEnvRelease, content);
    relKnob.slider.textFromValueFunction = relSync
        ? std::function<juce::String(double)>(divText)
        : [](double v) { return juce::String(v, 1) + " ms"; };

    atkKnob.slider.updateText();
    relKnob.slider.updateText();
    resized();
}

void AuroraFilterEditor::showSyncMenu(const juce::String& syncParamId)
{
    const auto* pSync = apvts.getRawParameterValue(syncParamId);
    const bool on = pSync && *pSync > 0.5f;

    juce::PopupMenu menu;
    menu.setLookAndFeel(&lnf);   // match the plugin palette (cream / amber highlight)
    menu.addItem(1, "FREE",       true, !on);
    menu.addItem(2, "TEMPO SYNC", true, on);

    const auto mouse = juce::Desktop::getMousePosition();
    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetScreenArea({ mouse.x, mouse.y, 1, 1 }),
        [this, syncParamId](int result) {
            if (result != 0)
                setParam(syncParamId, result == 2 ? 1.0f : 0.0f);
        });
}

void AuroraFilterEditor::updateButtonStates()
{
    auto raw = [this](const char* id) {
        const auto* p = apvts.getRawParameterValue(id);
        return p ? (int)p->load() : 0;
    };
    auto color = [](juce::TextButton& b, bool sel) {
        b.setColour(juce::TextButton::buttonColourId,
                    sel ? aurora::amber : aurora::baseSurface);
    };

    const int type = raw(ParamID::filterType);
    for (int i = 0; i < 6; ++i) color(typeBtns[i], kTypeMap[i] == type);

    const int slope = raw(ParamID::filterSlope);
    color(slope12, slope == 0);
    color(slope24, slope == 1);
    color(slope48, slope == 2);

    const int mode = raw(ParamID::filterMode);
    color(modeAnalog, mode == 0);
    color(modeClean,  mode == 1);

    const int wave = raw(boundLfo == 1 ? ParamID::fltLfoBWave : ParamID::fltLfoAWave);
    for (int i = 0; i < 4; ++i) color(waveBtns[i], wave == i);

    color(btnA, activeSlot == 0);
    color(btnB, activeSlot == 1);

    const bool on = raw(ParamID::filterBypass) == 0;
    if (powerBtn.getToggleState() != on)
        powerBtn.setToggleState(on, juce::dontSendNotification);

    // ---- enablement: a knob that can't affect the sound is disabled ----
    const int src = raw(ParamID::fltModSource);
    const bool lfoOn = (src == 1 || src == 2);

    gainKnob  .slider.setEnabled(type == 5 || type == 6 || type == 7);  // PEAK / shelves
    driveKnob .slider.setEnabled(raw(ParamID::filterMode) == 0);        // CLEAN bypasses drive
    amountKnob.slider.setEnabled(src != 0);
    rateKnob  .slider.setEnabled(lfoOn);
    depthKnob .slider.setEnabled(lfoOn);
    phaseKnob .slider.setEnabled(lfoOn);
    atkKnob   .slider.setEnabled(src == 3);
    relKnob   .slider.setEnabled(src == 3);
    sensKnob  .slider.setEnabled(src == 3);
}

void AuroraFilterEditor::timerCallback()
{
    display.refresh();
    meters.setLevels(analysis.peakL.exchange(0.0f), analysis.peakR.exchange(0.0f));
    envScope.push(analysis.envValue.load());
    lfoScope.repaint();
    updateButtonStates();

    // Follow the modulation source (and sync state) with the LFO ENGINE bindings
    const auto* pSrc = apvts.getRawParameterValue(ParamID::fltModSource);
    const int wantLfo = (pSrc && (int)pSrc->load() == 2) ? 1 : 0;
    const auto* pLS = apvts.getRawParameterValue(
        wantLfo == 1 ? ParamID::fltLfoBSync : ParamID::fltLfoASync);
    const int wantLfoSync = (pLS && *pLS > 0.5f) ? 1 : 0;
    if (wantLfo != boundLfo || wantLfoSync != boundLfoSync)
        rebindLfoKnobs(wantLfo);

    // Same for envelope attack/release sync
    const auto* pAS = apvts.getRawParameterValue(ParamID::fltEnvAtkSync);
    const auto* pRS = apvts.getRawParameterValue(ParamID::fltEnvRelSync);
    const int wantAtkSync = (pAS && *pAS > 0.5f) ? 1 : 0;
    const int wantRelSync = (pRS && *pRS > 0.5f) ? 1 : 0;
    if (wantAtkSync != boundAtkSync || wantRelSync != boundRelSync)
        rebindEnvKnobs();

    // Refresh dynamic painted regions (footer stats, panel LEDs, MIX readout)
    content.repaint(0, kFooterY - 4, kW, kH - kFooterY + 4);
    content.repaint(kModPanel.getX(), kModPanel.getY(), 240, 30);
    content.repaint(kEnvPanel.getX(), kEnvPanel.getY(), 240, 30);
    content.repaint(1040, 16, 160, 52);
}

void AuroraFilterEditor::paint(juce::Graphics& g)
{
    g.fillAll(aurora::graphBg);   // letterbox color behind scaled content
}

void AuroraFilterEditor::resized()
{
    const float scale = juce::jmin((float)getWidth() / (float)kW,
                                   (float)getHeight() / (float)kH);
    const float ox = ((float)getWidth()  - kW * scale) * 0.5f;
    const float oy = ((float)getHeight() - kH * scale) * 0.5f;
    content.setTransform(juce::AffineTransform::scale(scale).translated(ox, oy));

    // ---- Header ----
    prevPreset .setBounds(540,  36, 26, 26);
    presetLabel.setBounds(572,  35, 300, 28);
    nextPreset .setBounds(878,  36, 26, 26);
    saveBtn    .setBounds(910,  36, 52, 26);
    osBox   .setBounds(1044, 40, 64, 22);
    btnA    .setBounds(1216, 38, 38, 26);
    btnB    .setBounds(1256, 38, 38, 26);
    powerBtn.setBounds(1318, 18, 48, 62);

    // ---- Display + OUTPUT meter panel ----
    display.setBounds(16, kDisplayY, kKnobPanel.getRight() - 16 - 14, kDisplayH);
    meters .setBounds(kMeterPanel.getX() + (kMeterPanel.getWidth() - 66) / 2,
                      kMeterPanel.getY() + 38, 66, kMeterPanel.getHeight() - 58);

    // ---- FILTER TYPE option list ----
    for (int i = 0; i < 6; ++i)
        typeBtns[i].setBounds(kTypePanel.getX() + 10, kTypePanel.getY() + 34 + i * 27,
                              kTypePanel.getWidth() - 20, 24);

    // ---- SLOPE + MODE option lists ----
    {
        const int sx = kSlopePanel.getX() + 10, sw = kSlopePanel.getWidth() - 20;
        slope12.setBounds(sx, kSlopePanel.getY() + 30, sw, 23);
        slope24.setBounds(sx, kSlopePanel.getY() + 55, sw, 23);
        slope48.setBounds(sx, kSlopePanel.getY() + 80, sw, 23);
        modeAnalog.setBounds(sx, kSlopePanel.getY() + 128, sw, 23);
        modeClean .setBounds(sx, kSlopePanel.getY() + 153, sw, 23);
    }

    // ---- hero face knobs + GAIN at the right of the knob panel ----
    {
        const int slotW = 190;
        const int knobS = 150;
        const int ky    = kFilterY + (kFilterH - knobS) / 2;
        AuroraKnob* heroes[5] = { &freqKnob, &resKnob, &driveKnob, &mixKnob, &outKnob };
        for (int i = 0; i < 5; ++i)
            heroes[i]->slider.setBounds(kKnobPanel.getX() + i * slotW + (slotW - knobS) / 2,
                                        ky, knobS, knobS);
        gainKnob.slider.setBounds(kKnobPanel.getX() + 5 * slotW + 9, kFilterY + 62, 72, 60 + 17);
    }

    // ---- MODULATION panel ----
    {
        const int px = kModPanel.getX(), py = kModPanel.getY();
        srcBox.setBounds(px + 18, py + 52,  262, 30);
        dstBox.setBounds(px + 18, py + 116, 262, 30);
        amountKnob.slider.setBounds(px + 320, py + 56, 94, 70 + 17);
    }

    // ---- LFO ENGINE panel ----
    {
        const int px = kLfoPanel.getX(), py = kLfoPanel.getY();
        for (int i = 0; i < 4; ++i)
            waveBtns[i].setBounds(px + 16 + i * 56, py + 34, 50, 24);
        lfoScope.setBounds(px + 16, py + 68, 208, 100);
        rateKnob .slider.setBounds(px + 238, py + 64, 72, 60 + 17);
        depthKnob.slider.setBounds(px + 314, py + 64, 72, 60 + 17);
        phaseKnob.slider.setBounds(px + 390, py + 64, 72, 60 + 17);
    }

    // ---- ENVELOPE FOLLOWER panel ----
    {
        const int px = kEnvPanel.getX(), py = kEnvPanel.getY();
        envScope.setBounds(px + 16, py + 44, 180, 124);
        atkKnob .slider.setBounds(px + 208, py + 64, 72, 60 + 17);
        relKnob .slider.setBounds(px + 284, py + 64, 72, 60 + 17);
        sensKnob.slider.setBounds(px + 360, py + 64, 72, 60 + 17);
    }
}
