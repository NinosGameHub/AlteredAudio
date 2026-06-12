#pragma once
#include <JuceHeader.h>
#include "../ParameterIDs.h"

// ============================================================
//  Gain 76 — same Altered Audio faceplate language as Filter 76
//  (palette mirrors filter76-design/tokens/colors.css)
// ============================================================
namespace gain76
{
    const juce::Colour baseSurface { 0xFFE6E0D2 };  // dirty white — ABS plastic
    const juce::Colour warmBeige   { 0xFFD8D1C1 };  // secondary / recessed surface
    const juce::Colour cream       { 0xFFEDE7D8 };  // raised — knob bodies, keys
    const juce::Colour sand        { 0xFFCFC7B5 };  // pressed / sunken
    const juce::Colour border      { 0xFF7A7468 };
    const juce::Colour textPrimary { 0xFF1F1F1F };
    const juce::Colour textSecond  { 0xFF5E594E };
    const juce::Colour inkFaint    { 0xFF908A7D };
    const juce::Colour amber       { 0xFFD08A2E };
    const juce::Colour amberHi     { 0xFFE0A047 };
    const juce::Colour amberLo     { 0xFFB0721F };
    const juce::Colour textOnAccent{ 0xFF1A140A };
    const juce::Colour warnOrange  { 0xFFC56A2B };
    const juce::Colour graphBg     { 0xFF171717 };
    const juce::Colour graphLine   { 0xFFF0B547 };
    const juce::Colour led         { 0xFFD99A33 };
    const juce::Colour ledOff      { 0xFF6E685C };

    inline juce::Font heading(float h)
    {
        return juce::Font(h, juce::Font::bold).withExtraKerningFactor(0.10f);
    }
    inline juce::Font mono(float h)
    {
        return juce::Font(juce::Font::getDefaultMonospacedFontName(), h, juce::Font::plain);
    }
}

// ============================================================
//  GainAnalysisSource — lock-free audio→UI bridge
// ============================================================
struct GainAnalysisSource
{
    void pushInput(const juce::AudioBuffer<float>& buf)  { push(buf, inL,  inR);  }
    void pushOutput(const juce::AudioBuffer<float>& buf) { push(buf, outL, outR); }

    std::atomic<float> inL  { 0.0f }, inR  { 0.0f };
    std::atomic<float> outL { 0.0f }, outR { 0.0f };
    std::atomic<float> lufs { -100.0f };           // momentary, K-weighted, output
    std::atomic<float> sampleRate { 44100.0f };
    std::atomic<float> cpuPct { 0.0f };

private:
    static void push(const juce::AudioBuffer<float>& buf,
                     std::atomic<float>& l, std::atomic<float>& r)
    {
        const int n  = buf.getNumSamples();
        const int nc = juce::jmin(2, buf.getNumChannels());
        for (int ch = 0; ch < nc; ++ch)
        {
            const float mag = buf.getMagnitude(ch, 0, n);
            auto& pk = (ch == 0) ? l : r;
            float cur = pk.load();
            while (mag > cur && !pk.compare_exchange_weak(cur, mag)) {}
        }
    }
};

// ============================================================
//  Gain76LookAndFeel — Filter 76 face knob (value printed on the
//  disc), flat cream combo boxes, cream popup menus
// ============================================================
class Gain76LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Gain76LookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
};

// ============================================================
//  Gain76PowerKey — round cream power key with status LED
// ============================================================
class Gain76PowerKey : public juce::Button
{
public:
    Gain76PowerKey() : juce::Button("power") { setClickingTogglesState(true); }
    void paintButton(juce::Graphics&, bool over, bool down) override;
};

// ============================================================
//  VerticalMeter — segmented amber column + dB scale labels
// ============================================================
class VerticalMeter : public juce::Component
{
public:
    void setLevel(float lin) { disp = juce::jmax(lin, disp * 0.88f); repaint(); }
    void paint(juce::Graphics&) override;
private:
    float disp = 0.0f;
};

// ============================================================
//  HoldMeter — horizontal segmented peak meter with a hold marker
//  (mirrored variant grows right-to-left for the R channel)
// ============================================================
class HoldMeter : public juce::Component
{
public:
    explicit HoldMeter(bool mirroredBar) : mirrored(mirroredBar) {}

    void setLevel(float lin)
    {
        disp = juce::jmax(lin, disp * 0.88f);
        if (lin >= holdLin) { holdLin = lin; holdAge = 0; }
        else if (++holdAge > kHoldFrames) holdLin *= 0.94f;   // release after the hold time
        repaint();
    }

    void paint(juce::Graphics&) override;

private:
    static constexpr int kHoldFrames = 75;   // ~2.5 s at 30 fps
    bool  mirrored;
    float disp = 0.0f, holdLin = 0.0f;
    int   holdAge = 0;
};

// ============================================================
//  GainEditor — resizable, fixed 820x820 design space
// ============================================================
class GainEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    GainEditor(juce::AudioProcessor&, juce::AudioProcessorValueTreeState&,
               GainAnalysisSource&);
    ~GainEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int kW = 820, kH = 820;

private:
    void timerCallback() override;

    struct PaintDelegate : juce::Component
    {
        std::function<void(juce::Graphics&)> onPaint;
        void paint(juce::Graphics& g) override { if (onPaint) onPaint(g); }
    };
    PaintDelegate content;

    // click-to-reset zone for the held PEAK readout
    struct ClickZone : juce::Component
    {
        std::function<void()> fn;
        ClickZone() { setMouseCursor(juce::MouseCursor::PointingHandCursor); }
        void mouseDown(const juce::MouseEvent&) override { if (fn) fn(); }
    };

    juce::AudioProcessorValueTreeState& apvts;
    GainAnalysisSource& analysis;
    Gain76LookAndFeel lnf;

    juce::Slider gainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;

    juce::ComboBox modeBox, osBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttach, osAttach;

    Gain76PowerKey powerBtn;
    VerticalMeter inMeter, outMeter;
    HoldMeter     stripL { false }, stripR { true };
    ClickZone     peakReset;

    float heldPeakDb = -120.0f;   // infinite hold, click PEAK cell to reset

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainEditor)
};
