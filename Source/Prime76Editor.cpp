#include "Prime76Editor.h"
#include "ParameterIDs.h"
#include "BinaryData.h"

namespace { const juce::Colour kLed { 0xFFD99A33 }; }

// ------------------------------------------------------------
//  AmberGlyphs (Michroma amber sprite font)
// ------------------------------------------------------------
void AmberGlyphs::load()
{
    auto ld = [](const juce::String& n) -> juce::Image
    {
        int sz = 0;
        const char* d = BinaryData::getNamedResource(n.toRawUTF8(), sz);
        return d ? juce::ImageCache::getFromMemory(d, sz) : juce::Image();
    };
    for (int i = 0; i < 26; ++i)
        letters[i] = ld(juce::String::charToString((juce::juce_wchar) ('A' + i)) + "_png");
    const char* dn[13] = { "d0_png","d1_png","d2_png","d3_png","d4_png","d5_png","d6_png",
                           "d7_png","d8_png","d9_png","dot_png","plus_png","minus_png" };
    for (int i = 0; i < 13; ++i) digits[i] = ld(dn[i]);
    loaded = letters[0].isValid() && digits[0].isValid();
}

const juce::Image* AmberGlyphs::glyphFor(juce::juce_wchar c) const
{
    if (c >= 'A' && c <= 'Z') return &letters[c - 'A'];
    if (c >= 'a' && c <= 'z') return &letters[c - 'a'];
    if (c >= '0' && c <= '9') return &digits[c - '0'];
    if (c == '.') return &digits[10];
    if (c == '+') return &digits[11];
    if (c == '-') return &digits[12];
    return nullptr;
}

void AmberGlyphs::drawString(juce::Graphics& g, juce::Rectangle<float> area,
                             const juce::String& text, float heightFrac) const
{
    struct Tok { const juce::Image* im; float w; };   // w in source px (glyph width or space)
    std::vector<Tok> toks;
    for (int i = 0; i < text.length(); ++i)
    {
        const auto c = text[i];
        if (c == ' ') toks.push_back({ nullptr, 26.0f });
        else if (auto* im = glyphFor(c)) if (im->isValid()) toks.push_back({ im, (float) im->getWidth() });
    }
    if (toks.empty()) return;

    float totalW = kGapSrc * (float) (toks.size() - 1);
    for (auto& t : toks) totalW += t.w;

    float scale = area.getHeight() * heightFrac / kGlyphH;
    const float maxW = area.getWidth() - 4.0f;
    if (totalW * scale > maxW) scale = maxW / totalW;

    const float destH = kGlyphH * scale;
    float penX = area.getX() + (area.getWidth() - totalW * scale) * 0.5f;
    const float y = area.getY() + (area.getHeight() - destH) * 0.5f;
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    for (auto& t : toks)
    {
        const float dw = t.w * scale;
        if (t.im) g.drawImage(*t.im, penX, y, dw, destH, 0, 0, t.im->getWidth(), t.im->getHeight());
        penX += dw + kGapSrc * scale;
    }
}

float AmberGlyphs::textWidth(const juce::String& text, float glyphPx) const
{
    const float scale = glyphPx / kGlyphH;
    float total = 0.0f; int n = 0;
    for (int i = 0; i < text.length(); ++i)
    {
        const auto c = text[i];
        if (c == ' ') { total += 26.0f * scale; ++n; }
        else if (auto* im = glyphFor(c)) if (im->isValid()) { total += (float) im->getWidth() * scale; ++n; }
    }
    if (n > 1) total += kGapSrc * scale * (float) (n - 1);
    return total;
}

// ------------------------------------------------------------
//  Prime76LookAndFeel
// ------------------------------------------------------------
void Prime76LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                          float sliderPos, float startAngle, float endAngle,
                                          juce::Slider&)
{
    if (knobImage.isValid())
        g.drawImage(knobImage, x, y, w, h,
                    0, 0, knobImage.getWidth(), knobImage.getHeight());

    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const float rot   = angle - juce::MathConstants<float>::halfPi;  // dot baseline = 3 o'clock
    const float cx    = (float) x + (float) w * 0.5f;
    const float cy    = (float) y + (float) h * 0.5f;

    if (indicatorImage.isValid())
    {
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(rot, cx, cy));

        const float gx = cx + 0.223f * (float) w;
        const float gy = cy;
        auto blob = [&](float radius, float alpha)
        {
            juce::ColourGradient gr(kLed.withAlpha(alpha), gx, gy,
                                    kLed.withAlpha(0.0f), gx + radius, gy, true);
            g.setGradientFill(gr);
            g.fillEllipse(gx - radius, gy - radius, radius * 2.0f, radius * 2.0f);
        };
        blob(0.110f * (float) w, 0.34f);
        blob(0.055f * (float) w, 0.55f);

        g.drawImage(indicatorImage, x, y, w, h,
                    0, 0, indicatorImage.getWidth(), indicatorImage.getHeight());
        g.restoreState();
    }
}

// ------------------------------------------------------------
//  FooterStrip (rendered glyph font)
// ------------------------------------------------------------
void FooterStrip::paint(juce::Graphics& g)
{
    if (glyphs == nullptr || ! glyphs->ready()) return;
    const juce::Colour amber { 0xFFD99A33 };
    const float h = (float) getHeight(), w = (float) getWidth();
    const float sr = analysis ? juce::jmax(1.0f, analysis->sampleRate.load()) : 44100.0f;
    static const char* osN[3] = { "1X","4X","8X" };
    const auto* pOs = apvts ? apvts->getRawParameterValue("filter_oversampling") : nullptr;
    const int   osI = pOs ? juce::jlimit(0, 2, (int) pOs->load()) : 0;
    const float lat = proc ? (float) proc->getLatencySamples() / sr * 1000.0f : 0.0f;
    const float cpu = analysis ? analysis->cpuPct.load() : 0.0f;

    // procedural-style: dim small label + bright value per field
    g.setColour(amber);
    const float kFrac = 0.27f;          // uniform small text
    const float gh = h * kFrac, inGap = 7.0f;

    // SYSTEM stays at its left location
    g.setOpacity(0.45f);
    glyphs->drawString(g, { 14.0f, 0.0f, glyphs->textWidth("SYSTEM", gh) + 4.0f, h }, "SYSTEM", kFrac);

    // a label+value group, centred on cx (label tight with its value)
    auto grpAt = [&](float cx, const juce::String& lbl, const juce::String& val)
    {
        const float lw = lbl.isEmpty() ? 0.0f : glyphs->textWidth(lbl, gh);
        const float vw = val.isEmpty() ? 0.0f : glyphs->textWidth(val, gh);
        const float gap = (lbl.isNotEmpty() && val.isNotEmpty()) ? inGap : 0.0f;
        float x = cx - (lw + gap + vw) * 0.5f;
        if (lbl.isNotEmpty()) { g.setOpacity(0.45f); glyphs->drawString(g, { x, 0.0f, lw + 4.0f, h }, lbl, kFrac); x += lw + gap; }
        if (val.isNotEmpty()) { g.setOpacity(1.0f);  glyphs->drawString(g, { x, 0.0f, vw + 4.0f, h }, val, kFrac); }
    };
    struct Sec { juce::String lbl, val; };
    const Sec secs[6] = {
        { "SAMPLE RATE", juce::String(sr / 1000.0f, 1) + " KHZ" },
        { "OVERSAMP",    juce::String(osN[osI]) },
        { "LATENCY",     juce::String(lat, 2) + " MS" },
        { "CPU",         juce::String(juce::roundToInt(cpu)) },
        { "SIGNAL PATH", "STEREO" },
        { "",            "V1.0.9" } };
    for (int i = 0; i < 6; ++i)
        grpAt(w * (0.20f + 0.75f * (float) i / 5.0f), secs[i].lbl, secs[i].val);   // dispersed evenly
    g.setOpacity(1.0f);
}

// ------------------------------------------------------------
//  Prime76Editor
// ------------------------------------------------------------
Prime76Editor::Prime76Editor(juce::AudioProcessor& p,
                             juce::AudioProcessorValueTreeState& s,
                             FilterAnalysisSource& an)
    : juce::AudioProcessorEditor(p), apvts(s), analysis(an),
      display(s, an), lfoScope(s, an)
{
    faceplate          = juce::ImageCache::getFromMemory(BinaryData::prime_faceplate_png,
                                                         BinaryData::prime_faceplate_pngSize);
    lnf.knobImage      = juce::ImageCache::getFromMemory(BinaryData::prime_knob_png,
                                                         BinaryData::prime_knob_pngSize);
    lnf.indicatorImage = juce::ImageCache::getFromMemory(BinaryData::prime_indicator_png,
                                                         BinaryData::prime_indicator_pngSize);

    // live screens (reuse the Light editor components)
    addAndMakeVisible(display);
    addAndMakeVisible(lfoScope);  lfoScope.setLfoIndex(0);
    addAndMakeVisible(envScope);

    // knobs
    addKnob(freq,     ParamID::filterFreq,    247, 507, 112);
    addKnob(res,      ParamID::filterQ,       438, 507, 112);
    addKnob(drive,    ParamID::filterDrive,   629, 507, 112);
    addKnob(mix,      ParamID::filterMix,     820, 507, 112);
    addKnob(out,      ParamID::filterGain,   1010, 507, 112);  // hero slot -> GAIN (swapped)
    addKnob(gain,     ParamID::filterOutput, 1150, 529,  64);  // small slot -> OUT  (swapped)
    addKnob(amount,   ParamID::fltModAmount,  247, 736,  64);
    addKnob(lfoRate,  ParamID::fltLfoARate,   628, 736,  64);
    addKnob(lfoDepth, ParamID::fltLfoADepth,  725, 736,  64);
    addKnob(lfoPhase, ParamID::fltLfoAPhase,  820, 736,  64);
    addKnob(envAtk,   ParamID::fltEnvAttack, 1163, 737,  64);
    addKnob(envRel,   ParamID::fltEnvRelease,1239, 737,  64);
    addKnob(envSens,  ParamID::fltEnvSens,   1315, 737,  64);

    // option LEDs: filter type (6) / slope (3) / mode (2)
    auto row = [](int x, int y, int w) { return juce::Rectangle<int>(x, y - 14, w, 28); };
    // filterType has 8 choices (LowPass,HighPass,BandPass,Notch,AllPass,Peak,LowShelf,HighShelf);
    // the 6 plate buttons LP/HP/BP/NOTCH/PEAK/SHELF map to indices {0,1,2,3,5,6} (Light editor).
    // SHELF (last) cycles LowShelf(6) <-> HighShelf(7) on repeated clicks
    auto* pType = apvts.getParameter(ParamID::filterType);
    if (pType)
        optionLeds.addGroup(pType, 8, { {0},{1},{2},{3},{5},{6,7} },
            { {49,471},{49,500},{49,528},{49,556},{49,584},{49,612} },
            { row(33,471,104),row(33,500,104),row(33,528,104),
              row(33,556,104),row(33,584,104),row(33,612,104) });
    // SLOPE only affects LP(0)/HP(1): dim its LEDs for any other type (gate filterType in [0,1])
    if (auto* pSlope = apvts.getParameter(ParamID::filterSlope))
        optionLeds.addGroup(pSlope, 3, { {0},{1},{2} },
            { {1232,472},{1232,500},{1232,528} },
            { row(1211,472,139),row(1211,500,139),row(1211,528,139) },
            false, pType, 8, 0, 1);
    if (auto* pMode = apvts.getParameter(ParamID::filterMode))
        optionLeds.addGroup(pMode, 2, { {0},{1} },
            { {1232,584},{1232,611} },
            { row(1211,584,139),row(1211,611,139) });
    addAndMakeVisible(optionLeds);

    // readout screens under the hero knobs (name when idle, value while turning)
    glyphs.load();
    const char* rn[12] = { "FREQ","RES","DRIVE","MIX","GAIN",
                           "AMOUNT","RATE","DEPTH","PHASE","ATTACK","RELEASE","SENS" };
    const juce::String rp[12] = {
        ParamID::filterFreq, ParamID::filterQ, ParamID::filterDrive, ParamID::filterMix,
        ParamID::filterGain,                                            // hero slot now GAIN
        ParamID::fltModAmount, ParamID::fltLfoARate, ParamID::fltLfoADepth, ParamID::fltLfoAPhase,
        ParamID::fltEnvAttack, ParamID::fltEnvRelease, ParamID::fltEnvSens };
    auto i0 = [](float v){ return juce::String(juce::roundToInt(v)); };
    auto d1 = [](float v){ return juce::String(v, 1); };
    auto pc = [](float v){ return juce::String(juce::roundToInt(v * 100.0f)); };
    std::function<juce::String(float)> rf[12] = {
        i0, d1, d1, pc, d1,             // FREQ Hz · RES · DRIVE · MIX % · GAIN dB
        pc, d1, pc, i0, d1, i0, d1 };   // AMOUNT % · RATE Hz · DEPTH % · PHASE° · ATK ms · REL ms · SENS
    for (int i = 0; i < 12; ++i)
    {
        auto& r = readouts[i];
        r.glyphs = &glyphs; r.name = rn[i]; r.fmt = rf[i];
        r.param = apvts.getParameter(rp[i]);
        r.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(r);
    }
    // RATE readout (index 6) is clickable: toggles LFO A tempo-sync, shows "SYNC"
    readouts[6].syncParam = apvts.getParameter(ParamID::fltLfoASync);
    readouts[6].setInterceptsMouseClicks(true, false);

    // static plate labels (filter type / slope / mode) as movable components
    const char* ln[11] = { "LP","HP","BP","NOTCH","PEAK","SHELF","12","24","48","ANALOG","CLEAN" };
    for (int i = 0; i < 11; ++i)
    { labels[i].glyphs = &glyphs; labels[i].text = ln[i]; addAndMakeVisible(labels[i]); }
    labels[9].heightFrac = labels[10].heightFrac = 0.34f;   // ANALOG/CLEAN -15%

    // modulation: movable TEXT labels + fixed LED/click via OptionLeds (like filter type)
    auto* pSrc = apvts.getParameter(ParamID::fltModSource);   // OFF,LFO A,LFO B,Env (4)
    auto* pDst = apvts.getParameter(ParamID::fltModDest);     // FREQ,RES,DRIVE (3)
    const char* mt[6] = { "LFO A","LFO B","EF","FREQ","RESO","DRIVE" };
    for (int i = 0; i < 6; ++i)
    { modLabels[i].glyphs = &glyphs; modLabels[i].text = mt[i]; addAndMakeVisible(modLabels[i]); }

    peakReadout.glyphs = &glyphs; peakReadout.analysis = &analysis;
    addAndMakeVisible(peakReadout);

    footer.glyphs = &glyphs; footer.analysis = &analysis; footer.apvts = &apvts; footer.proc = &p;
    addAndMakeVisible(footer);
    addAndMakeVisible(waveSelect);   // param set in rebindLfo
    // source LEDs: LFO A/B/EF -> idx 1/2/3, click-active turns OFF
    if (pSrc) optionLeds.addGroup(pSrc, 4, { {1},{2},{3} },
        { {49,671},{49,699},{49,727} },
        { row(33,671,104), row(33,699,104), row(33,727,104) }, true);
    // dest LEDs: FREQ/RESO/DRIVE -> idx 0/1/2, lit only while source != OFF
    if (pDst) optionLeds.addGroup(pDst, 3, { {0},{1},{2} },
        { {49,760},{49,788},{49,816} },
        { row(33,760,104), row(33,788,104), row(33,816,104) }, false, pSrc, 4);

    buildHeader();
    applyDefaultPositions();

    // in-app layout editor + persistence
    layoutFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile("AlteredAudio").getChildFile("Filter 76").getChildFile("prime_layout.json");
    const juce::String lid[11] = { "LP","HP","BP","NOTCH","PEAK","SHELF",
                                   "SLOPE_12","SLOPE_24","SLOPE_48","MODE_ANALOG","MODE_CLEAN" };
    for (int i = 0; i < 11; ++i) layoutEditor.addItem(&labels[i], lid[i], &labels[i].text);
    const juce::String rid[12] = { "RD_FREQ","RD_RES","RD_DRIVE","RD_MIX","RD_GAIN",
        "RD_AMOUNT","RD_RATE","RD_DEPTH","RD_PHASE","RD_ATTACK","RD_RELEASE","RD_SENS" };
    for (int i = 0; i < 12; ++i) layoutEditor.addItem(&readouts[i], rid[i]);
    const juce::String mid[6] = { "MOD_LFOA","MOD_LFOB","MOD_EF","DST_FREQ","DST_RES","DST_DRIVE" };
    for (int i = 0; i < 6; ++i) layoutEditor.addItem(&modLabels[i], mid[i], &modLabels[i].text);
    layoutEditor.addItem(&peakReadout, "PEAK_READOUT");
    layoutEditor.addItem(&waveSelect, "WAVE_SELECT");
    layoutEditor.onChanged = [this] { layoutEditor.saveLayout(layoutFile); };
    layoutEditor.loadLayout(layoutFile);
    fitModLabels();   // constrain mod label boxes to their text (keeps centre)
    layoutEditor.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(layoutEditor);          // top z; transparent until edit mode
    addMouseListener(this, true);             // catch right-clicks anywhere

    startTimerHz(30);
    setSize(kW, kH);
}

Prime76Editor::~Prime76Editor()
{
    stopTimer();
    for (auto* k : all) k->slider.setLookAndFeel(nullptr);
    for (auto* b : { &hPrev, &hNext, &hSave, &hA, &hB, &hPwr, &hOS }) b->setLookAndFeel(nullptr);
    hName.setLookAndFeel(nullptr); hMix.setLookAndFeel(nullptr);
}

void Prime76Editor::addKnob(Knob& k, const juce::String& paramId, int cx, int cy, int diam)
{
    k.cx = cx; k.cy = cy; k.diam = diam;
    k.slider.setLookAndFeel(&lnf);
    addAndMakeVisible(k.slider);
    k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                   apvts, paramId, k.slider);
}

void Prime76Editor::timerCallback()
{
    display.refresh();
    lfoScope.repaint();
    envScope.push(analysis.envValue.load());

    // LFO ENGINE follows the mod source (A/B) and that LFO's sync state
    const auto* pSrc = apvts.getRawParameterValue(ParamID::fltModSource);
    const int  wantLfo = (pSrc && (int) pSrc->load() == 2) ? 1 : 0;
    const auto* pLS = apvts.getRawParameterValue(wantLfo == 1 ? ParamID::fltLfoBSync
                                                              : ParamID::fltLfoASync);
    const bool wantSync = (pLS && pLS->load() > 0.5f);
    if (wantLfo != boundLfo || wantSync != boundLfoSync) rebindLfo(wantLfo, wantSync);

    for (auto& r : readouts) r.tick();
    peakReadout.tick();
    footer.tick();
    waveSelect.repaint();
    updateHeader();
}

void Prime76Editor::applyDefaultPositions()
{
    // baked from the user's saved prime_layout.json (2026-06-16)
    const int ly[6] = { 461, 490, 518, 546, 574, 601 };       // LP..SHELF
    for (int i = 0; i < 6; ++i) labels[i].setBounds(64, ly[i], 72, 22);
    const int sy[3] = { 461, 489, 517 };                      // 12/24/48
    for (int i = 0; i < 3; ++i) labels[6 + i].setBounds(1252, sy[i], 84, 22);
    labels[9].setBounds(1250, 574, 90, 22);                   // ANALOG
    labels[10].setBounds(1250, 601, 90, 22);                  // CLEAN

    const int rx[5] = { 216, 406, 596, 787, 978 };            // readouts under hero knobs
    for (int i = 0; i < 5; ++i) readouts[i].setBounds(rx[i], 600, 63, 22);
    const int bx[7] = { 215, 597, 694, 788, 1131, 1207, 1283 }; // AMOUNT/RATE/DEPTH/PHASE/ATK/REL/SENS
    for (int i = 0; i < 7; ++i) readouts[5 + i].setBounds(bx[i], 795, 63, 22);

    const int mx[6]  = { 49, 48, 48, 48, 48, 48 };            // mod labels (text only)
    const int myy[6] = { 661, 689, 717, 750, 778, 805 };
    for (int i = 0; i < 6; ++i) modLabels[i].setBounds(mx[i], myy[i], 104, 22);

    peakReadout.setBounds(1232, 93, 102, 22);                 // screen above the meters
    footer.setBounds(30, 851, 1321, 30);                      // bottom info line
    waveSelect.setBounds(383, 807, 113, 16);                  // 4 LFO wave screens below the scope

    // header line controls (strip at y26..56)
    const int hy = 30, hh = 22;
    hPrev.setBounds(486, hy, 22, hh);
    hName.setBounds(510, hy, 180, hh);
    hNext.setBounds(692, hy, 22, hh);
    hSave.setBounds(720, hy, 50, hh);
    hOS  .setBounds(1000, hy, 44, hh);
    hMix .setBounds(1052, hy, 72, hh);
    hA   .setBounds(1150, hy, 26, hh);
    hB   .setBounds(1178, hy, 26, hh);
    hPwr .setBounds(1300, hy, 44, hh);
}

void Prime76Editor::buildHeader()
{
    headerLnF.glyphs = &glyphs;
    for (auto* b : { &hPrev, &hNext, &hSave, &hA, &hB, &hPwr, &hOS })
    { b->setLookAndFeel(&headerLnF); addAndMakeVisible(*b); }
    hA.setClickingTogglesState(true);
    hB.setClickingTogglesState(true);
    hPwr.setClickingTogglesState(true);

    hName.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF120D07));
    hName.setLookAndFeel(&headerLnF);
    addAndMakeVisible(hName);
    hMix.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    hMix.setLookAndFeel(&headerLnF);
    addAndMakeVisible(hMix);

    initState = apvts.copyState();
    abSlot[0] = apvts.copyState();
    refreshPresetList();
    gotoPreset(0);

    hPrev.onClick = [this]{ gotoPreset(presetIdx - 1); };
    hNext.onClick = [this]{ gotoPreset(presetIdx + 1); };
    hSave.onClick = [this]{ saveCurrentPreset(); };
    hA.onClick    = [this]{ setAB(0); };
    hB.onClick    = [this]{ setAB(1); };
    hA.setToggleState(true, juce::dontSendNotification);
    hPwr.onClick  = [this]{
        if (auto* p = apvts.getParameter(ParamID::filterBypass))
            p->setValueNotifyingHost(hPwr.getToggleState() ? 0.0f : 1.0f); };  // on = powered
    if (auto* pb = apvts.getRawParameterValue(ParamID::filterBypass))
        hPwr.setToggleState(*pb < 0.5f, juce::dontSendNotification);
    hOS.onClick   = [this]{
        if (auto* p = apvts.getParameter(ParamID::filterOversamp))
        { const int cur = juce::roundToInt(p->getValue() * 2.0f);
          p->setValueNotifyingHost((float)((cur + 1) % 3) / 2.0f); } };
}

void Prime76Editor::refreshPresetList()
{
    presets.clear();
    presets.add("INIT");
    presets.addArray(PresetManager::userNames());
}

void Prime76Editor::gotoPreset(int idx)
{
    if (presets.isEmpty()) return;
    presetIdx = (idx % presets.size() + presets.size()) % presets.size();
    const auto nm = presets[presetIdx];
    hName.setText(nm, juce::dontSendNotification);
    if (presetIdx == 0) apvts.replaceState(initState.createCopy());
    else { auto vt = PresetManager::load(nm); if (vt.isValid()) apvts.replaceState(vt); }
}

void Prime76Editor::saveCurrentPreset()
{
    auto* aw = new juce::AlertWindow("Save preset", "Name:", juce::MessageBoxIconType::NoIcon);
    aw->addTextEditor("n", "");
    aw->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    aw->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, aw](int r)
        {
            if (r == 1)
            {
                const auto nm = aw->getTextEditorContents("n").trim().toUpperCase();
                if (nm.isNotEmpty() && PresetManager::save(nm, apvts))
                { refreshPresetList(); presetIdx = presets.indexOf(nm);
                  hName.setText(nm, juce::dontSendNotification); }
            }
            delete aw;
        }), false);
}

void Prime76Editor::setAB(int slot)
{
    abSlot[abActive] = apvts.copyState();           // stash current into the active slot
    abActive = slot;
    if (abSlot[slot].isValid()) apvts.replaceState(abSlot[slot].createCopy());
    hA.setToggleState(slot == 0, juce::dontSendNotification);
    hB.setToggleState(slot == 1, juce::dontSendNotification);
}

void Prime76Editor::updateHeader()
{
    static const char* osN[3] = { "1x", "4x", "8x" };
    if (auto* pOs = apvts.getRawParameterValue(ParamID::filterOversamp))
        hOS.setButtonText(osN[juce::jlimit(0, 2, (int) pOs->load())]);
    if (auto* pMix = apvts.getRawParameterValue(ParamID::filterMix))
        hMix.setText("MIX " + juce::String(juce::roundToInt(pMix->load() * 100.0f)) + "%",
                     juce::dontSendNotification);
    if (auto* pb = apvts.getRawParameterValue(ParamID::filterBypass))
        hPwr.setToggleState(*pb < 0.5f, juce::dontSendNotification);
}

void Prime76Editor::bindKnob(Knob& k, const juce::String& pid)
{
    k.attach.reset();
    k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                   apvts, pid, k.slider);
}

void Prime76Editor::rebindLfo(int idx, bool sync)
{
    boundLfo = idx; boundLfoSync = sync;
    const bool b = (idx == 1);
    bindKnob(lfoRate,  sync ? (b ? ParamID::fltLfoBDiv  : ParamID::fltLfoADiv)
                            : (b ? ParamID::fltLfoBRate : ParamID::fltLfoARate));
    bindKnob(lfoDepth, b ? ParamID::fltLfoBDepth : ParamID::fltLfoADepth);
    bindKnob(lfoPhase, b ? ParamID::fltLfoBPhase : ParamID::fltLfoAPhase);

    auto& rd = readouts[6];   // RATE
    rd.syncParam = apvts.getParameter(b ? ParamID::fltLfoBSync : ParamID::fltLfoASync);
    if (sync)
    {
        rd.param = apvts.getParameter(b ? ParamID::fltLfoBDiv : ParamID::fltLfoADiv);
        rd.fmt = [](float v){
            // show just the denominator (e.g. "1/4" -> "4", "1/8T" -> "8T")
            return aurora::divNames()[juce::jlimit(0, 13, (int) std::round(v))]
                       .fromFirstOccurrenceOf("/", false, false); };
    }
    else
    {
        rd.param = apvts.getParameter(b ? ParamID::fltLfoBRate : ParamID::fltLfoARate);
        rd.fmt = [](float v){ return juce::String(v, 1); };
    }
    readouts[7].param = apvts.getParameter(b ? ParamID::fltLfoBDepth : ParamID::fltLfoADepth);
    readouts[8].param = apvts.getParameter(b ? ParamID::fltLfoBPhase : ParamID::fltLfoAPhase);
    waveSelect.param  = apvts.getParameter(b ? ParamID::fltLfoBWave  : ParamID::fltLfoAWave);
    waveSelect.repaint();
    lfoScope.setLfoIndex(idx);
}

void Prime76Editor::fitModLabels()
{
    if (! glyphs.ready()) return;
    for (auto& m : modLabels)
    {
        const float gh = (float) m.getHeight() * m.heightFrac;
        const int   w  = juce::roundToInt(glyphs.textWidth(m.text, gh)) + 8;   // text + small pad
        const int   cx = m.getX() + m.getWidth() / 2;
        m.setBounds(cx - w / 2, m.getY(), w, m.getHeight());
    }
}

void Prime76Editor::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu() && ! layoutEditor.editMode)
        layoutEditor.showMenu();
}

void Prime76Editor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF12100D));
    if (! faceplate.isValid()) return;

    // Draw only the plate's opaque region 1:1 into the (narrower) window, so the
    // window ends exactly at the plate's right edge — no stretch, knobs aligned
    // (same 0.5 scale as the layout coords). The render's transparent right
    // margin is simply not part of the window.
    constexpr int kOpaqueW = 2810;            // @2x plate opaque width (= kW*2)
    g.drawImage(faceplate,
                0, 0, getWidth(), getHeight(),
                0, 0, juce::jmin(kOpaqueW, faceplate.getWidth()), faceplate.getHeight());
}

void Prime76Editor::resized()
{
    const float ratio = lnf.knobImage.isValid()
                        ? (lnf.knobImage.getWidth() * 0.5f) / 112.0f : 1.156f;
    for (auto* k : all)
    {
        const int box = juce::roundToInt(k->diam * ratio);
        k->slider.setBounds(k->cx - box / 2, k->cy - box / 2, box, box);
    }

    display .setBounds( 58,  97, 1118, 304);   // Display_Glass
    lfoScope.setBounds(337, 716,  204,  79);   // LFO_Scope_Glass
    envScope.setBounds(887, 716,  204,  79);   // Env_Scope_Glass
    optionLeds.setBounds(getLocalBounds());
    layoutEditor.setBounds(getLocalBounds());
    // NB: label/readout positions come from applyDefaultPositions() + saved layout,
    //     so resized() must not reset them.
}
