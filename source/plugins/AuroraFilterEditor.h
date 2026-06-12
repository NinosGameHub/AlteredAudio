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
    static constexpr float kDbTop = 24.0f, kDbBot = -36.0f;

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
    void applyPreset(int index);
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
    juce::TextButton btnA { "A" }, btnB { "B" };
    PowerKey powerBtn;
    juce::Label presetLabel;
    int currentPreset = 0;
    juce::ValueTree slotA, slotB;
    int activeSlot = 0;

    // Modulation section — routing dropdowns (Select.jsx idiom)
    juce::ComboBox srcBox, dstBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> srcAttach, dstAttach;
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
