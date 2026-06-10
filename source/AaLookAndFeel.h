#pragma once
#include <JuceHeader.h>

// Apple-inspired color palette
namespace AaColor
{
    const juce::Colour bg          { 0xFFF5F5F7 };  // apple.com background
    const juce::Colour surface     { 0xFFFFFFFF };  // white card
    const juce::Colour surfaceAlt  { 0xFFF2F2F7 };  // very light grey
    const juce::Colour border      { 0xFFD2D2D7 };  // separator
    const juce::Colour textPrimary { 0xFF1D1D1F };  // near-black
    const juce::Colour textSecond  { 0xFF6E6E73 };  // mid grey
    const juce::Colour accent      { 0xFF0071E3 };  // apple blue
    const juce::Colour accentFaint { 0xFFEBF3FE };  // light blue tint
    const juce::Colour green       { 0xFF30D158 };  // apple green  (active)
    const juce::Colour inactive    { 0xFFAEAEB2 };  // grey (bypassed)
}

class AaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AaLookAndFeel()
    {
        using C = juce::LookAndFeel_V4::ColourScheme;
        setColourScheme(getLightColourScheme());

        setColour(juce::ResizableWindow::backgroundColourId,   AaColor::bg);
        setColour(juce::Slider::rotarySliderFillColourId,      AaColor::accent);
        setColour(juce::Slider::rotarySliderOutlineColourId,   AaColor::border);
        setColour(juce::Slider::trackColourId,                 AaColor::border);
        setColour(juce::Slider::thumbColourId,                 AaColor::accent);
        setColour(juce::Slider::textBoxTextColourId,           AaColor::textSecond);
        setColour(juce::Slider::textBoxBackgroundColourId,     juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId,        juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxHighlightColourId,      AaColor::accentFaint);

        setColour(juce::Label::textColourId,                   AaColor::textPrimary);
        setColour(juce::Label::backgroundColourId,             juce::Colours::transparentBlack);

        setColour(juce::TextButton::buttonColourId,            AaColor::surfaceAlt);
        setColour(juce::TextButton::buttonOnColourId,          AaColor::accentFaint);
        setColour(juce::TextButton::textColourOffId,           AaColor::textPrimary);
        setColour(juce::TextButton::textColourOnId,            AaColor::accent);

        setColour(juce::ToggleButton::textColourId,            AaColor::textPrimary);
        setColour(juce::ToggleButton::tickColourId,            AaColor::accent);
        setColour(juce::ToggleButton::tickDisabledColourId,    AaColor::border);

        setColour(juce::ComboBox::backgroundColourId,          AaColor::surface);
        setColour(juce::ComboBox::textColourId,                AaColor::textPrimary);
        setColour(juce::ComboBox::outlineColourId,             AaColor::border);
        setColour(juce::ComboBox::arrowColourId,               AaColor::textSecond);
        setColour(juce::ComboBox::buttonColourId,              AaColor::surfaceAlt);

        setColour(juce::PopupMenu::backgroundColourId,            AaColor::surface);
        setColour(juce::PopupMenu::textColourId,                  AaColor::textPrimary);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, AaColor::accentFaint);
        setColour(juce::PopupMenu::highlightedTextColourId,       AaColor::accent);
        setColour(juce::PopupMenu::headerTextColourId,            AaColor::textSecond);

        setColour(juce::AlertWindow::backgroundColourId,  AaColor::surface);
        setColour(juce::AlertWindow::textColourId,        AaColor::textPrimary);
    }

    // ---- Rotary Slider: thin arc, dot thumb, blue value arc ----
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider& /*slider*/) override
    {
        const float cx = (float)x + (float)width  * 0.5f;
        const float cy = (float)y + (float)height * 0.5f;
        const float r  = std::min((float)width, (float)height) * 0.38f;
        const float toAngle = startAngle + sliderPos * (endAngle - startAngle);

        // Track (full arc)
        juce::Path track;
        track.addCentredArc(cx, cy, r, r, 0.0f, startAngle, endAngle, true);
        g.setColour(AaColor::border);
        g.strokePath(track, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        // Value arc (blue, start → current value)
        if (sliderPos > 0.001f)
        {
            juce::Path value;
            value.addCentredArc(cx, cy, r, r, 0.0f, startAngle, toAngle, true);
            g.setColour(AaColor::accent);
            g.strokePath(value, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }

        // Thumb dot on arc
        const float tx = cx + r * std::sin(toAngle);
        const float ty = cy - r * std::cos(toAngle);
        g.setColour(sliderPos > 0.001f ? AaColor::accent : AaColor::border);
        g.fillEllipse(tx - 4.0f, ty - 4.0f, 8.0f, 8.0f);
    }

    // ---- Toggle button: pill-shaped (for bypass / active / auto-gain) ----
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b,
                          bool, bool) override
    {
        const bool on = b.getToggleState();
        const float bw = 32.0f, bh = 18.0f;
        const auto bounds = b.getLocalBounds().toFloat();
        const float bx = 2.0f;
        const float by = bounds.getCentreY() - bh * 0.5f;

        // Pill background
        g.setColour(on ? AaColor::green : AaColor::border);
        g.fillRoundedRectangle(bx, by, bw, bh, bh * 0.5f);

        // Thumb
        const float thumbX = on ? (bx + bw - bh) : bx;
        g.setColour(AaColor::surface);
        g.fillEllipse(thumbX + 1.5f, by + 1.5f, bh - 3.0f, bh - 3.0f);

        // Label text
        if (!b.getButtonText().isEmpty())
        {
            g.setColour(AaColor::textPrimary);
            g.setFont(juce::Font(11.0f));
            g.drawText(b.getButtonText(),
                       juce::Rectangle<float>(bx + bw + 6.0f, by, 80.0f, bh),
                       juce::Justification::centredLeft);
        }
    }

    // ---- ComboBox: flat, rounded corners, chevron arrow ----
    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool, int, int, int, int, juce::ComboBox&) override
    {
        const juce::Rectangle<float> box(0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f);
        g.setColour(AaColor::surface);
        g.fillRoundedRectangle(box, 6.0f);
        g.setColour(AaColor::border);
        g.drawRoundedRectangle(box, 6.0f, 1.0f);

        // Chevron
        const float cx = (float)width - 14.0f;
        const float cy = (float)height * 0.5f;
        juce::Path chevron;
        chevron.startNewSubPath(cx - 4.0f, cy - 2.5f);
        chevron.lineTo(cx, cy + 2.5f);
        chevron.lineTo(cx + 4.0f, cy - 2.5f);
        g.setColour(AaColor::textSecond);
        g.strokePath(chevron, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(13.0f);
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(11.0f);
    }
};
