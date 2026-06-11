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
    k.slider.setNumDecimalPlacesToDisplay(1);
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

    // Quadratic bezier midpoint smoothing — draws a smooth curve through all pts
    static void buildSmoothPath(juce::Path& path,
                                 const std::vector<juce::Point<float>>& pts)
    {
        if (pts.empty()) return;
        path.startNewSubPath(pts.front());
        const int n = (int)pts.size();
        for (int i = 0; i < n - 1; ++i)
        {
            const float mx = (pts[i].x + pts[(size_t)i + 1].x) * 0.5f;
            const float my = (pts[i].y + pts[(size_t)i + 1].y) * 0.5f;
            path.quadraticTo(pts[i].x, pts[i].y, mx, my);
        }
        path.lineTo(pts.back());
    }
}

// ============================================================
//  EQCurveDisplay
// ============================================================

juce::Colour EQCurveDisplay::bandColor(int i)
{
    static const juce::Colour kColors[] = {
        juce::Colour(0xFF4A9FE0),  // blue
        juce::Colour(0xFFE07840),  // orange
        juce::Colour(0xFF55C055),  // green
        juce::Colour(0xFFB060E0),  // purple
        juce::Colour(0xFFE0C040),  // yellow
        juce::Colour(0xFF40C8C8),  // cyan
        juce::Colour(0xFFE06080),  // pink
        juce::Colour(0xFF90C050),  // lime
    };
    return kColors[i % 8];
}

EQCurveDisplay::EQCurveDisplay(AlteredAudioProcessor& p) : proc(p)
{
    startTimerHz(30);
    setInterceptsMouseClicks(false, false);
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

        const auto* sl = apvts.getRawParameterValue(ParamID::eqSlope(n));

        BandData nd;
        nd.on    = en && *en > 0.5f;
        nd.type  = ty ? (int)ty->load() : 5;
        nd.freq  = fr ? fr->load() : 1000.0f;
        nd.q     = q  ? q->load()  : 0.7f;
        nd.gain  = ga ? ga->load() : 0.0f;
        nd.slope = sl ? (int)sl->load() : 1;

        if (nd.on != bands[i].on || nd.type != bands[i].type ||
            nd.freq != bands[i].freq || nd.q != bands[i].q ||
            nd.gain != bands[i].gain || nd.slope != bands[i].slope)
        {
            bands[i] = nd;
            changed  = true;
        }
    }
    if (changed)
    {
        repaint();
        if (onDataChanged) onDataChanged();
    }
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

// ---- selection ----

void EQCurveDisplay::setSelectedBand(int band)
{
    selectedBand = band;
    repaint();
}

// ---- response curve math ----

float EQCurveDisplay::computeSingleBandDb(int bi, float freqHz) const
{
    const auto& b = bands[bi];
    if (!b.on) return 0.f;

    // Exact transfer-function magnitude — identical math to the audio path,
    // so the curve shows precisely what you hear (including slope cascades
    // and response cramping near Nyquist).
    EQModule::BandParams p;
    p.enabled   = true;
    p.type      = static_cast<BiquadFilter::Type>(juce::jlimit(0, 7, b.type));
    p.frequency = b.freq;
    p.q         = b.q;
    p.gainDb    = b.gain;
    p.slope     = b.slope;

    const double sr = proc.getSampleRate() > 0.0 ? proc.getSampleRate() : 48000.0;
    return EQModule::bandResponseDb(p, freqHz, sr);
}

float EQCurveDisplay::computeResponseDb(float freqHz) const
{
    float total = 0.0f;
    for (int i = 0; i < EQModule::kMaxBands; ++i)
        total += computeSingleBandDb(i, freqHz);
    return juce::jlimit(-18.0f, 18.0f, total);
}

// ---- paint ----

void EQCurveDisplay::paint(juce::Graphics& g)
{
    const float w     = (float)getWidth();
    const float h     = (float)getHeight();
    const float plotW = w - padL - padR;
    const float plotH = h - padT - padB;
    const float logFMin = std::log10(FMIN), logFMax = std::log10(FMAX);

    g.setColour(AaColor::crtBg);
    g.fillRoundedRectangle(0.f, 0.f, w, h, 4.0f);

    // Selected-band frequency guide line
    if (selectedBand >= 0 && bands[selectedBand].on)
    {
        g.setColour(bandColor(selectedBand).withAlpha(0.20f));
        g.drawVerticalLine((int)xForF(bands[selectedBand].freq, w), padT, padT + plotH);
    }

    // Grid
    for (float f : { 20.f,50.f,100.f,200.f,500.f,1000.f,2000.f,5000.f,10000.f,20000.f })
    {
        g.setColour(AaColor::crtAmber.withAlpha(0.09f));
        g.drawVerticalLine((int)xForF(f, w), padT, padT + plotH);
    }
    for (float db : { -12.f,-6.f,0.f,6.f,12.f })
    {
        g.setColour(db == 0.f ? AaColor::crtAmber.withAlpha(0.28f)
                              : AaColor::crtAmber.withAlpha(0.09f));
        g.drawHorizontalLine((int)yForG(db, h), padL, padL + plotW);
    }

    // Per-band sub-curves (dim, in each band's color)
    for (int bi = 0; bi < EQModule::kMaxBands; ++bi)
    {
        if (!bands[bi].on) continue;
        const bool isSel = (bi == selectedBand);
        const float alpha = isSel ? 0.55f : 0.25f;

        std::vector<juce::Point<float>> pts;
        pts.reserve(201);
        for (int i = 0; i <= 200; ++i)
        {
            const float t  = (float)i / 200.f;
            const float f  = std::pow(10.f, logFMin + t * (logFMax - logFMin));
            const float db = computeSingleBandDb(bi, f);
            pts.push_back({ padL + t * plotW, yForG(juce::jlimit(-GMAX, GMAX, db), h) });
        }
        juce::Path subLine;
        buildSmoothPath(subLine, pts);
        g.setColour(bandColor(bi).withAlpha(alpha));
        g.strokePath(subLine, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved));
    }

    // Composite response curve (white/neutral, 2px) — bezier smoothed
    const float zeroY = yForG(0.f, h);
    {
        std::vector<juce::Point<float>> pts;
        pts.reserve(201);
        for (int i = 0; i <= 200; ++i)
        {
            const float t  = (float)i / 200.f;
            const float f  = std::pow(10.f, logFMin + t * (logFMax - logFMin));
            const float db = computeResponseDb(f);
            pts.push_back({ padL + t * plotW, yForG(db, h) });
        }
        juce::Path line, fill;
        buildSmoothPath(line, pts);
        buildSmoothPath(fill, pts);
        fill.lineTo(padL + plotW, zeroY);
        fill.lineTo(padL, zeroY);
        fill.closeSubPath();

        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.fillPath(fill);
        g.setColour(juce::Colours::white.withAlpha(0.80f));
        g.strokePath(line, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved));
    }

    // Freq axis labels
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
void GRMeter::setScale (float maxDb) { maxGrDb = maxDb; }

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
    const float w = (float)getWidth();

    // Header row: "GAIN REDUCTION" label + live dB value
    constexpr float hdrH = 14.f;
    const juce::Font mono { juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain };
    g.setFont(mono);
    g.setColour(AaColor::textSecond);
    g.drawText("GAIN REDUCTION", 0, 0, (int)w - 64, (int)hdrH,
               juce::Justification::centredLeft, false);

    if (-currentDb > 0.1f)
    {
        g.setColour(AaColor::catDynamics);
        g.drawText(juce::String(-currentDb, 1) + " dB",
                   (int)(w - 62), 0, 62, (int)hdrH,
                   juce::Justification::centredRight, false);
    }

    // Horizontal bar
    constexpr float barY = hdrH + 2.f, barH = 16.f;

    g.setColour(AaColor::crtBg);
    g.fillRoundedRectangle(0.f, barY, w, barH, 3.f);

    // dB scale gridlines inside track
    for (float db = 0.f; db <= maxGrDb; db += maxGrDb / 4.f)
    {
        const float gx = w - (db / maxGrDb) * w;
        g.setColour(AaColor::crtAmber.withAlpha(0.15f));
        g.fillRect(gx, barY + 2.f, 1.f, barH - 4.f);
    }

    // GR fill — grows from right edge leftward, amber→catDynamics gradient
    const float grFrac = juce::jlimit(0.f, 1.f, -currentDb / maxGrDb);
    if (grFrac > 0.001f)
    {
        const float fillW = grFrac * w;
        juce::ColourGradient grad(AaColor::crtAmber,    w - fillW, barY,
                                  AaColor::catDynamics,  w,         barY, false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(w - fillW, barY, fillW, barH, 3.f);
    }

    // Peak hold tick — 1.5px white line
    if (peakDb < -0.2f)
    {
        const float px = w - juce::jlimit(0.f, 1.f, -peakDb / maxGrDb) * w;
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.fillRect(px - 0.75f, barY, 1.5f, barH);
    }

    // dB scale labels below bar
    constexpr float scaleY = barY + barH + 2.f;
    g.setColour(AaColor::textSecond);
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::plain));
    const int steps = (int)(maxGrDb / 6.f);
    for (int s = 0; s <= steps; ++s)
    {
        const float db = s * 6.f;
        const float gx = w - (db / maxGrDb) * w;
        g.drawText(juce::String((int)db),
                   (int)(gx - 10), (int)scaleY, 20, 10,
                   juce::Justification::centred, false);
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
    makeCombo(typeCombo,  ParamID::filterType,  "TYPE",  types);
    makeCombo(slopeCombo, ParamID::filterSlope, "SLOPE", {"12","24"});
    makeKnob(freqKnob,   ParamID::filterFreq,   "FREQ");
    makeKnob(qKnob,      ParamID::filterQ,      "RESONANCE");
    makeKnob(gainKnob,   ParamID::filterGain,   "GAIN");
    makeKnob(driveKnob,  ParamID::filterDrive,  "DRIVE");
    makeKnob(mixKnob,    ParamID::filterMix,    "MIX");
    makeKnob(outputKnob, ParamID::filterOutput, "OUTPUT");
}

void FilterPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    typeCombo.label.setBounds (x,       y,           150, kLabelH);
    typeCombo.box.setBounds   (x,       y + kLabelH, 150, kComboH);
    slopeCombo.label.setBounds(x + 160, y,            80, kLabelH);
    slopeCombo.box.setBounds  (x + 160, y + kLabelH,  80, kComboH);

    Knob* row1[] = { &freqKnob, &qKnob, &gainKnob };
    placeKnobRow(row1, 3, x, y + kLabelH + kComboH + 12, w);

    Knob* row2[] = { &driveKnob, &mixKnob, &outputKnob };
    placeKnobRow(row2, 3, x, y + kLabelH + kComboH + 12 + kRowH + 12, w);
}

// ============================================================
//  EQPanel
// ============================================================

EQPanel::EQPanel(AlteredAudioProcessor& p)
    : ModulePanel(p, "EQ", ParamID::eqBypass), curveDisplay(p)
{
    addAndMakeVisible(curveDisplay);

    const juce::StringArray types { "LowPass","HighPass","BandPass","Notch",
                                    "AllPass","Peak","LowShelf","HighShelf" };
    for (int i = 0; i < kBands; ++i)
    {
        const int n = i + 1;

        // Band selector button (1–8) in the header row
        bandBtns[i].setButtonText(juce::String(n));
        bandBtns[i].setClickingTogglesState(false);
        bandBtns[i].onClick = [this, i]()
        {
            // If band is inactive, activate it first
            auto* en = proc.getAPVTS().getParameter(ParamID::eqEnabled(i + 1));
            if (en && en->getValue() < 0.5f)
                en->setValueNotifyingHost(1.0f);
            curveDisplay.setSelectedBand(i);
            showBand(i);
        };
        addAndMakeVisible(bandBtns[i]);

        bands[i].enableBtn.setButtonText("ACTIVE");
        bands[i].enableBtn.setClickingTogglesState(true);
        addAndMakeVisible(bands[i].enableBtn);
        bands[i].enableAttach =
            std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                proc.getAPVTS(), ParamID::eqEnabled(n), bands[i].enableBtn);

        makeCombo(bands[i].typeCombo,  ParamID::eqType(n),  "TYPE",  types);
        makeCombo(bands[i].slopeCombo, ParamID::eqSlope(n), "SLOPE", {"6","12","18","24","48"});
        bands[i].typeCombo.box.onChange = [this, i]() { showBand(i); };
        makeKnob (bands[i].freqKnob,   ParamID::eqFreq(n),  "FREQ");
        makeKnob (bands[i].qKnob,      ParamID::eqQ(n),     "Q");
        makeKnob (bands[i].gainKnob,   ParamID::eqGain(n),  "GAIN");
    }

    curveDisplay.onDataChanged = [this]() { updateBandButtons(); };
    showBand(0);
}

void EQPanel::showBand(int idx)
{
    auto* tyRaw = proc.getAPVTS().getRawParameterValue(ParamID::eqType(idx + 1));
    const int type = tyRaw ? (int)tyRaw->load() : 5;
    // Gain only affects Peak/LowShelf/HighShelf; slope only applies to LP/HP.
    const bool hasGain  = (type == 5 || type == 6 || type == 7);
    const bool hasSlope = (type == 0 || type == 1);

    for (int i = 0; i < kBands; ++i)
    {
        const bool vis = (i == idx);
        bands[i].enableBtn.setVisible(vis);
        bands[i].typeCombo.label.setVisible(vis);
        bands[i].typeCombo.box.setVisible(vis);
        bands[i].slopeCombo.label.setVisible(vis && hasSlope);
        bands[i].slopeCombo.box.setVisible(vis && hasSlope);
        bands[i].freqKnob.label.setVisible(vis);
        bands[i].freqKnob.slider.setVisible(vis);
        bands[i].qKnob.label.setVisible(vis);
        bands[i].qKnob.slider.setVisible(vis);
        bands[i].gainKnob.label.setVisible(vis && hasGain);
        bands[i].gainKnob.slider.setVisible(vis && hasGain);
    }
    updateBandButtons();
}

void EQPanel::updateBandButtons()
{
    const int sel = curveDisplay.getSelectedBand();
    for (int i = 0; i < kBands; ++i)
    {
        const auto* en = proc.getAPVTS().getRawParameterValue(ParamID::eqEnabled(i + 1));
        const bool active = en && *en > 0.5f;
        const bool isSel  = (i == sel);
        const juce::Colour col = EQCurveDisplay::bandColor(i);

        bandBtns[i].setColour(juce::TextButton::buttonColourId,
                               isSel  ? col :
                               active ? col.withAlpha(0.25f) : AaColor::surfaceAlt);
        bandBtns[i].setColour(juce::TextButton::textColourOffId,
                               isSel  ? juce::Colours::white :
                               active ? col : AaColor::textSecond);
    }
}

void EQPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int  ax = area.getX(), ay = area.getY(), aw = area.getWidth(), ah = area.getHeight();

    constexpr int btnRowH  = 28;
    constexpr int comboRowH = kLabelH + kComboH;
    constexpr int knobRowH  = kLabelH + kKnobH;
    constexpr int ctrlTotal = btnRowH + 6 + comboRowH + 6 + knobRowH;

    const int curveH = juce::jmax(120, ah - 8 - ctrlTotal);
    const int btnY   = ay + curveH + 8;
    const int ctrlY  = btnY + btnRowH + 6;

    curveDisplay.setBounds(ax, ay, aw, curveH);

    // Band selector row: 8 equal-width buttons
    const int btnW = aw / kBands;
    for (int i = 0; i < kBands; ++i)
        bandBtns[i].setBounds(ax + i * btnW, btnY, btnW, btnRowH);

    constexpr int typeW    = 115;
    constexpr int slopeW   = 80;
    constexpr int comboGap = 6;
    const int knobsX = ax + typeW + comboGap + slopeW + 10;
    const int knobsW = aw - (typeW + comboGap + slopeW + 10);
    const int comboY = ctrlY;
    const int knobY  = comboY + comboRowH + 6;

    for (int i = 0; i < kBands; ++i)
    {
        bands[i].enableBtn.setBounds(ax + aw - 80, btnY - btnRowH - 2, 80, btnRowH);

        bands[i].typeCombo.label.setBounds (ax,                    comboY,           typeW,  kLabelH);
        bands[i].typeCombo.box.setBounds   (ax,                    comboY + kLabelH, typeW,  kComboH);
        bands[i].slopeCombo.label.setBounds(ax + typeW + comboGap, comboY,           slopeW, kLabelH);
        bands[i].slopeCombo.box.setBounds  (ax + typeW + comboGap, comboY + kLabelH, slopeW, kComboH);

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
    makeKnob(timeLKnob,     ParamID::delayTime,      "TIME L (ms)");
    makeKnob(timeRKnob,     ParamID::delayTimeR,     "TIME R (ms)");
    makeKnob(feedbackKnob,  ParamID::delayFeedback,  "FEEDBACK");
    makeKnob(wetDryKnob,    ParamID::delayWetDry,    "WET/DRY");
    makeKnob(fbLPKnob,      ParamID::delayFbLPHz,    "TONE LP");
    makeKnob(fbHPKnob,      ParamID::delayFbHPHz,    "TONE HP");
    makeKnob(duckingKnob,   ParamID::delayDucking,   "DUCKING");
    makeKnob(diffusionKnob, ParamID::delayDiffusion, "DIFFUSION");
    makeKnob(wowKnob,       ParamID::delayWow,       "WOW");
    makeKnob(modRateKnob,   ParamID::delayModRate,   "MOD RATE");
    makeKnob(modDepthKnob,  ParamID::delayModDepth,  "MOD DEPTH");

    makeCombo(divLCombo, ParamID::delayDivL, "TIME L", DelayModule::syncDivisionNames());
    makeCombo(divRCombo, ParamID::delayDivR, "TIME R", DelayModule::syncDivisionNames());
    divLCombo.label.setJustificationType(juce::Justification::centred);
    divRCombo.label.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(syncBtn);
    syncBtn.setClickingTogglesState(true);
    syncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        proc.getAPVTS(), ParamID::delaySync, syncBtn);
    syncBtn.onClick = [this]() { updateTimeControls(); };

    addAndMakeVisible(pingPongBtn);
    pingPongAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        proc.getAPVTS(), ParamID::delayPingPong, pingPongBtn);

    updateTimeControls();
}

void DelayPanel::updateTimeControls()
{
    const bool sync = syncBtn.getToggleState();
    timeLKnob.label.setVisible(!sync);
    timeLKnob.slider.setVisible(!sync);
    timeRKnob.label.setVisible(!sync);
    timeRKnob.slider.setVisible(!sync);
    divLCombo.label.setVisible(sync);
    divLCombo.box.setVisible(sync);
    divRCombo.label.setVisible(sync);
    divRCombo.box.setVisible(sync);
}

void DelayPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int x = area.getX(), y = area.getY(), w = area.getWidth();

    Knob* row1[] = { &timeLKnob, &timeRKnob, &feedbackKnob, &wetDryKnob };
    placeKnobRow(row1, 4, x, y, w);

    // Synced mode: division combos occupy the two time-knob slots
    const int slotW = w / 4;
    divLCombo.label.setBounds(x,             y,                        slotW - 4, kLabelH);
    divLCombo.box.setBounds  (x,             y + kLabelH + 20,         slotW - 4, kComboH);
    divRCombo.label.setBounds(x + slotW,     y,                        slotW - 4, kLabelH);
    divRCombo.box.setBounds  (x + slotW,     y + kLabelH + 20,         slotW - 4, kComboH);

    Knob* row2[] = { &fbLPKnob, &fbHPKnob, &duckingKnob, &diffusionKnob };
    placeKnobRow(row2, 4, x, y + kRowH + 12, w);

    // Row 3: three knobs over 3/4 width; toggles stacked in the last quarter
    const int y3 = y + (kRowH + 12) * 2;
    Knob* row3[] = { &wowKnob, &modRateKnob, &modDepthKnob };
    placeKnobRow(row3, 3, x, y3, w * 3 / 4);

    const int tx = x + w * 3 / 4 + 8;
    syncBtn    .setBounds(tx, y3 + kLabelH + 6,  110, 22);
    pingPongBtn.setBounds(tx, y3 + kLabelH + 34, 110, 22);
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
    const auto area = contentArea();
    const int  x = area.getX(), y = area.getY(), w = area.getWidth();

    grMeter.setBounds(x, y, w, GRMeter::kHeight);
    const int knobY = y + GRMeter::kHeight + 8;

    Knob* row1[] = { &threshKnob, &ratioKnob, &attackKnob, &releaseKnob,
                     &kneeKnob, &makeupKnob, &rmsPeakKnob };
    placeKnobRow(row1, 7, x, knobY, w);

    Knob* row2[] = { &autoRelKnob, &satKnob, &scHPKnob, &scLPKnob,
                     &stereoLinkKnob, &mixKnob, &lookaheadKnob };
    placeKnobRow(row2, 7, x, knobY + kRowH + 12, w);
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
    grMeter.setScale(12.f);
    addAndMakeVisible(grMeter);

    makeKnob(ceilingKnob, ParamID::limCeiling, "CEILING (dB)");
    makeKnob(releaseKnob, ParamID::limRelease, "RELEASE (ms)");
}

void LimiterPanel::resized()
{
    ModulePanel::resized();
    const auto area = contentArea();
    const int  x = area.getX(), y = area.getY(), w = area.getWidth();

    grMeter.setBounds(x, y, w, GRMeter::kHeight);

    Knob* row[] = { &ceilingKnob, &releaseKnob };
    placeKnobRow(row, 2, x, y + GRMeter::kHeight + 8, w / 2);
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
