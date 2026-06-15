#pragma once
#include <JuceHeader.h>

namespace ParamID
{
    constexpr const char* gainBypass   = "gain_bypass";
    constexpr const char* gainDb       = "gain_db";
    constexpr const char* gainMode     = "gain_mode";
    constexpr const char* gainOversamp = "gain_oversamp";
}

namespace gain76
{
    const juce::Colour baseSurface { 0xFFE6E0D2 };
    const juce::Colour warmBeige   { 0xFFD8D1C1 };
    const juce::Colour cream       { 0xFFEDE7D8 };
    const juce::Colour sand        { 0xFFCFC7B5 };
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
//  GainAnalysisSource
// ============================================================
struct GainAnalysisSource
{
    void pushInput(const juce::AudioBuffer<float>& buf)  { push(buf, inL,  inR);  }
    void pushOutput(const juce::AudioBuffer<float>& buf) { push(buf, outL, outR); }

    std::atomic<float> inL  { 0.0f }, inR  { 0.0f };
    std::atomic<float> outL { 0.0f }, outR { 0.0f };
    std::atomic<float> lufs { -100.0f };
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
//  LayoutStore — persists component positions across sessions
// ============================================================
struct LayoutStore
{
    // Defaults aligned to the square 720x720 Blender design sprites.
    juce::Rectangle<int> knob     { 190, 190, 340, 340 };
    juce::Rectangle<int> inMeter  {  68, 166,  63, 350 };
    juce::Rectangle<int> outMeter { 588, 166,  63, 351 };
    juce::Rectangle<int> power    { 660,  18,  44,  44 };
    juce::Rectangle<int> footer   {  67, 592, 586,  41 };
    juce::Rectangle<int> panel    {  40, 140, 640, 430 };

    static juce::File storageFile();
    void load();
    void save() const;
};

// ============================================================
//  Gain76LookAndFeel
// ============================================================
class Gain76LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Gain76LookAndFeel();

    // Blender-rendered knob disc, full-frame @2x; drawRotarySlider crops the
    // knob region from it so the sprite stays locked to the slider component.
    juce::Image knobImage;       // static base (flat body + rim/groove/ticks)
    juce::Image grainImage;      // body grain overlay — rotates with the value
    juce::Image indicatorImage;  // LED position dot — rotates with the value

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
};

// ============================================================
//  Gain76PowerKey
// ============================================================
class Gain76PowerKey : public juce::Button
{
public:
    Gain76PowerKey() : juce::Button("power") { setClickingTogglesState(true); }
    void paintButton(juce::Graphics&, bool over, bool down) override;
};

// ============================================================
//  VerticalMeter
// ============================================================
class VerticalMeter : public juce::Component
{
public:
    explicit VerticalMeter(bool scaleOnRight) : mirrored(scaleOnRight) {}

    // Full-frame @2x Blender meter sprites; paint() crops each to this
    // component's region and locks them so overlays align exactly.
    juce::Image housingImage;   // brass housing (background)
    juce::Image stripImage;     // amber lit-segment strip (recessed inside)

    float level() const noexcept { return disp; }   // smoothed display level (lin)

    void setLevel(float lin)
    {
        disp = juce::jmax(lin, disp * 0.88f);
        if (lin >= holdLin) { holdLin = lin; holdAge = 0; }
        else if (++holdAge > kHoldFrames) holdLin *= 0.93f;
        repaint();
    }
    void paint(juce::Graphics&) override;
private:
    static constexpr int kHoldFrames = 42;
    bool  mirrored;
    float disp = 0.0f, holdLin = 0.0f;
    int   holdAge = 0;
};

// ============================================================
//  HoldMeter
// ============================================================
class HoldMeter : public juce::Component
{
public:
    explicit HoldMeter(bool mirroredBar) : mirrored(mirroredBar) {}

    void setLevel(float lin)
    {
        disp = juce::jmax(lin, disp * 0.88f);
        if (lin >= holdLin) { holdLin = lin; holdAge = 0; }
        else if (++holdAge > kHoldFrames) holdLin *= 0.94f;
        repaint();
    }

    void paint(juce::Graphics&) override;

private:
    static constexpr int kHoldFrames = 75;
    bool  mirrored;
    float disp = 0.0f, holdLin = 0.0f;
    int   holdAge = 0;
};

// ============================================================
//  GainEditor
// ============================================================
class GainEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    GainEditor(juce::AudioProcessor&, juce::AudioProcessorValueTreeState&,
               GainAnalysisSource&);
    ~GainEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int kW = 720, kH = 720;   // square — matches the design

private:
    void timerCallback() override;
    // Draws a numeric string as amber glyph sprites filling a W x H screen cell.
    void drawAmberReadout(juce::Graphics&, int W, int H, const juce::String& num, float scale = 1.0f);
    void applyLayout();
    void enterEditMode();
    void exitEditMode();

    // ---- Faceplate background + right-click hook ----
    struct PaintDelegate : juce::Component
    {
        std::function<void(juce::Graphics&)> onPaint;
        std::function<void(const juce::MouseEvent&)> onRightClick;
        std::function<void()> onLeftClick;
        void paint(juce::Graphics& g) override { if (onPaint) onPaint(g); }
        void mouseDown(const juce::MouseEvent& e) override
        {
            if (e.mods.isRightButtonDown() && onRightClick) onRightClick(e);
            else if (onLeftClick) onLeftClick();
        }
    };
    PaintDelegate content;

    // ---- Edit-mode drag overlay ----
    struct EditOverlay : juce::Component
    {
        struct Entry { juce::Component* comp; juce::String name; };
        juce::Array<Entry> entries;
        int  activeIdx = -1;
        bool resizing  = false;
        juce::Point<int>     dragStart;
        juce::Rectangle<int> startBounds;
        std::function<void()> onChange;
        std::function<void(const juce::MouseEvent&)> onRightClick;

        void paint(juce::Graphics&) override;
        bool hitTest(int x, int y) override;
        void mouseDown(const juce::MouseEvent&) override;
        void mouseDrag(const juce::MouseEvent&) override;
        void mouseUp(const juce::MouseEvent&) override;
    };
    EditOverlay editOverlay_;
    PaintDelegate footerComp_;
    PaintDelegate mainPanel_;
    PaintDelegate glowLayer_;     // casts lit meter colours onto the brass
    PaintDelegate valueDisplay_;  // dB readout in the info-line centre cube
    PaintDelegate inDisplay_;     // far-left screen: input level (dB)
    PaintDelegate outDisplay_;    // far-right screen: output level (dB)
    PaintDelegate modeScreen_;    // 2nd screen (IN<->GAIN): STEREO/MONO toggle

    struct ClickZone : juce::Component
    {
        std::function<void()> fn;
        ClickZone() { setMouseCursor(juce::MouseCursor::PointingHandCursor); }
        void mouseDown(const juce::MouseEvent&) override { if (fn) fn(); }
    };

    juce::AudioProcessorValueTreeState& apvts;
    GainAnalysisSource& analysis;
    Gain76LookAndFeel lnf;

    // Gain knob: mouse wheel steps a fixed 0.5 dB per notch (the default
    // proportional step gave ~1.7 dB per notch).
    struct ScrollKnob : juce::Slider
    {
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
        {
            if (! isEnabled()) { juce::Slider::mouseWheelMove(e, w); return; }
            const float dy = w.isReversed ? -w.deltaY : w.deltaY;
            if      (dy > 0.0f) setValue(getValue() + 0.5, juce::sendNotificationSync);
            else if (dy < 0.0f) setValue(getValue() - 0.5, juce::sendNotificationSync);
        }
    };
    ScrollKnob gainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;

    juce::ComboBox modeBox, osBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttach, osAttach;

    Gain76PowerKey powerBtn;
    VerticalMeter inMeter { false }, outMeter { true };

    ClickZone peakReset;
    float heldPeakDb = -120.0f;

    // In/out numeric screens: held peak level (dB). Holds the running max and
    // resets when the input goes silent so each new signal starts fresh.
    float inPeakDb_  = -60.0f;
    float outPeakDb_ = -60.0f;

    // Clip-triggered screen-glitch state for the dB readout.
    float        glitchT_      = 0.0f;     // remaining glitch time (seconds)
    bool         wasClipping_  = false;    // rising-edge detect on 0 dB output
    juce::Random glitchRng_;

    // Blender-rendered static sprites (all full-frame @2x, 1640x1440).
    // brassTexture_ holds the faceplate background; the rest are overlay layers.
    juce::Image brassTexture_;     // faceplate.png
    juce::Image meterLImg_;        // meter_housing_left.png
    juce::Image meterRImg_;        // meter_housing_right.png
    juce::Image infolineImg_;      // infoline_housing.png
    juce::Image logoImg_;          // text_altered_audio.png
    juce::Image gainTextImg_;      // text_gain76.png
    juce::Image glyphImg_[13];     // embossed brass digits: 0-9, '.', '+', '-'

    LayoutStore layout_;
    bool        editMode_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainEditor)
};
