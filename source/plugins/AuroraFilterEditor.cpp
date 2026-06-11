#include "AuroraFilterEditor.h"

// ============================================================
//  Layout constants (1400 x 900 design space)
// ============================================================
namespace
{
    constexpr int kHeaderH   = 64;
    constexpr int kDisplayY  = 76,  kDisplayH = 360;            // 40% of interface
    constexpr int kFilterY   = 448, kFilterH  = 192;
    constexpr int kBottomY   = 652, kBottomH  = 178;
    constexpr int kFooterY   = 842;

    const juce::Rectangle<int> kModPanel  { 16,  kBottomY, 430, kBottomH };
    const juce::Rectangle<int> kLfoPanel  { 458, kBottomY, 470, kBottomH };
    const juce::Rectangle<int> kEnvPanel  { 944, kBottomY, 440, kBottomH };

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
    setColour(juce::Slider::textBoxTextColourId,       aurora::textPrimary);
    setColour(juce::Slider::textBoxBackgroundColourId, aurora::baseSurface);
    setColour(juce::Slider::textBoxOutlineColourId,    aurora::border);
    setColour(juce::Label::textColourId,               aurora::textPrimary);
    setColour(juce::TextButton::textColourOffId,       aurora::textPrimary);
    setColour(juce::TextButton::textColourOnId,        aurora::textPrimary);
}

void AuroraLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float sliderPos, float startAngle, float endAngle,
                                         juce::Slider&)
{
    const auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(3.0f);
    const float size  = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const auto  area  = bounds.withSizeKeepingCentre(size, size);
    const float cx    = area.getCentreX(), cy = area.getCentreY();
    const float rOut  = size * 0.5f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Scale ticks
    g.setColour(aurora::textSecond.withAlpha(0.6f));
    for (int t = 0; t <= 10; ++t)
    {
        const float a  = startAngle + (float)t / 10.0f * (endAngle - startAngle);
        g.drawLine(cx + rOut * 0.98f * std::sin(a), cy - rOut * 0.98f * std::cos(a),
                   cx + rOut * 0.90f * std::sin(a), cy - rOut * 0.90f * std::cos(a), 1.0f);
    }

    // Amber value arc
    {
        juce::Path arc;
        arc.addCentredArc(cx, cy, rOut * 0.84f, rOut * 0.84f, 0.0f, startAngle, angle, true);
        g.setColour(aurora::amber);
        g.strokePath(arc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
    }

    // Cream plastic body, flat — engineered, not glossy
    const float rBody = rOut * 0.72f;
    g.setColour(aurora::warmBeige);
    g.fillEllipse(cx - rBody, cy - rBody, rBody * 2.0f, rBody * 2.0f);
    g.setColour(aurora::border);
    g.drawEllipse(cx - rBody, cy - rBody, rBody * 2.0f, rBody * 2.0f, 1.4f);

    // Black indicator line
    g.setColour(aurora::textPrimary);
    g.drawLine(cx + rBody * 0.25f * std::sin(angle), cy - rBody * 0.25f * std::cos(angle),
               cx + rBody * 0.95f * std::sin(angle), cy - rBody * 0.95f * std::cos(angle), 2.6f);
}

juce::Label* AuroraLookAndFeel::createSliderTextBox(juce::Slider& s)
{
    auto* l = LookAndFeel_V4::createSliderTextBox(s);
    l->setFont(aurora::mono(11.0f));
    l->setJustificationType(juce::Justification::centred);
    return l;
}

void AuroraLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                             const juce::Colour& bg, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(highlighted ? bg.brighter(0.05f) : bg);
    g.fillRoundedRectangle(r, 2.0f);
    g.setColour(aurora::border);
    g.drawRoundedRectangle(r, 2.0f, 1.0f);
}

juce::Font AuroraLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    return aurora::mono(11.0f);
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
    if (analysis.blockReady.load())
    {
        std::memset(fftWork, 0, sizeof(fftWork));
        std::memcpy(fftWork, analysis.fftInput, sizeof(float) * FilterAnalysisSource::kFFTSize);
        analysis.blockReady.store(false);

        window.multiplyWithWindowingTable(fftWork, FilterAnalysisSource::kFFTSize);
        fft.performFrequencyOnlyForwardTransform(fftWork);

        const float sr   = analysis.sampleRate.load();
        const float norm = 2.0f / (float)FilterAnalysisSource::kFFTSize;
        const float w    = (float)juce::jmax(1, getWidth());

        for (int p = 0; p < kSpecPoints; ++p)
        {
            const float fLo = freqForX((float)p       / kSpecPoints * w, w);
            const float fHi = freqForX((float)(p + 1) / kSpecPoints * w, w);
            int bLo = (int)(fLo / (sr * 0.5f) * (FilterAnalysisSource::kFFTSize / 2));
            int bHi = (int)(fHi / (sr * 0.5f) * (FilterAnalysisSource::kFFTSize / 2));
            bLo = juce::jlimit(1, FilterAnalysisSource::kFFTSize / 2 - 1, bLo);
            bHi = juce::jlimit(bLo, FilterAnalysisSource::kFFTSize / 2 - 1, bHi);

            float mag = 0.0f;
            for (int b = bLo; b <= bHi; ++b)
                mag = juce::jmax(mag, fftWork[b]);

            const float db = juce::Decibels::gainToDecibels(mag * norm, -100.0f) + 18.0f;
            specDisp[p] = (db > specDisp[p]) ? db : specDisp[p] - 1.5f;
        }
    }
    else
    {
        for (auto& v : specDisp) v -= 1.5f;
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

    g.setColour(aurora::graphBg);
    g.fillRoundedRectangle(0.0f, 0.0f, w, h, 3.0f);

    // Grid — subtle dark grey
    g.setColour(juce::Colour(0xFF2B2B28));
    for (float f : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f })
        g.drawVerticalLine((int)xForFreq(f, w), 4.0f, h - 18.0f);
    for (float db : { 12.f, 0.f, -12.f, -24.f })
        g.drawHorizontalLine((int)yForDb(db, h), 4.0f, w - 4.0f);
    g.setColour(juce::Colour(0xFF3A3A35));
    g.drawHorizontalLine((int)yForDb(0.0f, h), 4.0f, w - 4.0f);

    // Live spectrum
    {
        juce::Path spec;
        spec.startNewSubPath(0.0f, h);
        for (int p = 0; p < kSpecPoints; ++p)
            spec.lineTo((float)p / (kSpecPoints - 1) * w,
                        juce::jlimit(0.0f, h, yForDb(specDisp[p], h)));
        spec.lineTo(w, h);
        spec.closeSubPath();
        g.setColour(aurora::graphLine.withAlpha(0.22f));
        g.fillPath(spec);
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
    {
        juce::Path fill(curve);
        fill.lineTo(w, h);
        fill.lineTo(0.0f, h);
        fill.closeSubPath();
        g.setColour(aurora::graphLine.withAlpha(0.10f));
        g.fillPath(fill);
    }
    g.setColour(aurora::graphLine);
    g.strokePath(curve, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));

    // Filter node — crosshair at cutoff, same mapping the drag inverts
    {
        const float nx = xForFreq(freq, w);
        const float ny = yForDb(juce::jlimit(kDbBot, kDbTop,
                          nodeDbFor((int)type, q, gain)), h);
        g.setColour(aurora::graphLine);
        g.drawEllipse(nx - 7.0f, ny - 7.0f, 14.0f, 14.0f, 1.6f);
        g.drawLine(nx - 13.0f, ny, nx - 8.0f, ny, 1.2f);
        g.drawLine(nx + 8.0f,  ny, nx + 13.0f, ny, 1.2f);
        g.drawLine(nx, ny - 13.0f, nx, ny - 8.0f, 1.2f);
        g.drawLine(nx, ny + 8.0f,  nx, ny + 13.0f, 1.2f);
    }

    // Frequency labels per spec
    g.setColour(aurora::textSecond);
    g.setFont(aurora::mono(10.0f));
    for (auto [f, lbl] : std::initializer_list<std::pair<float, const char*>>{
            { 20.f, "20Hz" }, { 50.f, "50Hz" }, { 100.f, "100Hz" }, { 200.f, "200Hz" },
            { 500.f, "500Hz" }, { 1000.f, "1k" }, { 2000.f, "2k" }, { 5000.f, "5k" },
            { 10000.f, "10k" }, { 20000.f, "20k" } })
    {
        const float lx = juce::jlimit(2.0f, w - 38.0f, xForFreq(f, w) - 18.0f);
        g.drawText(lbl, (int)lx, (int)h - 16, 40, 12, juce::Justification::centred);
    }
    for (float db : { 12.f, 0.f, -12.f, -24.f })
        g.drawText(juce::String((int)db), 6, (int)yForDb(db, h) - 12, 28, 11,
                   juce::Justification::centredLeft);
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
    const float w = (float)getWidth(), h = (float)getHeight();
    const float barW = (w - 10.0f) / 2.0f;

    for (int ch = 0; ch < 2; ++ch)
    {
        const float bx  = ch == 0 ? 0.0f : barW + 10.0f;
        const float lvl = ch == 0 ? dispL : dispR;

        g.setColour(aurora::graphBg);
        g.fillRoundedRectangle(bx, 0.0f, barW, h, 2.0f);

        const float db   = juce::Decibels::gainToDecibels(lvl, -60.0f);
        const float frac = juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 66.0f);
        if (frac > 0.005f)
        {
            g.setColour(db > -3.0f ? aurora::warnOrange : aurora::graphLine);
            g.fillRoundedRectangle(bx + 1.5f, h * (1.0f - frac), barW - 3.0f, h * frac, 1.5f);
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
    g.setColour(juce::Colour(0xFF2B2B28));
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

    juce::Path p;
    p.startNewSubPath(0.0f, h);
    for (int i = 0; i < kHist; ++i)
    {
        const float v  = hist[(head + i) % kHist];
        const float px = (float)i / (kHist - 1) * w;
        p.lineTo(px, h - juce::jlimit(0.0f, 1.0f, v) * (h - 4.0f));
    }
    p.lineTo(w, h);
    p.closeSubPath();
    g.setColour(aurora::graphLine.withAlpha(0.55f));
    g.fillPath(p);
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

    // ---- Filter type buttons ----
    for (int i = 0; i < 6; ++i)
    {
        typeBtns[i].setButtonText(kTypeNames[i]);
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

    freqKnob.slider.textFromValueFunction = [](double v) {
        return v >= 1000.0 ? juce::String(v / 1000.0, 1) + " kHz"
                           : juce::String(v, 1) + " Hz"; };
    resKnob   .slider.textFromValueFunction = [](double v) { return juce::String(v, 1); };
    driveKnob .slider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    mixKnob   .slider.textFromValueFunction = [](double v) { return juce::String(v * 100.0, 1) + " %"; };
    outKnob   .slider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    gainKnob  .slider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    amountKnob.slider.textFromValueFunction = [](double v) { return juce::String(v * 100.0, 1) + " %"; };
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

    // ---- Slope ----
    slope12.onClick = [this]() { setParam(ParamID::filterSlope, 0.0f); };
    slope24.onClick = [this]() { setParam(ParamID::filterSlope, 1.0f); };
    content.addAndMakeVisible(slope12);
    content.addAndMakeVisible(slope24);

    // ---- Header: presets, A/B, power ----
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setFont(aurora::mono(12.0f));
    presetLabel.setColour(juce::Label::textColourId, aurora::baseSurface);
    presetLabel.setColour(juce::Label::outlineColourId, aurora::border);
    presetLabel.setText(presets()[0].name, juce::dontSendNotification);
    content.addAndMakeVisible(presetLabel);

    prevPreset.onClick = [this]() {
        applyPreset((currentPreset + (int)presets().size() - 1) % (int)presets().size()); };
    nextPreset.onClick = [this]() {
        applyPreset((currentPreset + 1) % (int)presets().size()); };
    content.addAndMakeVisible(prevPreset);
    content.addAndMakeVisible(nextPreset);

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

    // ---- Modulation routing buttons ----
    const char* srcNames[4] = { "OFF", "LFO A", "LFO B", "ENV" };
    for (int i = 0; i < 4; ++i)
    {
        srcBtns[i].setButtonText(srcNames[i]);
        srcBtns[i].onClick = [this, i]() { setParam(ParamID::fltModSource, (float)i); };
        content.addAndMakeVisible(srcBtns[i]);
    }
    const char* dstNames[3] = { "FREQ", "RES", "DRIVE" };
    for (int i = 0; i < 3; ++i)
    {
        dstBtns[i].setButtonText(dstNames[i]);
        dstBtns[i].onClick = [this, i]() { setParam(ParamID::fltModDest, (float)i); };
        content.addAndMakeVisible(dstBtns[i]);
    }

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

    // ---- Static panel art ----
    content.onPaint = [this](juce::Graphics& g) { /* drawn in editor paint helper */
        g.fillAll(aurora::baseSurface);

        // Header
        g.setColour(aurora::graphBg);
        g.fillRect(0, 0, kW, kHeaderH);
        g.setColour(aurora::baseSurface);
        g.setFont(aurora::heading(20.0f));
        g.drawText("AURORA FILTER", 24, 10, 320, 24, juce::Justification::centredLeft);
        g.setColour(aurora::textSecond.brighter(0.4f));
        g.setFont(aurora::mono(10.0f));
        g.drawText("MODEL AF-74 MK III", 24, 36, 320, 14, juce::Justification::centredLeft);

        // Panels
        auto panel = [&g](juce::Rectangle<int> r, const char* title) {
            g.setColour(aurora::warmBeige);
            g.fillRoundedRectangle(r.toFloat(), 4.0f);
            g.setColour(aurora::border);
            g.drawRoundedRectangle(r.toFloat(), 4.0f, 1.2f);
            g.setColour(aurora::textPrimary);
            g.setFont(aurora::heading(12.0f));
            g.drawText(title, r.getX() + 16, r.getY() + 10, r.getWidth() - 32, 14,
                       juce::Justification::centredLeft);
        };
        panel({ 16, kFilterY, kW - 32, kFilterH }, "FILTER");
        panel(kModPanel, "MODULATION");
        panel(kLfoPanel, "LFO ENGINE");
        panel(kEnvPanel, "ENVELOPE FOLLOWER");

        // Knob titles
        auto title = [&g](juce::Component& c, const char* t) {
            g.setColour(aurora::textPrimary);
            g.setFont(aurora::heading(10.0f));
            g.drawText(t, c.getX(), c.getY() - 14, c.getWidth(), 12, juce::Justification::centred);
        };
        title(freqKnob.slider,   "FREQUENCY");
        title(resKnob.slider,    "RESONANCE");
        title(driveKnob.slider,  "DRIVE");
        title(mixKnob.slider,    "MIX");
        title(outKnob.slider,    "OUTPUT");
        title(gainKnob.slider,   "GAIN");
        title(amountKnob.slider, "AMOUNT");
        title(rateKnob.slider,   "RATE");
        title(depthKnob.slider,  "DEPTH");
        title(phaseKnob.slider,  "PHASE");
        title(atkKnob.slider,    "ATTACK");
        title(relKnob.slider,    "RELEASE");
        title(sensKnob.slider,   "SENS");

        // Section sub-labels
        g.setColour(aurora::textSecond);
        g.setFont(aurora::mono(9.5f));
        g.drawText("LFO SOURCE",  kModPanel.getX() + 16, kModPanel.getY() + 32,  200, 12, juce::Justification::centredLeft);
        g.drawText("DESTINATION", kModPanel.getX() + 16, kModPanel.getY() + 96,  200, 12, juce::Justification::centredLeft);
        g.drawText("SLOPE",       kW - 200, kFilterY + 30, 120, 12, juce::Justification::centredLeft);
        g.drawText("OUTPUT",      kW - 112, kDisplayY - 14, 96, 12, juce::Justification::centred);

        // Mod-active LED
        const auto* pSrc = apvts.getRawParameterValue(ParamID::fltModSource);
        const bool modOn = pSrc && (int)pSrc->load() != 0;
        g.setColour(modOn ? aurora::led : aurora::border.withAlpha(0.4f));
        g.fillEllipse((float)kModPanel.getRight() - 28.0f, (float)kModPanel.getY() + 12.0f, 9.0f, 9.0f);

        // Footer — SYSTEM STATUS
        g.setColour(aurora::graphBg);
        g.fillRect(0, kFooterY, kW, kH - kFooterY);
        g.setColour(aurora::textSecond.brighter(0.4f));
        g.setFont(aurora::mono(10.0f));
        g.drawText("SYSTEM STATUS", 24, kFooterY, 140, kH - kFooterY, juce::Justification::centredLeft);

        auto field = [&g](int x, const char* name, const juce::String& val) {
            g.setColour(aurora::textSecond.brighter(0.2f));
            g.drawText(name, x, kFooterY + 10, 130, 14, juce::Justification::centredLeft);
            g.setColour(aurora::graphLine);
            g.drawText(val,  x, kFooterY + 28, 130, 14, juce::Justification::centredLeft);
        };
        const float srK = analysis.sampleRate.load() / 1000.0f;
        field(220, "SAMPLE RATE",  juce::String(srK, 1) + " kHz");
        field(400, "OVERSAMPLING", "1x");
        field(580, "LATENCY",      "0.00 ms");
        field(760, "CPU",          juce::String(analysis.cpuPct.load(), 1) + " %");
        field(940, "SIGNAL PATH",  "STEREO");

        g.setColour(aurora::textSecond.brighter(0.2f));
        g.drawText("v" + juce::String(JucePlugin_VersionString),
                   kW - 140, kFooterY, 116, kH - kFooterY, juce::Justification::centredRight);
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

void AuroraFilterEditor::applyPreset(int index)
{
    currentPreset = juce::jlimit(0, (int)presets().size() - 1, index);
    for (const auto& [id, v] : presets()[(size_t)currentPreset].values)
        setParam(id, v);
    presetLabel.setText(presets()[(size_t)currentPreset].name, juce::dontSendNotification);
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

    const int src = raw(ParamID::fltModSource);
    for (int i = 0; i < 4; ++i) color(srcBtns[i], src == i);
    const int dst = raw(ParamID::fltModDest);
    for (int i = 0; i < 3; ++i) color(dstBtns[i], dst == i);

    const int wave = raw(boundLfo == 1 ? ParamID::fltLfoBWave : ParamID::fltLfoAWave);
    for (int i = 0; i < 4; ++i) color(waveBtns[i], wave == i);

    color(btnA, activeSlot == 0);
    color(btnB, activeSlot == 1);

    const bool on = raw(ParamID::filterBypass) == 0;
    if (powerBtn.getToggleState() != on)
        powerBtn.setToggleState(on, juce::dontSendNotification);
    powerBtn.setColour(juce::TextButton::buttonColourId,
                       on ? aurora::led : aurora::baseSurface);
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

    // Refresh dynamic painted regions (footer fields, mod LED)
    content.repaint(0, kFooterY, kW, kH - kFooterY);
    content.repaint(kModPanel.getRight() - 40, kModPanel.getY(), 40, 28);
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
    prevPreset .setBounds(560, 18, 36, 28);
    presetLabel.setBounds(600, 18, 200, 28);
    nextPreset .setBounds(804, 18, 36, 28);
    btnA    .setBounds(1180, 18, 34, 28);
    btnB    .setBounds(1218, 18, 34, 28);
    powerBtn.setBounds(1284, 18, 92, 28);

    // ---- Display + meters ----
    display.setBounds(16, kDisplayY, kW - 32 - 112, kDisplayH);
    meters .setBounds(kW - 112, kDisplayY, 96, kDisplayH);

    // ---- Filter section ----
    for (int i = 0; i < 6; ++i)
        typeBtns[i].setBounds(48 + i * 106, kFilterY + 26, 96, 32);
    slope12.setBounds(kW - 200, kFilterY + 46, 56, 32);
    slope24.setBounds(kW - 138, kFilterY + 46, 56, 32);
    gainKnob.slider.setBounds(kW - 188, kFilterY + 96, 90, 76 + 17);

    const int knobY = kFilterY + 84;
    const int knobW = 110, knobH = 70 + 17 + 6;
    freqKnob .slider.setBounds(70,  knobY, knobW, knobH);
    resKnob  .slider.setBounds(290, knobY, knobW, knobH);
    driveKnob.slider.setBounds(510, knobY, knobW, knobH);
    mixKnob  .slider.setBounds(730, knobY, knobW, knobH);
    outKnob  .slider.setBounds(950, knobY, knobW, knobH);

    // ---- MODULATION panel ----
    {
        const int px = kModPanel.getX(), py = kModPanel.getY();
        for (int i = 0; i < 4; ++i)
            srcBtns[i].setBounds(px + 16 + i * 70, py + 48, 64, 26);
        for (int i = 0; i < 3; ++i)
            dstBtns[i].setBounds(px + 16 + i * 90, py + 112, 84, 26);
        amountKnob.slider.setBounds(px + 320, py + 52, 94, 70 + 17);
    }

    // ---- LFO ENGINE panel ----
    {
        const int px = kLfoPanel.getX(), py = kLfoPanel.getY();
        for (int i = 0; i < 4; ++i)
            waveBtns[i].setBounds(px + 16 + i * 56, py + 32, 50, 24);
        lfoScope.setBounds(px + 16, py + 66, 208, 96);
        rateKnob .slider.setBounds(px + 238, py + 60, 72, 60 + 17);
        depthKnob.slider.setBounds(px + 314, py + 60, 72, 60 + 17);
        phaseKnob.slider.setBounds(px + 390, py + 60, 72, 60 + 17);
    }

    // ---- ENVELOPE FOLLOWER panel ----
    {
        const int px = kEnvPanel.getX(), py = kEnvPanel.getY();
        envScope.setBounds(px + 16, py + 44, 180, 118);
        atkKnob .slider.setBounds(px + 208, py + 60, 72, 60 + 17);
        relKnob .slider.setBounds(px + 284, py + 60, 72, 60 + 17);
        sensKnob.slider.setBounds(px + 360, py + 60, 72, 60 + 17);
    }
}
