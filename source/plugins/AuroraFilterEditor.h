#pragma once
#include <JuceHeader.h>
#include "../FilterModule.h"
#include "../ParameterIDs.h"

// ============================================================
//  Filter 76 palette — filter76-design/tokens/colors.css
// ============================================================
namespace aurora
{
    const juce::Colour baseSurface { 0xFFE6E0D2 };  // dirty white — ABS plastic
    const juce::Colour warmBeige   { 0xFFD8D1C1 };  // secondary / recessed surface
    const juce::Colour cream       { 0xFFEDE7D8 };  // raised — knob bodies, keys
    const juce::Colour sand        { 0xFFCFC7B5 };  // pressed / sunken
    const juce::Colour border      { 0xFF7A7468 };
    const juce::Colour textPrimary { 0xFF1F1F1F };
    const juce::Colour textSecond  { 0xFF5E594E };
    const juce::Colour inkFaint    { 0xFF908A7D };  // tertiary / disabled
    const juce::Colour amber       { 0xFFD08A2E };
    const juce::Colour amberHi     { 0xFFE0A047 };  // hover
    const juce::Colour amberLo     { 0xFFB0721F };  // press / product mark
    const juce::Colour textOnAccent{ 0xFF1A140A };
    const juce::Colour mustard     { 0xFFB8943A };
    const juce::Colour warnOrange  { 0xFFC56A2B };
    const juce::Colour graphBg     { 0xFF171717 };
    const juce::Colour graphLine   { 0xFFF0B547 };
    const juce::Colour curveGlow   { 0xFFFBD27A };  // response-curve halo
    const juce::Colour curveLine   { 0xFFFFE0A0 };  // response-curve core
    const juce::Colour led         { 0xFFD99A33 };
    const juce::Colour ledOff      { 0xFF6E685C };  // disabled indicator / LED bezel

    inline juce::Font heading(float h)
    {
        return juce::Font(h, juce::Font::bold).withExtraKerningFactor(0.10f);
    }
    inline juce::Font mono(float h)
    {
        return juce::Font(juce::Font::getDefaultMonospacedFontName(), h, juce::Font::plain);
    }

    // Tempo-sync note divisions, shared by DSP and UI
    inline const juce::StringArray& divNames()
    {
        static const juce::StringArray n {
            "1/1","1/2","1/2D","1/2T","1/4","1/4D","1/4T",
            "1/8","1/8D","1/8T","1/16","1/16D","1/16T","1/32" };
        return n;
    }
    inline float divToMs(int idx, double bpm)
    {
        static constexpr float beats[] = {
            4.0f, 2.0f, 3.0f, 4.0f/3.0f, 1.0f, 1.5f, 2.0f/3.0f,
            0.5f, 0.75f, 1.0f/3.0f, 0.25f, 0.375f, 1.0f/6.0f, 0.125f };
        const int i = juce::jlimit(0, 13, idx);
        return (float)(beats[i] * 60000.0 / juce::jlimit(20.0, 999.0, bpm));
    }
}

// ============================================================
//  PresetManager — user presets in ~/Documents/AlteredAudio/Filter 76/Presets/
// ============================================================
struct PresetManager
{
    static juce::File presetsDir()
    {
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
               .getChildFile("AlteredAudio")
               .getChildFile("Filter 76")
               .getChildFile("Presets");
    }
    static juce::StringArray userNames()
    {
        juce::StringArray names;
        auto dir = presetsDir();
        if (!dir.isDirectory()) return names;
        juce::Array<juce::File> files;
        dir.findChildFiles(files, juce::File::findFiles, false, "*.f76preset");
        for (auto& f : files)
            names.add(f.getFileNameWithoutExtension());
        names.sort(false);
        return names;
    }
    static bool save(const juce::String& name, juce::AudioProcessorValueTreeState& apvts)
    {
        auto dir = presetsDir();
        if (!dir.isDirectory() && !dir.createDirectory()) return false;
        auto xml = apvts.copyState().createXml();
        return xml && xml->writeTo(dir.getChildFile(name + ".f76preset"));
    }
    static juce::ValueTree load(const juce::String& name)
    {
        auto f = presetsDir().getChildFile(name + ".f76preset");
        if (!f.existsAsFile()) return {};
        auto xml = juce::XmlDocument::parse(f);
        return xml ? juce::ValueTree::fromXml(*xml) : juce::ValueTree{};
    }
    static bool remove(const juce::String& name)
    {
        return presetsDir().getChildFile(name + ".f76preset").deleteFile();
    }
};

// ============================================================
//  SavePresetOverlay — inline save dialog, floats over the content component
// ============================================================
class SavePresetOverlay : public juce::Component
{
public:
    std::function<void(const juce::String&)> onSave;
    std::function<void()> onDismiss;

    explicit SavePresetOverlay(const juce::String& initialName)
    {
        nameEd.setText(initialName, false);
        nameEd.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
        nameEd.setColour(juce::TextEditor::backgroundColourId,      aurora::graphBg);
        nameEd.setColour(juce::TextEditor::textColourId,            aurora::graphLine);
        nameEd.setColour(juce::TextEditor::outlineColourId,         aurora::border);
        nameEd.setColour(juce::TextEditor::focusedOutlineColourId,  aurora::amber);
        nameEd.setJustification(juce::Justification::centredLeft);
        nameEd.onReturnKey = [this]() { doConfirm(); };
        nameEd.onEscapeKey = [this]() { doCancel();  };
        addAndMakeVisible(nameEd);

        confirmBtn.setColour(juce::TextButton::buttonColourId,  aurora::amber);
        confirmBtn.setColour(juce::TextButton::textColourOffId, aurora::textOnAccent);
        confirmBtn.onClick = [this]() { doConfirm(); };
        cancelBtn .onClick = [this]() { doCancel();  };
        addAndMakeVisible(confirmBtn);
        addAndMakeVisible(cancelBtn);
        setInterceptsMouseClicks(true, true);
    }

    void visibilityChanged() override
    {
        if (isVisible()) { nameEd.selectAll(); nameEd.grabKeyboardFocus(); }
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0x8C171717));   // --overlay-scrim: rgba(23,23,23,0.55)
        g.fillAll();

        const auto dlg = dialogBounds();
        g.setColour(aurora::baseSurface);
        g.fillRoundedRectangle(dlg, 8.0f);
        g.setColour(juce::Colour(0x66FFFCF4));
        g.fillRect(dlg.getX() + 2.0f, dlg.getY() + 1.0f, dlg.getWidth() - 4.0f, 1.5f);
        g.setColour(aurora::border);
        g.drawRoundedRectangle(dlg, 8.0f, 1.0f);

        g.setColour(aurora::textPrimary);
        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold)
                  .withExtraKerningFactor(0.14f));
        g.drawText("SAVE PRESET",
                   juce::roundToInt(dlg.getX()) + 20,
                   juce::roundToInt(dlg.getY()) + 16, 300, 15,
                   juce::Justification::centredLeft);
        g.setColour(aurora::textSecond);
        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain)
                  .withExtraKerningFactor(0.06f));
        g.drawText("NAME",
                   juce::roundToInt(dlg.getX()) + 20,
                   juce::roundToInt(dlg.getY()) + 44, 80, 12,
                   juce::Justification::centredLeft);
    }

    void resized() override
    {
        const auto dlg = dialogBounds();
        nameEd.setBounds(juce::roundToInt(dlg.getX()) + 20,
                         juce::roundToInt(dlg.getY()) + 58,
                         juce::roundToInt(dlg.getWidth()) - 40, 28);
        constexpr int bw = 74, bh = 26;
        confirmBtn.setBounds(juce::roundToInt(dlg.getRight()) - bw * 2 - 18,
                             juce::roundToInt(dlg.getBottom()) - bh - 14, bw, bh);
        cancelBtn .setBounds(juce::roundToInt(dlg.getRight()) - bw - 10,
                             juce::roundToInt(dlg.getBottom()) - bh - 14, bw, bh);
    }

private:
    juce::TextEditor nameEd;
    juce::TextButton confirmBtn { "SAVE" }, cancelBtn { "CANCEL" };

    juce::Rectangle<float> dialogBounds() const
    {
        const float w = (float)getWidth(), h = (float)getHeight();
        constexpr float dw = 440.0f, dh = 160.0f;
        return { (w - dw) * 0.5f, (h - dh) * 0.5f, dw, dh };
    }
    void doConfirm()
    {
        const auto n = nameEd.getText().trim().toUpperCase();
        if (n.isNotEmpty() && onSave) onSave(n);
        doCancel();
    }
    void doCancel()
    {
        if (onDismiss) juce::MessageManager::callAsync(onDismiss);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SavePresetOverlay)
};

// ============================================================
//  FilterAnalysisSource — lock-free audio→UI data bridge
// ============================================================
struct FilterAnalysisSource
{
    // Large FFT + continuously-written ring buffer: the UI snapshots the
    // most recent kFFTSize samples every frame (overlapped analysis), so
    // the spectrum is both high-resolution in the lows and temporally
    // smooth — instead of one chunky update per FFT fill.
    static constexpr int kFFTOrder = 13;
    static constexpr int kFFTSize  = 1 << kFFTOrder;   // 8192

    void pushBlock(const juce::AudioBuffer<float>& buf)
    {
        const int n  = buf.getNumSamples();
        const int nc = juce::jmin(2, buf.getNumChannels());

        for (int ch = 0; ch < nc; ++ch)
        {
            const float mag = buf.getMagnitude(ch, 0, n);
            auto& pk = (ch == 0) ? peakL : peakR;
            float cur = pk.load();
            while (mag > cur && !pk.compare_exchange_weak(cur, mag)) {}
        }

        int wp = writePos.load(std::memory_order_relaxed);
        for (int i = 0; i < n; ++i)
        {
            float s = 0.0f;
            for (int ch = 0; ch < nc; ++ch) s += buf.getSample(ch, i);
            ring[wp] = s / (float)juce::jmax(1, nc);
            wp = (wp + 1) & (kFFTSize - 1);
        }
        writePos.store(wp, std::memory_order_release);
    }

    float ring[kFFTSize] = {};
    std::atomic<int>   writePos { 0 };
    std::atomic<float> peakL { 0.0f }, peakR { 0.0f };
    std::atomic<float> sampleRate { 44100.0f };
    std::atomic<float> cpuPct   { 0.0f };
    std::atomic<float> envValue { 0.0f };
    std::atomic<float> lfoPhase { 0.0f };

    // Live post-modulation filter state, for the animated ghost curve
    std::atomic<bool>  modActive { false };
    std::atomic<float> modFreq  { 1000.0f };
    std::atomic<float> modQ     { 0.707f };
};

// ============================================================
//  AuroraLookAndFeel — Filter 76 controls: matte-cream puck knobs
//  with a floating tick ring + amber dot, LED option rows, amber
//  toggle keys, dark amber readout wells.
// ============================================================
class AuroraLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AuroraLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool highlighted, bool down) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool highlighted, bool down) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
};

// ============================================================
//  PowerKey — round cream power key with status LED beneath
// ============================================================
class PowerKey : public juce::Button
{
public:
    PowerKey() : juce::Button("power") { setClickingTogglesState(true); }

    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        const bool on = getToggleState();
        const auto r  = getLocalBounds().toFloat();
        const float d = juce::jmin(r.getWidth(), r.getHeight() - 14.0f);
        juce::Rectangle<float> key(r.getCentreX() - d * 0.5f, r.getY(), d, d);

        // soft drop shadow + cream dome
        g.setColour(juce::Colour(0x402A251C));
        g.fillEllipse(key.translated(0.0f, 1.8f));
        juce::ColourGradient grad(juce::Colour(0xFFF3EEE0),
                                  key.getX() + d * 0.38f, key.getY() + d * 0.30f,
                                  juce::Colour(0xFFDCD4C2),
                                  key.getX() + d * 0.5f,  key.getBottom(), true);
        grad.addColour(0.42, aurora::cream);
        g.setGradientFill(grad);
        g.fillEllipse(key.reduced(down ? 1.2f : 0.0f));
        g.setColour(aurora::border.withAlpha(over ? 0.95f : 0.65f));
        g.drawEllipse(key.reduced(0.5f), 1.0f);

        // power glyph — thin rounded strokes
        const float cx = key.getCentreX(), cy = key.getCentreY(), gr = d * 0.22f;
        g.setColour(on ? aurora::textPrimary : aurora::inkFaint);
        g.drawLine(cx, cy - gr * 1.25f, cx, cy - gr * 0.15f, 2.2f);
        juce::Path arc;
        arc.addCentredArc(cx, cy, gr, gr, 0.0f,
                          juce::MathConstants<float>::pi * 0.30f,
                          juce::MathConstants<float>::pi * 1.70f, true);
        g.strokePath(arc, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        // status LED
        const float lr = 3.0f, ly = r.getBottom() - lr * 2.2f;
        if (on)
        {
            g.setColour(aurora::led.withAlpha(0.35f));
            g.fillEllipse(cx - lr * 2.0f, ly - lr * 2.0f + lr, lr * 4.0f, lr * 4.0f);
            g.setColour(aurora::led);
        }
        else
            g.setColour(aurora::border.withAlpha(0.5f));
        g.fillEllipse(cx - lr, ly, lr * 2.0f, lr * 2.0f);
    }
};

// ============================================================
//  ResponseDisplay — spectrum + exact response + draggable node
// ============================================================
class ResponseDisplay : public juce::Component
{
public:
    ResponseDisplay(juce::AudioProcessorValueTreeState&, FilterAnalysisSource&);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void refresh();

private:
    float xForFreq(float hz, float w) const;
    float freqForX(float x, float w) const;
    float yForDb(float db, float h) const;

    juce::AudioProcessorValueTreeState& apvts;
    FilterAnalysisSource& analysis;

    juce::dsp::FFT fft { FilterAnalysisSource::kFFTOrder };
    juce::dsp::WindowingFunction<float> window {
        FilterAnalysisSource::kFFTSize, juce::dsp::WindowingFunction<float>::hann };
    float fftWork[FilterAnalysisSource::kFFTSize * 2] = {};

    static constexpr int kSpecPoints = 400;
    float specDisp[kSpecPoints] = {};
    float specWork[kSpecPoints] = {};

    static constexpr float kMinF = 20.0f, kMaxF = 20000.0f;
    static constexpr float kDbTop = 12.0f, kDbBot = -60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResponseDisplay)
};

// ============================================================
//  MeterColumn — stereo peak meters
// ============================================================
class MeterColumn : public juce::Component
{
public:
    void setLevels(float l, float r);
    void paint(juce::Graphics&) override;
private:
    float dispL = 0.0f, dispR = 0.0f;
};

// ============================================================
//  LfoScope — mini oscilloscope of the active LFO
// ============================================================
class LfoScope : public juce::Component
{
public:
    LfoScope(juce::AudioProcessorValueTreeState&, FilterAnalysisSource&);
    void setLfoIndex(int idx) { lfoIndex = idx; }
    void paint(juce::Graphics&) override;
private:
    juce::AudioProcessorValueTreeState& apvts;
    FilterAnalysisSource& analysis;
    int lfoIndex = 0;
};

// ============================================================
//  EnvScope — scrolling envelope-follower level history
// ============================================================
class EnvScope : public juce::Component
{
public:
    void push(float v);
    void paint(juce::Graphics&) override;
private:
    static constexpr int kHist = 120;
    float hist[kHist] = {};
    int   head = 0;
};

// ============================================================
//  AuroraFilterEditor — resizable, fixed 1400x900 design space
// ============================================================
class AuroraFilterEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    AuroraFilterEditor(juce::AudioProcessor&,
                       juce::AudioProcessorValueTreeState&,
                       FilterAnalysisSource&);
    ~AuroraFilterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int kW = 1400, kH = 900;   // logical design size

private:
    struct AuroraKnob
    {
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    void timerCallback() override;
    void makeKnob(AuroraKnob&, const juce::String& paramId, juce::Component& parent);
    void setParam(const juce::String& id, float realValue);
    void applyPreset(int index);      // factory-only; prefer loadPresetAtIdx
    void loadPresetAtIdx(int idx);    // unified factory + user navigation
    void updatePresetLabel();
    void browsePresets();
    void doSavePreset(const juce::String& name);
    void switchAB(int slot);
    void rebindLfoKnobs(int lfoIndex);
    void rebindEnvKnobs();
    void updateButtonStates();
    void showSyncMenu(const juce::String& syncParamId);

    // Right-click hook for knobs that offer FREE / TEMPO SYNC
    struct RightClick : juce::MouseListener
    {
        std::function<void()> fn;
        void mouseDown(const juce::MouseEvent& e) override
        {
            if (e.mods.isPopupMenu() && fn) fn();
        }
    };

    // All children live on this fixed-size content component, scaled to fit
    struct PaintDelegate : juce::Component
    {
        std::function<void(juce::Graphics&)> onPaint;
        void paint(juce::Graphics& g) override { if (onPaint) onPaint(g); }
    };
    PaintDelegate content;

    juce::AudioProcessorValueTreeState& apvts;
    FilterAnalysisSource& analysis;
    AuroraLookAndFeel lnf;

    ResponseDisplay display;
    MeterColumn     meters;
    LfoScope        lfoScope;
    EnvScope        envScope;

    // Filter section
    juce::TextButton typeBtns[6];   // LP HP BP NOTCH PEAK SHELF
    AuroraKnob freqKnob, resKnob, driveKnob, mixKnob, outKnob, gainKnob;
    juce::TextButton slope12 { "12" }, slope24 { "24" }, slope48 { "48" };
    juce::TextButton modeAnalog { "ANALOG" }, modeClean { "CLEAN" };

    // Header
    juce::TextButton prevPreset { "<" }, nextPreset { ">" };
    juce::TextButton saveBtn    { "SAVE" };
    juce::TextButton btnA { "A" }, btnB { "B" };
    PowerKey powerBtn;
    juce::Label presetLabel;
    int  currentPresetIdx  = 0;
    bool currentIsUser     = false;
    juce::String currentPresetName { "INIT" };
    juce::ValueTree slotA, slotB;
    int activeSlot = 0;

    // Preset save overlay (shown inline on SAVE click)
    std::unique_ptr<SavePresetOverlay> saveOverlay;

    // Click-to-browse handler for the preset label
    struct PresetLabelClick : juce::MouseListener
    {
        std::function<void()> fn;
        void mouseDown(const juce::MouseEvent& e) override
        { if (!e.mods.isPopupMenu() && fn) fn(); }
    };
    PresetLabelClick presetLabelClick;

    // Modulation section — routing dropdowns (Select.jsx idiom)
    juce::ComboBox srcBox, dstBox, osBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> srcAttach, dstAttach, osAttach;
    AuroraKnob amountKnob;

    // LFO engine
    juce::TextButton waveBtns[4];   // SINE TRI SQR RND
    AuroraKnob rateKnob, depthKnob, phaseKnob;
    int boundLfo = -1;
    int boundLfoSync = -1;

    // Envelope follower
    AuroraKnob atkKnob, relKnob, sensKnob;
    int boundAtkSync = -1, boundRelSync = -1;

    RightClick rateRC, atkRC, relRC;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraFilterEditor)
};
