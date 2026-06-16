#pragma once
#include <JuceHeader.h>
#include "AuroraFilterEditor.h"   // FilterAnalysisSource, ResponseDisplay, LfoScope, EnvScope

// ============================================================
//  Prime76LookAndFeel — sprite knob (see .cpp).
// ============================================================
class Prime76LookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Image knobImage, indicatorImage;
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
};

// ============================================================
//  OptionLeds — transparent overlay that lights the selected
//  option LED for choice params (filter type / slope / mode) and
//  sets the param on click. hitTest() is solid only over the
//  clickable rows, so it never blocks the knobs underneath.
// ============================================================
class OptionLeds : public juce::Component, private juce::Timer
{
public:
    OptionLeds() { setInterceptsMouseClicks(true, false); startTimerHz(20); }
    ~OptionLeds() override { stopTimer(); }

    // idx[k] = the param CHOICE INDEX for option k (may be non-contiguous);
    // numChoices = the param's total choice count (for normalisation).
    // each option's values: 1 index normally, or several to cycle (e.g. SHELF = {6,7}).
    void addGroup(juce::RangedAudioParameter* p, int numChoices,
                  std::vector<std::vector<int>> idx,
                  std::vector<juce::Point<float>> leds,
                  std::vector<juce::Rectangle<int>> rows,
                  bool toggleOff = false,
                  juce::RangedAudioParameter* gate = nullptr, int gateChoices = 0,
                  int gateMin = 1, int gateMax = 99)   // group active only if gate idx in [min,max]
    {
        groups.push_back({ p, numChoices, std::move(idx), std::move(leds), std::move(rows),
                           toggleOff, gate, gateChoices, gateMin, gateMax });
    }

    bool hitTest(int x, int y) override
    {
        for (auto& g : groups) for (auto& r : g.rows) if (r.contains(x, y)) return true;
        return false;
    }
    void mouseDown(const juce::MouseEvent& e) override
    {
        for (auto& g : groups)
            for (size_t i = 0; i < g.rows.size(); ++i)
                if (g.rows[i].contains(e.getPosition()) && g.p != nullptr)
                {
                    const int cur = juce::roundToInt(g.p->getValue() * (float) (g.nc - 1));
                    const auto& vals = g.idx[i];
                    int pos = -1;
                    for (int k = 0; k < (int) vals.size(); ++k) if (vals[k] == cur) pos = k;
                    const int tgt = (pos < 0) ? vals[0]
                                  : (g.toggleOff && (int) vals.size() == 1) ? 0
                                  : vals[(pos + 1) % (int) vals.size()];
                    g.p->beginChangeGesture();
                    g.p->setValueNotifyingHost(g.nc > 1 ? (float) tgt / (float) (g.nc - 1) : 0.0f);
                    g.p->endChangeGesture();
                    return;
                }
    }
    void paint(juce::Graphics& g) override
    {
        const juce::Colour led { 0xFFD99A33 };
        for (auto& grp : groups)
        {
            if (grp.p == nullptr) continue;
            const int cur = juce::roundToInt(grp.p->getValue() * (float) (grp.nc - 1));
            bool gateOff = false;
            if (grp.gate != nullptr)
            {
                const int gi = juce::roundToInt(grp.gate->getValue() * (float) (grp.gateNc - 1));
                gateOff = (gi < grp.gateMin || gi > grp.gateMax);
            }
            for (int i = 0; i < (int) grp.leds.size(); ++i)
            {
                const auto p = grp.leds[i];
                bool sel = false;
                for (int v : grp.idx[i]) if (v == cur) sel = true;
                if (sel && ! gateOff)
                {
                    juce::ColourGradient gr(led.withAlpha(0.55f), p.x, p.y,
                                            led.withAlpha(0.0f), p.x + 9.0f, p.y, true);
                    g.setGradientFill(gr);
                    g.fillEllipse(p.x - 9.0f, p.y - 9.0f, 18.0f, 18.0f);
                    g.setColour(led);
                }
                else
                    g.setColour(juce::Colour(0xFF2E2820));
                g.fillEllipse(p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f);
            }
        }
    }

private:
    struct Group { juce::RangedAudioParameter* p; int nc; std::vector<std::vector<int>> idx;
                   std::vector<juce::Point<float>> leds; std::vector<juce::Rectangle<int>> rows;
                   bool toggleOff; juce::RangedAudioParameter* gate; int gateNc; int gateMin; int gateMax; };
    std::vector<Group> groups;
    void timerCallback() override { repaint(); }
};

// ============================================================
//  AmberGlyphs — amber sprite font (Michroma, rendered in Blender):
//  A-Z + 0-9 . + - , cropped to a shared baseline band. drawString
//  lays glyphs out proportionally, centred, height-fitted to an area.
// ============================================================
class AmberGlyphs
{
public:
    void load();
    void drawString(juce::Graphics&, juce::Rectangle<float> area,
                    const juce::String& text, float heightFrac) const;
    float textWidth(const juce::String& text, float glyphPx) const;   // rendered width at a glyph height
    bool ready() const { return loaded; }
private:
    const juce::Image* glyphFor(juce::juce_wchar c) const;
    juce::Image letters[26];
    juce::Image digits[13];   // 0-9, '.', '+', '-'
    bool loaded = false;
    static constexpr float kGlyphH = 69.0f, kGapSrc = 6.0f;
};

// ============================================================
//  GlyphLnF — header buttons/labels rendered with the sprite font
//  (chevrons drawn for < / > since the font has no slash/brackets).
// ============================================================
class GlyphLnF : public juce::LookAndFeel_V4
{
public:
    const AmberGlyphs* glyphs = nullptr;
    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&,
                              bool over, bool down) override
    {
        const juce::Colour amber { 0xFFD99A33 };
        auto r = b.getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(juce::Colour(0xFF18120A)); g.fillRoundedRectangle(r, 3.0f);
        const bool on = b.getToggleState() || down;
        g.setColour(amber.withAlpha(on ? 0.95f : over ? 0.5f : 0.30f));
        g.drawRoundedRectangle(r, 3.0f, on ? 1.6f : 1.0f);
    }
    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        const juce::Colour amber { 0xFFD99A33 };
        const auto t = b.getButtonText();
        auto r = b.getLocalBounds().toFloat();
        if (t == "<" || t == ">")
        {
            const float cx = r.getCentreX(), cy = r.getCentreY(), s = 3.5f;
            juce::Path p;
            if (t == "<") { p.startNewSubPath(cx + s, cy - s); p.lineTo(cx - s, cy); p.lineTo(cx + s, cy + s); }
            else          { p.startNewSubPath(cx - s, cy - s); p.lineTo(cx + s, cy); p.lineTo(cx - s, cy + s); }
            g.setColour(amber);
            g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            return;
        }
        if (glyphs && glyphs->ready())
            glyphs->drawString(g, r.reduced(4.0f, 3.0f), t, 0.5f);
    }
    void drawLabel(juce::Graphics& g, juce::Label& l) override
    {
        g.fillAll(l.findColour(juce::Label::backgroundColourId));
        if (glyphs && glyphs->ready())
            glyphs->drawString(g, l.getLocalBounds().toFloat().reduced(3.0f, 2.0f), l.getText(), 0.5f);
    }
};

// ============================================================
//  PeakReadout — amber dB peak numbers on the screen above the meters.
//  Per-channel peak HOLD that resets only after the signal goes silent.
// ============================================================
class PeakReadout : public juce::Component
{
public:
    const AmberGlyphs* glyphs = nullptr;
    FilterAnalysisSource* analysis = nullptr;
    void tick()
    {
        if (analysis == nullptr) return;
        upd(heldL, silL, analysis->peakL.exchange(0.0f));
        upd(heldR, silR, analysis->peakR.exchange(0.0f));
        repaint();
    }
    void paint(juce::Graphics& g) override
    {
        if (glyphs == nullptr || ! glyphs->ready()) return;
        auto b = getLocalBounds().toFloat();
        auto lh = b.removeFromLeft(b.getWidth() * 0.5f);
        glyphs->drawString(g, lh, fmt(heldL), 0.5f);
        glyphs->drawString(g, b,  fmt(heldR), 0.5f);
    }
private:
    float heldL = -99.0f, heldR = -99.0f; int silL = 0, silR = 0;
    static void upd(float& held, int& sil, float lin)
    {
        if (lin > 1.0e-5f)
        { const float db = juce::Decibels::gainToDecibels(lin); if (db > held) held = db; sil = 0; }
        else if (++sil > 45) held = -99.0f;   // ~1.5s of silence -> clear the hold
    }
    static juce::String fmt(float db) { return db <= -99.0f ? juce::String("-") : juce::String(db, 1); }
};

// ============================================================
//  ReadoutScreen — the small dark slot under a hero knob. Shows the
//  function name when idle; switches to the live value while the
//  param is changing, then reverts after a short hold.
// ============================================================
class ReadoutScreen : public juce::Component
{
public:
    const AmberGlyphs* glyphs = nullptr;
    juce::RangedAudioParameter* param = nullptr;
    juce::String name;
    std::function<juce::String(float)> fmt;   // real value -> numeric string
    juce::RangedAudioParameter* syncParam = nullptr;  // optional: click toggles tempo-sync

    bool synced() const { return syncParam && syncParam->getValue() > 0.5f; }

    void tick()
    {
        const bool s = synced();
        if (s != lastSync) { lastSync = s; repaint(); }
        if (param == nullptr) return;
        const float v = param->convertFrom0to1(param->getValue());
        if (first) { last = v; first = false; }
        if (std::abs(v - last) > 1.0e-6f) { last = v; hold = 40; repaint(); }
        else if (hold > 0 && --hold == 0) repaint();
    }
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (syncParam == nullptr || e.mods.isPopupMenu()) return;
        const bool on = synced();
        syncParam->beginChangeGesture();
        syncParam->setValueNotifyingHost(on ? 0.0f : 1.0f);
        syncParam->endChangeGesture();
        repaint();
    }
    void paint(juce::Graphics& g) override
    {
        if (glyphs == nullptr || ! glyphs->ready()) return;
        const auto a = getLocalBounds().toFloat();
        if (synced())   // synced: show the division increment while turning, else "SYNC"
            glyphs->drawString(g, a, hold > 0 ? (fmt ? fmt(last) : juce::String(last, 1))
                                              : juce::String("SYNC"), hold > 0 ? 0.45f : 0.35f);
        else if (hold > 0)
            glyphs->drawString(g, a, fmt ? fmt(last) : juce::String(last, 1), 0.45f);  // -20%
        else
            glyphs->drawString(g, a, name, 0.35f);                                      // -20%
    }
private:
    float last = 0.0f; bool first = true; int hold = 0; bool lastSync = false;
};

// ============================================================
//  FooterStrip — the bottom info line: SYSTEM LED + sample-rate,
//  oversampling, latency, CPU, signal path, version (amber on dark).
// ============================================================
class AmberGlyphs;
class FooterStrip : public juce::Component
{
public:
    const AmberGlyphs* glyphs = nullptr;
    FilterAnalysisSource* analysis = nullptr;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::AudioProcessor* proc = nullptr;
    void tick() { repaint(); }
    void paint(juce::Graphics& g) override;   // drawn with the rendered glyph font
};

// ============================================================
//  WaveSelect — 4 LFO waveform screens (Sine/Tri/Square/Random),
//  drawn as waveform symbols; click to select. Follows LFO A/B.
// ============================================================
class WaveSelect : public juce::Component
{
public:
    juce::RangedAudioParameter* param = nullptr;   // flt_lfoX_wave (rebound A/B)
    static constexpr int kN = 4;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (param == nullptr || e.mods.isPopupMenu()) return;
        const int idx = juce::jlimit(0, kN - 1, e.x * kN / juce::jmax(1, getWidth()));
        param->beginChangeGesture();
        param->setValueNotifyingHost((float) idx / (float) (kN - 1));
        param->endChangeGesture();
        repaint();
    }
    void paint(juce::Graphics& g) override
    {
        const juce::Colour amber { 0xFFD99A33 }, dim { 0xFF5A4A28 };
        const int cur = param ? juce::roundToInt(param->getValue() * (float)(kN - 1)) : 0;
        const float cw = (float) getWidth() / (float) kN;
        for (int i = 0; i < kN; ++i)
        {
            juce::Rectangle<float> cell((float) i * cw, 0.0f, cw, (float) getHeight());
            auto sym = cell.reduced(5.0f, 3.0f);
            sym = sym.withSizeKeepingCentre(sym.getWidth() * 0.64f, sym.getHeight() * 0.64f);  // smaller
            drawWave(g, sym, i, i == cur ? amber : dim, i == cur ? 1.7f : 1.2f);
        }
    }
private:
    static void drawWave(juce::Graphics& g, juce::Rectangle<float> r, int type, juce::Colour c, float th)
    {
        juce::Path p;
        const float x0 = r.getX(), w = r.getWidth(), mid = r.getCentreY(), a = r.getHeight() * 0.42f;
        if (type == 0)            // sine
            for (int i = 0; i <= 24; ++i)
            { const float t = (float) i / 24.0f, px = x0 + t * w,
                          py = mid - std::sin(t * juce::MathConstants<float>::twoPi) * a;
              if (i == 0) p.startNewSubPath(px, py); else p.lineTo(px, py); }
        else if (type == 1)       // triangle
        { p.startNewSubPath(x0, mid + a); p.lineTo(x0 + w*0.25f, mid - a);
          p.lineTo(x0 + w*0.75f, mid + a); p.lineTo(x0 + w, mid - a); }
        else if (type == 2)       // square
        { p.startNewSubPath(x0, mid + a); p.lineTo(x0, mid - a); p.lineTo(x0 + w*0.5f, mid - a);
          p.lineTo(x0 + w*0.5f, mid + a); p.lineTo(x0 + w, mid + a); p.lineTo(x0 + w, mid - a); }
        else                      // random (sample & hold)
        {
            const float lv[5] = { 0.5f, -0.7f, 0.2f, -0.3f, 0.6f };
            for (int i = 0; i < 4; ++i)
            { const float px = x0 + w * (float) i / 4.0f, nx = x0 + w * (float)(i + 1) / 4.0f, py = mid - lv[i] * a;
              if (i == 0) p.startNewSubPath(px, py); else p.lineTo(px, py);
              p.lineTo(nx, py); if (i < 3) p.lineTo(nx, mid - lv[i + 1] * a); }
        }
        g.setColour(c);
        g.strokePath(p, juce::PathStrokeType(th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
};

// ============================================================
//  LabelText — a static amber word (filter type / slope / mode) as
//  its own component so the LayoutEditor can select & nudge it.
// ============================================================
class LabelText : public juce::Component
{
public:
    const AmberGlyphs* glyphs = nullptr;
    juce::String text;
    float heightFrac = 0.4f;
    LabelText() { setInterceptsMouseClicks(false, false); }
    void paint(juce::Graphics& g) override
    {
        if (glyphs && glyphs->ready())
            glyphs->drawString(g, getLocalBounds().toFloat(), text, heightFrac);
    }
};

// ============================================================
//  LayoutEditor — in-app positioning overlay. Right-click toggles
//  edit mode; click a text box to select; arrow keys nudge it
//  (Shift = 10px); "Edit text…" changes the content; positions
//  persist to JSON so they survive restarts (and can be baked in).
// ============================================================
class LayoutEditor : public juce::Component
{
public:
    LayoutEditor() { setWantsKeyboardFocus(true); }

    struct Item { juce::Component* comp; juce::String id; juce::String* textPtr; };
    std::vector<Item> items;
    bool editMode = false;
    juce::Component* selected = nullptr;
    std::function<void()> onChanged;

    void addItem(juce::Component* c, const juce::String& id, juce::String* textPtr = nullptr)
    { items.push_back({ c, id, textPtr }); }

    void setEditMode(bool on)
    {
        editMode = on;
        setInterceptsMouseClicks(on, on);
        if (on) { toFront(false); grabKeyboardFocus(); }
        repaint();
    }
    bool hitTest(int, int) override { return editMode; }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (! editMode) return;
        if (e.mods.isPopupMenu()) { showMenu(); return; }
        selected = nullptr;
        const auto p = e.getPosition();
        for (auto it = items.rbegin(); it != items.rend(); ++it)
            if (it->comp->getBounds().contains(p)) { selected = it->comp; break; }
        grabKeyboardFocus();
        repaint();
    }
    bool keyPressed(const juce::KeyPress& k) override
    {
        if (! editMode) return false;
        if (k.getKeyCode() == juce::KeyPress::escapeKey) { setEditMode(false); return true; }
        if (selected == nullptr) return false;
        const int s = k.getModifiers().isShiftDown() ? 10 : 1;
        auto b = selected->getBounds();
        if      (k.getKeyCode() == juce::KeyPress::leftKey)  b.translate(-s, 0);
        else if (k.getKeyCode() == juce::KeyPress::rightKey) b.translate( s, 0);
        else if (k.getKeyCode() == juce::KeyPress::upKey)    b.translate(0, -s);
        else if (k.getKeyCode() == juce::KeyPress::downKey)  b.translate(0,  s);
        else return false;
        selected->setBounds(b);
        repaint();
        if (onChanged) onChanged();
        return true;
    }
    juce::String idOf(juce::Component* c) const
    { for (auto& it : items) if (it.comp == c) return it.id; return {}; }

    void paint(juce::Graphics& g) override
    {
        if (! editMode) return;
        g.fillAll(juce::Colours::black.withAlpha(0.12f));
        for (auto& it : items)
        { g.setColour(juce::Colours::deepskyblue.withAlpha(0.55f)); g.drawRect(it.comp->getBounds(), 1); }
        if (selected != nullptr)
        {
            g.setColour(juce::Colours::yellow); g.drawRect(selected->getBounds(), 2);
            const auto b = selected->getBounds();
            g.setFont(13.0f);
            g.drawText(idOf(selected) + "   x=" + juce::String(b.getX()) + " y=" + juce::String(b.getY())
                       + " w=" + juce::String(b.getWidth()) + " h=" + juce::String(b.getHeight()),
                       8, getHeight() - 24, getWidth() - 16, 18, juce::Justification::left);
        }
        g.setColour(juce::Colours::yellow); g.setFont(13.0f);
        g.drawText("LAYOUT EDIT  —  click text to select • arrows nudge (Shift=10) • right-click menu • Esc to exit",
                   8, 4, getWidth() - 16, 18, juce::Justification::left);
    }

    void showMenu()
    {
        juce::PopupMenu m;
        m.addItem(1, "Layout edit mode", true, editMode);
        m.addItem(2, "Edit text…", textOf(selected) != nullptr);
        m.addSeparator();
        m.addItem(3, "Save layout now");
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this](int r)
            {
                if      (r == 1) setEditMode(! editMode);
                else if (r == 2) editSelectedText();
                else if (r == 3 && onChanged) onChanged();
            });
    }

    juce::String* textOf(juce::Component* c) const
    { for (auto& it : items) if (it.comp == c) return it.textPtr; return nullptr; }

    void editSelectedText()
    {
        auto* tp = textOf(selected);
        if (tp == nullptr) return;
        auto* comp = selected;
        auto* aw = new juce::AlertWindow("Edit text", "Label text:", juce::MessageBoxIconType::NoIcon);
        aw->addTextEditor("t", *tp);
        aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
        aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        aw->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, aw, tp, comp](int r)
            {
                if (r == 1) { *tp = aw->getTextEditorContents("t").toUpperCase();
                              if (comp) comp->repaint(); if (onChanged) onChanged(); }
                delete aw;
            }), false);
    }

    void saveLayout(const juce::File& f)
    {
        juce::DynamicObject::Ptr root = new juce::DynamicObject();
        for (auto& it : items)
        {
            const auto b = it.comp->getBounds();
            juce::Array<juce::var> a { b.getX(), b.getY(), b.getWidth(), b.getHeight() };
            if (it.textPtr) a.add(*it.textPtr);
            root->setProperty(it.id, a);
        }
        f.create();
        f.replaceWithText(juce::JSON::toString(juce::var(root.get())));
    }
    void loadLayout(const juce::File& f)
    {
        if (! f.existsAsFile()) return;
        const auto v = juce::JSON::parse(f);
        if (! v.isObject()) return;
        for (auto& it : items)
        {
            const auto a = v.getProperty(it.id, juce::var());
            if (a.isArray() && a.size() >= 4)
            {
                it.comp->setBounds((int) a[0], (int) a[1], (int) a[2], (int) a[3]);
                if (it.textPtr && a.size() >= 5) { *it.textPtr = a[4].toString(); it.comp->repaint(); }
            }
        }
    }
};

// ============================================================
//  Prime76Editor — Blender hardware UI: faceplate + 13 sprite
//  knobs + live response/LFO/Env screens + option LEDs.
//  Canvas 1431x900 (render 2862x1801 @1x, 182.36 px/world-unit).
// ============================================================
class Prime76Editor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    Prime76Editor(juce::AudioProcessor&,
                  juce::AudioProcessorValueTreeState&,
                  FilterAnalysisSource&);
    ~Prime76Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // window = the faceplate's true opaque size (plate ends here — no right strip).
    // plate opaque 2810x1801 @2x  ->  1405x900 @1x.
    static constexpr int kW = 1405, kH = 900;

private:
    struct Knob
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag,
                              juce::Slider::NoTextBox };
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
        int cx = 0, cy = 0, diam = 0;
    };
    void addKnob(Knob&, const juce::String& paramId, int cx, int cy, int diam);
    void timerCallback() override;

    juce::AudioProcessorValueTreeState& apvts;
    FilterAnalysisSource& analysis;
    Prime76LookAndFeel lnf;
    GlyphLnF headerLnF;
    juce::Image faceplate;

    Knob freq, res, drive, mix, out, gain;
    Knob amount, lfoRate, lfoDepth, lfoPhase, envAtk, envRel, envSens;
    std::array<Knob*, 13> all { &freq,&res,&drive,&mix,&out,&gain,&amount,
                                &lfoRate,&lfoDepth,&lfoPhase,&envAtk,&envRel,&envSens };

    ResponseDisplay display;
    LfoScope        lfoScope;
    EnvScope        envScope;
    OptionLeds      optionLeds;

    AmberGlyphs                  glyphs;
    std::array<ReadoutScreen, 12> readouts;    // 5 hero + 7 bottom (mod/lfo/env)
    std::array<LabelText, 11>    labels;        // 6 filter-type + 3 slope + 2 mode
    std::array<LabelText, 6>     modLabels;     // LFO A/B/EF source + FREQ/RESO/DRIVE dest (text only)
    PeakReadout                  peakReadout;   // dB peak numbers above the meters
    FooterStrip                  footer;        // bottom info line
    WaveSelect                   waveSelect;    // LFO wave symbols (sine/tri/sqr/rand)
    LayoutEditor                 layoutEditor;  // in-app nudge/edit overlay
    juce::File                   layoutFile;
    // ---- header line (preset / A-B / power / oversampling / mix) ----
    juce::TextButton hPrev { "<" }, hNext { ">" }, hSave { "SAVE" },
                     hA { "A" }, hB { "B" }, hPwr { "PWR" }, hOS { "1x" };
    juce::Label hName, hMix;
    juce::StringArray presets; int presetIdx = 0;
    juce::ValueTree abSlot[2]; int abActive = 0;
    juce::ValueTree initState;
    void buildHeader();
    void refreshPresetList();
    void gotoPreset(int idx);
    void saveCurrentPreset();
    void setAB(int slot);
    void updateHeader();

    void applyDefaultPositions();
    void fitModLabels();                        // shrink mod label boxes to their text
    void bindKnob(Knob&, const juce::String& paramId);
    void rebindLfo(int lfoIndex, bool sync);    // follow mod source A/B + sync (rate<->division)
    int  boundLfo = -1; bool boundLfoSync = false;
    void mouseDown(const juce::MouseEvent&) override;   // right-click -> layout menu

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Prime76Editor)
};
