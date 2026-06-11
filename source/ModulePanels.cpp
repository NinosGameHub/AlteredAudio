#include "ModulePanels.h"
#include "AaLookAndFeel.h"
#include "ChainStrip.h"

// ============================================================
//  ModulePanel base
// ============================================================

ModulePanel::ModulePanel(AlteredAudioProcessor& p,
                         const juce::String& name,
                         const juce::String& bypassId)
    : proc(p), moduleName(name), bypassParamId(bypassId)
{
    addAndMakeVisible(activeBtn);
    activeBtn.setClickingTogglesState(true);

    activeBtn.onClick = [this]()
    {
        if (auto* p2 = proc.getAPVTS().getParameter(bypassParamId))
            p2->setValueNotifyingHost(activeBtn.getToggleState() ? 0.0f : 1.0f);
    };

    startTimerHz(10);
}

ModulePanel::~ModulePanel()
{
    stopTimer();
}

void ModulePanel::timerCallback()
{
    const std::atomic<float>* raw = proc.getAPVTS().getRawParameterValue(bypassParamId);
    const bool active = raw ? (*raw < 0.5f) : false;
    if (activeBtn.getToggleState() != active)
        activeBtn.setToggleState(active, juce::dontSendNotification);
}

void ModulePanel::paint(juce::Graphics& g)
{
    g.fillAll(AaColor::bg);

    const auto header = getLocalBounds().removeFromTop(kHeaderH).toFloat();
    g.setColour(AaColor::surface);
    g.fillRect(header);
    g.setColour(AaColor::border);
    g.fillRect(header.getX(), header.getBottom() - 1.0f, header.getWidth(), 1.0f);

    g.setColour(AaColor::textPrimary);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(moduleName, header.withTrimmedRight(80.0f).withTrimmedLeft(16.0f),
               juce::Justification::centredLeft);
}

void ModulePanel::resized()
{
    activeBtn.setBounds(getWidth() - 90, (kHeaderH - 22) / 2, 80, 22);
}

juce::Rectangle<int> ModulePanel::contentArea() const
{
    return getLocalBounds().withTrimmedTop(kHeaderH).reduced(16, 8);
}

// ---- Helpers ----------------------------------------------------------------

void ModulePanel::makeKnob(Knob& k, const juce::String& paramId, const juce::String& labelText)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
    addAndMakeVisible(k.slider);

    k.label.setText(labelText, juce::dontSendNotification);
    k.label.setJustificationType(juce::Justification::centred);
    k.label.setFont(juce::Font(10.0f));
    k.label.setColour(juce::Label::textColourId, AaColor::textSecond);
    addAndMakeVisible(k.label);

    k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        proc.getAPVTS(), paramId, k.slider);
}

void ModulePanel::makeCombo(Combo& c, const juce::String& paramId,
                             const juce::String& labelText,
                             const juce::StringArray& choices)
{
    for (int i = 0; i < choices.size(); ++i)
        c.box.addItem(choices[i], i + 1);
    addAndMakeVisible(c.box);

    c.label.setText(labelText, juce::dontSendNotification);
    c.label.setJustificationType(juce::Justification::centredLeft);
    c.label.setFont(juce::Font(10.0f));
    c.label.setColour(juce::Label::textColourId, AaColor::textSecond);
    addAndMakeVisible(c.label);

    c.attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        proc.getAPVTS(), paramId, c.box);
}

// ============================================================
//  StubPanel
// ============================================================

StubPanel::StubPanel(AlteredAudioProcessor& p,
                     const juce::String& name,
                     const juce::String& bypassId)
    : ModulePanel(p, name, bypassId)
{}

void StubPanel::paint(juce::Graphics& g)
{
    ModulePanel::paint(g);
    g.setColour(AaColor::textSecond);
    g.setFont(juce::Font(13.0f));
    g.drawText("Controls coming soon", contentArea(), juce::Justification::centred);
}

// ============================================================
//  Layout helpers (local to this file)
// ============================================================

namespace
{
    static constexpr int kLabelH = 14;
    static constexpr int kKnobH  = 68;
    static constexpr int kRowH   = kLabelH + kKnobH;
    static constexpr int kComboH = 24;

    void placeKnobRow(ModulePanel::Knob** knobs, int count,
                      int x, int y, int totalW, int gap = 4)
    {
        const int slotW = totalW / count;
        for (int i = 0; i < count; ++i)
        {
            const int kx = x + i * slotW;
            const int kw = slotW - gap;
            knobs[i]->label.setBounds(kx, y,           kw, kLabelH);
            knobs[i]->slider.setBounds(kx, y + kLabelH, kw, kKnobH);
        }
    }
}

// ============================================================
//  EQCurveDisplay
// ============================================================

EQCurveDisplay::EQCurveDisplay(AlteredAudioProcessor& p) : proc(p)
{
    startTimerHz(30);
    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

void EQCurveDisplay::timerCallback()
{
    auto& apvts = proc.getAPVTS();
    bool changed = false;
    for (int i = 0; i < EQModule::kMaxBands; ++i)
    {
        const int n = i + 1;
        const auto* en = apvts.getRawParameterValue(ParamID::eqEnabled(n));
        const auto* ty = apvts.getRawParameterValue(ParamID::eqType(n));
        const auto* fr = apvts.getRawParameterValue(ParamID::eqFreq(n));
        const auto* q  = apvts.getRawParameterValue(ParamID::eqQ(n));
        const auto* ga = apvts.getRawParameterValue(ParamID::eqGain(n));

        BandData nd;
        nd.on   = en && *en > 0.5f;
        nd.type = ty ? (int)ty->load() : 5;
        nd.freq = fr ? fr->load() : 1000.0f;
        nd.q    = q  ? q->load()  : 0.7f;
        nd.gain = ga ? ga->load() : 0.0f;

        if (nd.on != bands[i].on || nd.type != bands[i].type ||
            nd.freq != bands[i].freq || nd.q != bands[i].q || nd.gain != bands[i].gain)
        {
            bands[i] = nd;
            changed  = true;
        }
    }
    if (changed)
        repaint();
}

// ---- coordinate helpers ----

float EQCurveDisplay::xForF(float f, float w) const
{
    const float plotW = w - padL - padR;
    return padL + (std::log10(juce::jlimit(FMIN, FMAX, f)) - std::log10(FMIN))
                  / (std::log10(FMAX) - std::log10(FMIN)) * plotW;
}

float EQCurveDisplay::yForG(float db, float h) const
{
    const float plotH = h - padT - padB;
    return padT + (1.f - (db + GMAX) / (2.f * GMAX)) * plotH;
}

float EQCurveDisplay::fForX(float x, float w) const
{
    const float plotW = w - padL - padR;
    return std::pow(10.f, std::log10(FMIN)
                    + ((x - padL) / plotW) * (std::log10(FMAX) - std::log10(FMIN)));
}

float EQCurveDisplay::gForY(float y, float h) const
{
    const float plotH = h - padT - padB;
    return (1.f - (y - padT) / plotH) * 2.f * GMAX - GMAX;
}

int EQCurveDisplay::bandAtPos(float x, float y) const
{
    const float w = (float)getWidth(), h = (float)getHeight();
    constexpr float hitR = 12.f;
    float bestD = hitR;
    int   found = -1;
    for (int i = 0; i < EQModule::kMaxBands; ++i)
    {
        const float cx = xForF(bands[i].freq, w);
        const float cy = yForG(bands[i].on ? bands[i].gain : 0.f, h);
        const float d  = std::hypot(x - cx, y - cy);
        if (d < bestD) { bestD = d; found = i; }
    }
    return found;
}

// ---- APVTS writers ----

void EQCurveDisplay::setBandFreq(int band, float hz)
{
    if (auto* p = proc.getAPVTS().getParameter(ParamID::eqFreq(band + 1)))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(hz));
}

void EQCurveDisplay::setBandGain(int band, float db)
{
    if (auto* p = proc.getAPVTS().getParameter(ParamID::eqGain(band + 1)))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(db));
}

void EQCurveDisplay::setBandEnabled(int band, bool on)
{
    if (auto* p = proc.getAPVTS().getParameter(ParamID::eqEnabled(band + 1)))
        p->setValueNotifyingHost(on ? 1.0f : 0.0f);
}

// ---- selection ----

void EQCurveDisplay::setSelectedBand(int band)
{
    selectedBand = band;
    repaint();
    if (onBandSelected)
        onBandSelected(band);
}

// ---- mouse ----

void EQCurveDisplay::mouseDown(const juce::MouseEvent& e)
{
    const float mx = (float)e.x, my = (float)e.y;
    const float w = (float)getWidth(), h = (float)getHeight();

    int hit = bandAtPos(mx, my);
    if (hit >= 0)
    {
        dragBand = hit;
        setSelectedBand(hit);
        return;
    }

    // Click on empty curve area — place a new band
    const float clickFreq = juce::jlimit(FMIN, FMAX, fForX(mx, w));
    const float clickGain = juce::jlimit(-GMAX, GMAX, gForY(my, h));

    // Find first disabled band
    int target = -1;
    for (int i = 0; i < EQModule::kMaxBands; ++i)
        if (!bands[i].on) { target = i; break; }

    if (target >= 0)
    {
        setBandFreq   (target, clickFreq);
        setBandGain   (target, clickGain);
        setBandEnabled(target, true);
        dragBand = target;
        setSelectedBand(target);
    }
    else
    {
        // All bands active — select the nearest one by frequency
        float bestD = 1e9f;
        for (int i = 0; i < EQModule::kMaxBands; ++i)
        {
            const float d = std::abs(std::log10(bands[i].freq) - std::log10(clickFreq));
            if (d < bestD) { bestD = d; target = i; }
        }
        dragBand = target;
        setSelectedBand(target);
    }
}

void EQCurveDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if (dragBand < 0) return;
    const float w = (float)getWidth(), h = (float)getHeight();
    const float mx = juce::jlimit(padL, w - padR, (float)e.x);
    const float my = juce::jlimit(padT, h - padB, (float)e.y);

    setBandFreq(dragBand, juce::jlimit(FMIN, FMAX, fForX(mx, w)));
    setBandGain(dragBand, juce::jlimit(-GMAX, GMAX, gForY(my, h)));
}

void EQCurveDisplay::mouseUp(const juce::MouseEvent&)
{
    dragBand = -1;
}

// ---- response curve math ----

float EQCurveDisplay::computeResponseDb(float freqHz) const
{
    float total = 0.0f;
    for (int i = 0; i < EQModule::kMaxBands; ++i)
    {
        const auto& b = bands[i];
        if (!b.on) continue;
        const float q   = std::max(0.1f, b.q);
        const float oct = std::log2(freqHz / std::max(1.0f, b.freq));
        const float bw  = 1.0f / q;
        switch (b.type)
        {
            case 5: // Peak
                total += b.gain * std::exp(-(oct * oct) / (2.0f * bw * bw));
                break;
            case 6: // LowShelf
                total += b.gain / (1.0f + std::pow(std::max(0.0001f, freqHz / b.freq), 2.2f));
                break;
            case 7: // HighShelf
                total += b.gain / (1.0f + std::pow(std::max(0.0001f, b.freq / freqHz), 2.2f));
                break;
            case 0: // LowPass
                if (freqHz > b.freq)
                    total -= 20.0f * q * std::log10(freqHz / b.freq);
                break;
            case 1: // HighPass
                if (freqHz < b.freq)
                    total -= 20.0f * q * std::log10(b.freq / freqHz);
                break;
            default: break;
        }
    }
    return juce::jlimit(-18.0f, 18.0f, total);
}

// ---- paint ----

void EQCurveDisplay::paint(juce::Graphics& g)
{
    const auto  bounds = getLocalBounds().toFloat();
    const float w      = bounds.getWidth();
    const float h      = bounds.getHeight();
    const float plotW  = w - padL - padR;
    const float plotH  = h - padT - padB;

    g.setColour(AaColor::crtBg);
    g.fillRoundedRectangle(bounds, 4.0f);

    const float logFMin = std::log10(FMIN), logFMax = std::log10(FMAX);

    // Selected band frequency guide line
    if (selectedBand >= 0)
    {
        const float selX = xForF(bands[selectedBand].freq, w);
        g.setColour(AaColor::catFilterEQ.withAlpha(0.18f));
        g.drawVerticalLine((int)selX, padT, padT + plotH);
    }

    // Freq grid
    for (float f : { 20.f,50.f,100.f,200.f,500.f,1000.f,2000.f,5000.f,10000.f,20000.f })
    {
        g.setColour(AaColor::crtAmber.withAlpha(0.09f));
        g.drawVerticalLine((int)xForF(f, w), padT, padT + plotH);
    }
    // dB grid
    for (float db : { -12.f,-6.f,0.f,6.f,12.f })
    {
        g.setColour(db == 0.f ? AaColor::crtAmber.withAlpha(0.28f)
                              : AaColor::crtAmber.withAlpha(0.09f));
        g.drawHorizontalLine((int)yForG(db, h), padL, padL + plotW);
    }

    // Response curve (200 points)
    juce::Path fill, line;
    const float zeroY = yForG(0.f, h);
    for (int i = 0; i <= 200; ++i)
    {
        const float t  = (float)i / 200.f;
        const float f  = std::pow(10.f, logFMin + t * (logFMax - logFMin));
        const float db = computeResponseDb(f);
        const float x  = padL + t * plotW;
        const float y  = yForG(db, h);
        if (i == 0) { fill.startNewSubPath(x, y); line.startNewSubPath(x, y); }
        else        { fill.lineTo(x, y);           line.lineTo(x, y); }
    }
    fill.lineTo(padL + plotW, zeroY);
    fill.lineTo(padL, zeroY);
    fill.closeSubPath();

    g.setColour(AaColor::catFilterEQ.withAlpha(0.14f));
    g.fillPath(fill);
    g.setColour(AaColor::catFilterEQ);
    g.strokePath(line, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));

    // Band handles (selected on top, so draw unselected first)
    g.setFont(juce::Font(8.0f));
    for (int pass = 0; pass < 2; ++pass)
    {
        for (int i = 0; i < EQModule::kMaxBands; ++i)
        {
            const bool isSel = (i == selectedBand);
            if (pass == 0 && isSel)  continue;
            if (pass == 1 && !isSel) continue;

            const auto& b  = bands[i];
            const float cx = xForF(b.freq, w);
            const float cy = yForG(b.on ? b.gain : 0.f, h);

            if (isSel)
            {
                // outer glow ring
                g.setColour(AaColor::catFilterEQ.withAlpha(0.35f));
                g.fillEllipse(cx - 13.f, cy - 13.f, 26.f, 26.f);
                // filled circle
                g.setColour(b.on ? AaColor::catFilterEQ : AaColor::inactive);
                g.fillEllipse(cx - 9.f, cy - 9.f, 18.f, 18.f);
                g.setColour(juce::Colours::white);
                g.setFont(juce::Font(9.0f, juce::Font::bold));
                g.drawText(juce::String(i + 1),
                           juce::Rectangle<float>(cx - 9.f, cy - 9.f + 1.f, 18.f, 18.f),
                           juce::Justification::centred);
            }
            else
            {
                const float r = b.on ? 7.f : 5.f;
                g.setColour(b.on ? AaColor::catFilterEQ.withAlpha(0.6f) : AaColor::inactive);
                g.fillEllipse(cx - r, cy - r, r * 2.f, r * 2.f);
                g.setColour(juce::Colours::white.withAlpha(0.7f));
                g.setFont(juce::Font(8.0f));
                g.drawText(juce::String(i + 1),
                           juce::Rectangle<float>(cx - r, cy - r + 1.f, r * 2.f, r * 2.f),
                           juce::Justification::centred);
            }
        }
    }

    // Freq labels
    g.setColour(AaColor::crtAmber.withAlpha(0.55f));
    g.setFont(juce::Font(9.0f));
    for (auto [f, lbl] : std::initializer_list<std::pair<float,const char*>>{
            {20,"20"},{100,"100"},{1000,"1k"},{10000,"10k"},{20000,"20k"}})
    {
        g.drawText(lbl, juce::Rectangle<float>(xForF(f, w) - 16.f, h - padB + 2.f, 32.f, 12.f),
                   juce::Justification::centred);
    }
}

// ============================================================
//  GRMeter
// ============================================================

GRMeter::GRMeter() { startTimerHz(30); }

void GRMeter::setSource(std::function<float()> fn) { getGRFn = std::move(fn); }

void GRMeter::timerCallback()
{
    const float newDb = getGRFn ? getGRFn() : 0.0f;
    bool needRepaint  = false;

    if (newDb < peakDb)
    {
        peakDb       = newDb;
        peakHoldSecs = 2.0f;
        needRepaint  = true;
    }
    if (peakHoldSecs > 0.0f)
    {
        peakHoldSecs -= 1.0f / 30.0f;
        if (peakHoldSecs <= 0.0f) { peakDb = 0.0f; needRepaint = true; }
    }
    if (std::abs(newDb - currentDb) > 0.05f)
    {
        currentDb   = newDb;
        needRepaint = true;
    }
    if (needRepaint) repaint();
}

void GRMeter::paint(juce::Graphics& g)
{
    const auto  b    = getLocalBounds().toFloat();
    const float w    = b.getWidth();
    const float h    = b.getHeight();
    constexpr float GR_MAX = 24.f;
    const float barX = w * 0.25f, barW = w * 0.50f;
    const float barTop = 14.f, barH = h - barTop - 26.f;

    // Background
    g.setColour(AaColor::crtBg);
    g.fillRoundedRectangle(b, 3.0f);

    // Track
    g.setColour(AaColor::surfaceAlt);
    g.fillRoundedRectangle(barX, barTop, barW, barH, 2.0f);

    // GR fill (downward from top)
    const float grFrac = juce::jlimit(0.f, 1.f, -currentDb / GR_MAX);
    if (grFrac > 0.001f)
    {
        g.setColour(grFrac > 0.6f ? juce::Colour(0xFFD44C00) : AaColor::catDynamics);
        g.fillRoundedRectangle(barX, barTop, barW, grFrac * barH, 2.0f);
    }

    // Peak hold
    if (peakDb < -0.5f)
    {
        const float peakFrac = juce::jlimit(0.f, 1.f, -peakDb / GR_MAX);
        g.setColour(AaColor::catDynamics.brighter(0.5f));
        g.fillRect(barX, barTop + peakFrac * barH - 1.f, barW, 2.f);
    }

    // Labels
    g.setFont(juce::Font(9.0f));
    g.setColour(AaColor::textSecond);
    g.drawText("GR", b.withTrimmedTop(4.f).withHeight(10.f), juce::Justification::centred);
    if (-currentDb > 0.5f)
    {
        g.setColour(AaColor::catDynamics);
        g.drawText(juce::String((int)-currentDb) + "dB",
                   b.withTrimmedTop(h - 14.f).withHeight(12.f),
                   juce::Justification::centred);
    }
}

// ============================================================
//  FilterPanel
// ============================================================

FilterPanel::FilterPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "FILTER", ParamID::filterBypass)
{
    const juce::StringArray types { "LowPass","HighPass","BandPass","Notch",
                                    "AllPass","Peak","LowShelf","HighShelf" };
    makeCombo(typeCombo, ParamID::filterType, "TYPE", types);
    makeKnob(freqKnob, ParamID::filterFreq, "FREQ");
    makeKnob(qKnob,    ParamID::filterQ,    "Q");
    makeKnob(gainKnob, ParamID::filterGain, "GAIN");
}

void FilterPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    typeCombo.label.setBounds(x,            y,             150, kLabelH);
    typeCombo.box.setBounds  (x,            y + kLabelH,   150, kComboH);

    Knob* row[] = { &freqKnob, &qKnob, &gainKnob };
    placeKnobRow(row, 3, x, y + kLabelH + kComboH + 12, w);
}

// ============================================================
//  EQPanel
// ============================================================

EQPanel::EQPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "EQ", ParamID::eqBypass), curveDisplay(p)
{
    addAndMakeVisible(curveDisplay);

    bandLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    bandLabel.setColour(juce::Label::textColourId, AaColor::catFilterEQ);
    bandLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(bandLabel);

    const juce::StringArray types { "LowPass","HighPass","BandPass","Notch",
                                    "AllPass","Peak","LowShelf","HighShelf" };
    for (int i = 0; i < kBands; ++i)
    {
        const int n = i + 1;
        bands[i].enableBtn.setButtonText("ACTIVE");
        bands[i].enableBtn.setClickingTogglesState(true);
        addAndMakeVisible(bands[i].enableBtn);
        bands[i].enableAttach =
            std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                proc.getAPVTS(), ParamID::eqEnabled(n), bands[i].enableBtn);

        makeCombo(bands[i].typeCombo, ParamID::eqType(n),  "TYPE", types);
        makeKnob (bands[i].freqKnob,  ParamID::eqFreq(n),  "FREQ");
        makeKnob (bands[i].qKnob,     ParamID::eqQ(n),     "Q");
        makeKnob (bands[i].gainKnob,  ParamID::eqGain(n),  "GAIN");
    }

    curveDisplay.onBandSelected = [this](int idx) { showBand(idx); };
    showBand(0);
}

void EQPanel::showBand(int idx)
{
    bandLabel.setText("BAND " + juce::String(idx + 1), juce::dontSendNotification);
    for (int i = 0; i < kBands; ++i)
    {
        const bool vis = (i == idx);
        bands[i].enableBtn.setVisible(vis);
        bands[i].typeCombo.label.setVisible(vis);
        bands[i].typeCombo.box.setVisible(vis);
        bands[i].freqKnob.label.setVisible(vis);
        bands[i].freqKnob.slider.setVisible(vis);
        bands[i].qKnob.label.setVisible(vis);
        bands[i].qKnob.slider.setVisible(vis);
        bands[i].gainKnob.label.setVisible(vis);
        bands[i].gainKnob.slider.setVisible(vis);
    }
}

void EQPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int  ax = area.getX(), ay = area.getY(), aw = area.getWidth(), ah = area.getHeight();

    // Reserve space for controls: label row + combo row + knob row
    constexpr int labelRowH = 26;
    constexpr int comboRowH = kLabelH + kComboH;
    constexpr int knobRowH  = kLabelH + kKnobH;
    constexpr int ctrlTotal = labelRowH + 6 + comboRowH + 6 + knobRowH;

    const int curveH = juce::jmax(120, ah - 8 - ctrlTotal);
    const int ctrlY  = ay + curveH + 8;

    curveDisplay.setBounds(ax, ay, aw, curveH);

    // Band label + enable toggle
    bandLabel.setBounds(ax, ctrlY, 100, labelRowH);
    const int comboW  = 150;
    const int knobsX  = ax + comboW + 12;
    const int knobsW  = aw - comboW - 12;
    const int comboY  = ctrlY + labelRowH + 6;
    const int knobY   = comboY + comboRowH + 6;

    for (int i = 0; i < kBands; ++i)
    {
        bands[i].enableBtn.setBounds(ax + aw - 80, ctrlY, 80, labelRowH);

        bands[i].typeCombo.label.setBounds(ax, comboY,           comboW, kLabelH);
        bands[i].typeCombo.box.setBounds  (ax, comboY + kLabelH, comboW, kComboH);

        Knob* krow[] = { &bands[i].freqKnob, &bands[i].qKnob, &bands[i].gainKnob };
        placeKnobRow(krow, 3, knobsX, knobY, knobsW);
    }
}

// ============================================================
//  GainPanel
// ============================================================

GainPanel::GainPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "GAIN", ParamID::gainBypass)
{
    makeKnob(gainKnob, ParamID::gainDb, "GAIN (dB)");
}

void GainPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    gainKnob.label.setBounds (area.getX(), area.getY(),           120, kLabelH);
    gainKnob.slider.setBounds(area.getX(), area.getY() + kLabelH, 120, kKnobH);
}

// ============================================================
//  DelayPanel
// ============================================================

DelayPanel::DelayPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "DELAY", ParamID::delayBypass)
{
    makeKnob(timeKnob,      ParamID::delayTime,      "TIME (ms)");
    makeKnob(feedbackKnob,  ParamID::delayFeedback,  "FEEDBACK");
    makeKnob(spreadKnob,    ParamID::delaySpread,    "SPREAD (ms)");
    makeKnob(wetDryKnob,    ParamID::delayWetDry,    "WET/DRY");
    makeKnob(fbLPKnob,      ParamID::delayFbLPHz,    "TONE LP");
    makeKnob(fbHPKnob,      ParamID::delayFbHPHz,    "TONE HP");
    makeKnob(duckingKnob,   ParamID::delayDucking,   "DUCKING");
    makeKnob(diffusionKnob, ParamID::delayDiffusion, "DIFFUSION");
    makeKnob(wowKnob,       ParamID::delayWow,       "WOW");
    makeKnob(modRateKnob,   ParamID::delayModRate,   "MOD RATE");
    makeKnob(modDepthKnob,  ParamID::delayModDepth,  "MOD DEPTH");

    addAndMakeVisible(pingPongBtn);
    pingPongAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        proc.getAPVTS(), ParamID::delayPingPong, pingPongBtn);
}

void DelayPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    Knob* row1[] = { &timeKnob, &feedbackKnob, &spreadKnob, &wetDryKnob };
    placeKnobRow(row1, 4, x, y, w);

    Knob* row2[] = { &fbLPKnob, &fbHPKnob, &duckingKnob, &diffusionKnob };
    placeKnobRow(row2, 4, x, y + kRowH + 12, w);

    Knob* row3[] = { &wowKnob, &modRateKnob, &modDepthKnob };
    placeKnobRow(row3, 3, x, y + (kRowH + 12) * 2, w);

    pingPongBtn.setBounds(x + w * 3 / 4, y + (kRowH + 12) * 2 + kRowH / 2 - 11, 110, 22);
}

// ============================================================
//  ReverbPanel
// ============================================================

ReverbPanel::ReverbPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "REVERB", ParamID::reverbBypass)
{
    makeKnob(decayKnob,    ParamID::reverbRoom,      "DECAY");
    makeKnob(dampKnob,     ParamID::reverbDamp,      "DAMPING");
    makeKnob(diffusionKnob,ParamID::reverbDiffusion, "DIFFUSION");
    makeKnob(preDelayKnob, ParamID::reverbPreDelay,  "PRE-DELAY");
    makeKnob(wetDryKnob,   ParamID::reverbWetDry,    "WET/DRY");
    makeKnob(modRateKnob,  ParamID::reverbModRate,   "MOD RATE");
    makeKnob(erLevelKnob,  ParamID::reverbErLevel,   "ER LEVEL");
    makeKnob(lowCutKnob,   ParamID::reverbLowCut,    "LOW CUT");
    makeKnob(shimmerKnob,  ParamID::reverbShimmer,   "SHIMMER");
    makeKnob(pitchSemiKnob,ParamID::reverbPitchSemi, "PITCH SEMI");
}

void ReverbPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    Knob* row1[] = { &decayKnob, &dampKnob, &diffusionKnob, &preDelayKnob, &wetDryKnob };
    placeKnobRow(row1, 5, x, y, w);

    Knob* row2[] = { &modRateKnob, &erLevelKnob, &lowCutKnob, &shimmerKnob, &pitchSemiKnob };
    placeKnobRow(row2, 5, x, y + kRowH + 12, w);
}

// ============================================================
//  WaveshaperPanel
// ============================================================

WaveshaperPanel::WaveshaperPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "WAVESHAPER", ParamID::wsBypass)
{
    makeCombo(algoCombo, ParamID::wsAlgo, "ALGORITHM",
              { "SoftClip","HardClip","TanhSat","BitCrush","FoldBack" });
    makeKnob(driveKnob,    ParamID::wsDrive,    "DRIVE");
    makeKnob(wetDryKnob,   ParamID::wsWetDry,   "WET/DRY");
    makeKnob(bitDepthKnob, ParamID::wsBitDepth, "BIT DEPTH");
}

void WaveshaperPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    algoCombo.label.setBounds(x,          y,            180, kLabelH);
    algoCombo.box.setBounds  (x,          y + kLabelH,  180, kComboH);

    Knob* row[] = { &driveKnob, &wetDryKnob, &bitDepthKnob };
    placeKnobRow(row, 3, x, y + kLabelH + kComboH + 12, w);
}

// ============================================================
//  CompressorPanel
// ============================================================

CompressorPanel::CompressorPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "COMPRESSOR", ParamID::compBypass)
{
    grMeter.setSource([&p]() {
        if (auto* m = p.getCompressorModule()) return m->getGainReductionDb();
        return 0.0f;
    });
    addAndMakeVisible(grMeter);

    makeKnob(threshKnob,     ParamID::compThreshold,  "THRESH");
    makeKnob(ratioKnob,      ParamID::compRatio,      "RATIO");
    makeKnob(attackKnob,     ParamID::compAttack,     "ATTACK");
    makeKnob(releaseKnob,    ParamID::compRelease,    "RELEASE");
    makeKnob(kneeKnob,       ParamID::compKnee,       "KNEE");
    makeKnob(makeupKnob,     ParamID::compMakeup,     "MAKEUP");
    makeKnob(rmsPeakKnob,    ParamID::compRmsPeak,    "PEAK/RMS");
    makeKnob(autoRelKnob,    ParamID::compAutoRel,    "AUTO REL");
    makeKnob(satKnob,        ParamID::compSaturation, "SAT");
    makeKnob(scHPKnob,       ParamID::compScHP,       "SC HP");
    makeKnob(scLPKnob,       ParamID::compScLP,       "SC LP");
    makeKnob(stereoLinkKnob, ParamID::compStereoLink, "ST LINK");
    makeKnob(mixKnob,        ParamID::compMix,        "MIX");
    makeKnob(lookaheadKnob,  ParamID::compLookahead,  "LOOKAHEAD");
}

void CompressorPanel::resized()
{
    ModulePanel::resized();
    const auto area   = contentArea();
    const int  x      = area.getX(), y = area.getY();
    constexpr int meterW = 44, meterGap = 8;
    const int  knobW  = area.getWidth() - meterW - meterGap;

    grMeter.setBounds(area.getRight() - meterW, y, meterW, area.getHeight());

    Knob* row1[] = { &threshKnob, &ratioKnob, &attackKnob, &releaseKnob,
                     &kneeKnob, &makeupKnob, &rmsPeakKnob };
    placeKnobRow(row1, 7, x, y, knobW);

    Knob* row2[] = { &autoRelKnob, &satKnob, &scHPKnob, &scLPKnob,
                     &stereoLinkKnob, &mixKnob, &lookaheadKnob };
    placeKnobRow(row2, 7, x, y + kRowH + 12, knobW);
}

// ============================================================
//  TransientShaperPanel
// ============================================================

TransientShaperPanel::TransientShaperPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "TRANSIENT SHAPER", ParamID::transBypass)
{
    makeKnob(attackKnob,  ParamID::transAttack,  "ATTACK");
    makeKnob(sustainKnob, ParamID::transSustain, "SUSTAIN");
    makeKnob(speedKnob,   ParamID::transSpeed,   "SPEED");
    makeKnob(linkKnob,    ParamID::transLink,    "ST LINK");
    makeKnob(wetDryKnob,  ParamID::transWetDry,  "WET/DRY");
}

void TransientShaperPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    Knob* row[] = { &attackKnob, &sustainKnob, &speedKnob, &linkKnob, &wetDryKnob };
    placeKnobRow(row, 5, area.getX(), area.getY(), area.getWidth());
}

// ============================================================
//  LimiterPanel
// ============================================================

LimiterPanel::LimiterPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "LIMITER", ParamID::limBypass)
{
    grMeter.setSource([&p]() {
        if (auto* m = p.getLimiterModule()) return m->getGainReductionDb();
        return 0.0f;
    });
    addAndMakeVisible(grMeter);

    makeKnob(ceilingKnob, ParamID::limCeiling, "CEILING (dB)");
    makeKnob(releaseKnob, ParamID::limRelease, "RELEASE (ms)");
}

void LimiterPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    constexpr int meterW = 44, meterGap = 8;

    grMeter.setBounds(area.getRight() - meterW, area.getY(), meterW, area.getHeight());

    Knob* row[] = { &ceilingKnob, &releaseKnob };
    placeKnobRow(row, 2, area.getX(), area.getY(), area.getWidth() / 2);
}

// ============================================================
//  GatePanel
// ============================================================

GatePanel::GatePanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "GATE", ParamID::gateBypass)
{
    makeKnob(threshKnob,  ParamID::gateThreshold, "THRESHOLD");
    makeKnob(attackKnob,  ParamID::gateAttack,    "ATTACK");
    makeKnob(releaseKnob, ParamID::gateRelease,   "RELEASE");
    makeKnob(rangeKnob,   ParamID::gateRange,     "RANGE");
}

void GatePanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    Knob* row[] = { &threshKnob, &attackKnob, &releaseKnob, &rangeKnob };
    placeKnobRow(row, 4, area.getX(), area.getY(), area.getWidth());
}

// ============================================================
//  ChorusPanel
// ============================================================

ChorusPanel::ChorusPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "CHORUS", ParamID::chorusBypass)
{
    makeKnob(rateKnob,   ParamID::chorusRate,   "RATE (Hz)");
    makeKnob(depthKnob,  ParamID::chorusDepth,  "DEPTH (ms)");
    makeKnob(centreKnob, ParamID::chorusCentre, "CENTRE (ms)");
    makeKnob(voicesKnob, ParamID::chorusVoices, "VOICES");
    makeKnob(wetDryKnob, ParamID::chorusWetDry, "WET/DRY");
}

void ChorusPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    Knob* row[] = { &rateKnob, &depthKnob, &centreKnob, &voicesKnob, &wetDryKnob };
    placeKnobRow(row, 5, area.getX(), area.getY(), area.getWidth());
}

// ============================================================
//  FlangerPanel
// ============================================================

FlangerPanel::FlangerPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "FLANGER", ParamID::flangerBypass)
{
    makeKnob(rateKnob,     ParamID::flangerRate,     "RATE (Hz)");
    makeKnob(depthKnob,    ParamID::flangerDepth,    "DEPTH (ms)");
    makeKnob(centreKnob,   ParamID::flangerCentre,   "CENTRE (ms)");
    makeKnob(feedbackKnob, ParamID::flangerFeedback, "FEEDBACK");
    makeKnob(wetDryKnob,   ParamID::flangerWetDry,   "WET/DRY");
}

void FlangerPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    Knob* row[] = { &rateKnob, &depthKnob, &centreKnob, &feedbackKnob, &wetDryKnob };
    placeKnobRow(row, 5, area.getX(), area.getY(), area.getWidth());
}

// ============================================================
//  PhaserPanel
// ============================================================

PhaserPanel::PhaserPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "PHASER", ParamID::phaserBypass)
{
    makeCombo(stagesCombo, ParamID::phaserStages, "STAGES", { "2","4","6","8" });
    makeKnob(rateKnob,     ParamID::phaserRate,     "RATE (Hz)");
    makeKnob(depthKnob,    ParamID::phaserDepth,    "DEPTH");
    makeKnob(baseFreqKnob, ParamID::phaserBaseFreq, "BASE FREQ");
    makeKnob(feedbackKnob, ParamID::phaserFeedback, "FEEDBACK");
    makeKnob(wetDryKnob,   ParamID::phaserWetDry,   "WET/DRY");
}

void PhaserPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    stagesCombo.label.setBounds(x,         y,           120, kLabelH);
    stagesCombo.box.setBounds  (x,         y + kLabelH, 120, kComboH);

    Knob* row[] = { &rateKnob, &depthKnob, &baseFreqKnob, &feedbackKnob, &wetDryKnob };
    placeKnobRow(row, 5, x, y + kLabelH + kComboH + 12, w);
}

// ============================================================
//  ClipperPanel
// ============================================================

ClipperPanel::ClipperPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "CLIPPER", ParamID::clipBypass)
{
    makeKnob (ceilingKnob, ParamID::clipCeiling,  "CEILING");
    makeKnob (kneeKnob,    ParamID::clipKneeWidth, "KNEE");
    makeKnob (mixKnob,     ParamID::clipMix,       "MIX");
    makeKnob (biasKnob,    ParamID::clipBias,       "BIAS");
    makeKnob (emphFreqKnob,ParamID::clipEmphFreq,  "EMPH FREQ");
    makeKnob (emphGainKnob,ParamID::clipEmphGain,  "EMPH dB");

    makeCombo(modeCombo, ParamID::clipMode,
              "MODE", { "Hard", "Soft", "Sine", "Tape" });
    makeCombo(osCombo,   ParamID::clipOsQuality,
              "OVERSAMPLE", { "2x", "4x", "8x" });

    addAndMakeVisible(autoGainBtn);
    autoGainAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        proc.getAPVTS(), ParamID::clipAutoGain, autoGainBtn);
}

void ClipperPanel::resized()
{
    ModulePanel::resized();

    const auto area  = contentArea();
    const int  areaW = area.getWidth();

    const int comboH   = 28;
    const int labelH   = 16;
    const int comboW   = 140;
    const int comboY   = area.getY() + 8;

    modeCombo.label.setBounds(area.getX(), comboY, comboW, labelH);
    modeCombo.box.setBounds  (area.getX(), comboY + labelH, comboW, comboH);

    osCombo.label.setBounds(area.getX() + comboW + 16, comboY, comboW, labelH);
    osCombo.box.setBounds  (area.getX() + comboW + 16, comboY + labelH, comboW, comboH);

    const int knobRow  = comboY + labelH + comboH + 12;
    const int knobH    = 68;
    const int labelH2  = 14;
    const int knobNameH= labelH2;
    const int knobTotalH = knobNameH + knobH;

    const float knobW = (float)areaW / 6.0f;
    Knob* knobs[] = { &ceilingKnob, &kneeKnob, &mixKnob, &biasKnob, &emphFreqKnob, &emphGainKnob };
    for (int i = 0; i < 6; ++i)
    {
        const int kx = area.getX() + (int)(i * knobW);
        const int kw = (int)knobW - 4;
        knobs[i]->label.setBounds(kx, knobRow, kw, knobNameH);
        knobs[i]->slider.setBounds(kx, knobRow + knobNameH, kw, knobH);
    }

    const int toggleRow = knobRow + knobTotalH + 12;
    autoGainBtn.setBounds(area.getX(), toggleRow, 120, 22);
}

// ============================================================
//  SpatialPanel
// ============================================================

SpatialPanel::SpatialPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "SPATIAL", ParamID::spatialBypass)
{
    makeKnob(widthKnob,    ParamID::spatialWidth,    "WIDTH");
    makeKnob(panKnob,      ParamID::spatialPan,      "PAN");
    makeKnob(wetDryKnob,   ParamID::spatialWetDry,   "WET/DRY");
    makeKnob(rotationKnob, ParamID::spatialRotation, "ROTATION");
    makeKnob(midGainKnob,  ParamID::spatialMidGain,  "MID GAIN");
    makeKnob(sideGainKnob, ParamID::spatialSideGain, "SIDE GAIN");
    makeKnob(bassMonoKnob, ParamID::spatialBassMono, "BASS MONO");
    makeKnob(haasKnob,     ParamID::spatialHaasMs,   "HAAS (ms)");
}

void SpatialPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    Knob* row1[] = { &widthKnob, &panKnob, &wetDryKnob };
    placeKnobRow(row1, 3, x, y, w / 2);

    Knob* row2[] = { &rotationKnob, &midGainKnob, &sideGainKnob, &bassMonoKnob, &haasKnob };
    placeKnobRow(row2, 5, x, y + kRowH + 12, w);
}

// ============================================================
//  Factory
// ============================================================

std::unique_ptr<ModulePanel> createModulePanel(AlteredAudioProcessor& proc, int moduleIdx)
{
    switch (moduleIdx)
    {
        case  0: return std::make_unique<FilterPanel>         (proc);
        case  1: return std::make_unique<EQPanel>             (proc);
        case  2: return std::make_unique<GainPanel>           (proc);
        case  3: return std::make_unique<DelayPanel>          (proc);
        case  4: return std::make_unique<ReverbPanel>         (proc);
        case  5: return std::make_unique<WaveshaperPanel>     (proc);
        case  6: return std::make_unique<CompressorPanel>     (proc);
        case  7: return std::make_unique<ClipperPanel>        (proc);
        case  8: return std::make_unique<TransientShaperPanel>(proc);
        case  9: return std::make_unique<LimiterPanel>        (proc);
        case 10: return std::make_unique<GatePanel>           (proc);
        case 11: return std::make_unique<ChorusPanel>         (proc);
        case 12: return std::make_unique<FlangerPanel>        (proc);
        case 13: return std::make_unique<PhaserPanel>         (proc);
        case 14: return std::make_unique<SpatialPanel>        (proc);
        default: return std::make_unique<StubPanel>(proc, "UNKNOWN", ParamID::filterBypass);
    }
}
