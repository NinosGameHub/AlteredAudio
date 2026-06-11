#pragma once
#include <JuceHeader.h>

namespace AaColor
{
    const juce::Colour bg          { 0xFFE7E2D6 };  // dirty warm white (ABS plastic)
    const juce::Colour surface     { 0xFFEDE8DC };  // slightly lighter card
    const juce::Colour surfaceAlt  { 0xFFD9D4C8 };  // pressed/alt surface
    const juce::Colour border      { 0xFFB2ADA1 };  // warm grey separator
    const juce::Colour textPrimary { 0xFF1C1815 };  // near-black
    const juce::Colour textSecond  { 0xFF7A746E };  // warm mid-grey

    // Category accent colors
    const juce::Colour catDynamics   { 0xFFC4621D };  // burnt orange
    const juce::Colour catSpatial    { 0xFF5E8480 };  // desaturated teal
    const juce::Colour catModulation { 0xFFAA8B10 };  // mustard yellow
    const juce::Colour catTime       { 0xFF7A5438 };  // warm brown
    const juce::Colour catFilterEQ   { 0xFF3A6694 };  // space-age blue

    const juce::Colour accent      = catDynamics;
    const juce::Colour accentFaint { 0xFFEED5C0 };
    const juce::Colour green       { 0xFF4CA84C };
    const juce::Colour inactive    { 0xFFB2ADA1 };

    // CRT display
    const juce::Colour crtBg    { 0xFF060504 };
    const juce::Colour crtAmber { 0xFFE4A210 };

    // Knob
    const juce::Colour knobBody { 0xFFE4DFD0 };  // dirty cream
    const juce::Colour knobLine { 0xFF1C1815 };  // near-black indicator
    const juce::Colour knobDot  { 0xFF8C8680 };  // range marker dots
}

class AaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AaLookAndFeel()
    {
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

        setColour(juce::Label::textColourId,       AaColor::textPrimary);
        setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

        setColour(juce::TextButton::buttonColourId,   AaColor::surfaceAlt);
        setColour(juce::TextButton::buttonOnColourId, AaColor::accentFaint);
        setColour(juce::TextButton::textColourOffId,  AaColor::textPrimary);
        setColour(juce::TextButton::textColourOnId,   AaColor::accent);

        setColour(juce::ToggleButton::textColourId,         AaColor::textPrimary);
        setColour(juce::ToggleButton::tickColourId,         AaColor::accent);
        setColour(juce::ToggleButton::tickDisabledColourId, AaColor::border);

        setColour(juce::ComboBox::backgroundColourId, AaColor::surface);
        setColour(juce::ComboBox::textColourId,       AaColor::textPrimary);
        setColour(juce::ComboBox::outlineColourId,    AaColor::border);
        setColour(juce::ComboBox::arrowColourId,      AaColor::textSecond);
        setColour(juce::ComboBox::buttonColourId,     AaColor::surfaceAlt);

        setColour(juce::PopupMenu::backgroundColourId,            AaColor::surface);
        setColour(juce::PopupMenu::textColourId,                  AaColor::textPrimary);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, AaColor::accentFaint);
        setColour(juce::PopupMenu::highlightedTextColourId,       AaColor::accent);
        setColour(juce::PopupMenu::headerTextColourId,            AaColor::textSecond);

        setColour(juce::AlertWindow::backgroundColourId, AaColor::surface);
        setColour(juce::AlertWindow::textColourId,       AaColor::textPrimary);
    }

    // ---- Flat cream knob — Braun SK / Moog modular inspired ----------------
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider& /*slider*/) override
    {
        const float cx = (float)x + (float)width  * 0.5f;
        const float cy = (float)y + (float)height * 0.5f;
        const float r  = std::min((float)width, (float)height) * 0.40f;

        // Knob body: flat dirty cream disc
        g.setColour(AaColor::knobBody);
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);

        // Subtle border ring
        g.setColour(AaColor::border);
        g.drawEllipse(cx - r + 0.5f, cy - r + 0.5f, r * 2.0f - 1.0f, r * 2.0f - 1.0f, 1.0f);

        // Range dots at 7-o'clock (startAngle) and 5-o'clock (endAngle)
        const float dotOffset = r + 5.5f;
        const auto drawRangeDot = [&](float angle)
        {
            const float dx = cx + dotOffset * std::sin(angle);
            const float dy = cy - dotOffset * std::cos(angle);
            g.setColour(AaColor::knobDot);
            g.fillEllipse(dx - 1.5f, dy - 1.5f, 3.0f, 3.0f);
        };
        drawRangeDot(startAngle);
        drawRangeDot(endAngle);

        // Indicator line from center out — near-black, 2.5px, round cap
        const float toAngle   = startAngle + sliderPos * (endAngle - startAngle);
        const float lineInner = r * 0.20f;
        const float lineOuter = r * 0.86f;
        const float lx1 = cx + lineInner * std::sin(toAngle);
        const float ly1 = cy - lineInner * std::cos(toAngle);
        const float lx2 = cx + lineOuter * std::sin(toAngle);
        const float ly2 = cy - lineOuter * std::cos(toAngle);

        juce::Path line;
        line.startNewSubPath(lx1, ly1);
        line.lineTo(lx2, ly2);
        g.setColour(AaColor::knobLine);
        g.strokePath(line, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // ---- Bypass toggle: pill switch -----------------------------------------
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b,
                          bool, bool) override
    {
        const bool on = b.getToggleState();
        const float bw = 32.0f, bh = 18.0f;
        const float bx = 2.0f;
        const float by = (float)b.getHeight() * 0.5f - bh * 0.5f;

        g.setColour(on ? AaColor::accent : AaColor::border);
        g.fillRoundedRectangle(bx, by, bw, bh, bh * 0.5f);

        const float thumbX = on ? (bx + bw - bh) : bx;
        g.setColour(AaColor::knobBody);
        g.fillEllipse(thumbX + 1.5f, by + 1.5f, bh - 3.0f, bh - 3.0f);

        if (!b.getButtonText().isEmpty())
        {
            g.setColour(AaColor::textPrimary);
            g.setFont(juce::Font(11.0f));
            g.drawText(b.getButtonText(),
                       juce::Rectangle<float>(bx + bw + 6.0f, by, 80.0f, bh),
                       juce::Justification::centredLeft);
        }
    }

    // ---- ComboBox: flat, warm rounded rect ----------------------------------
    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool, int, int, int, int, juce::ComboBox&) override
    {
        const juce::Rectangle<float> box(0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f);
        g.setColour(AaColor::surface);
        g.fillRoundedRectangle(box, 4.0f);
        g.setColour(AaColor::border);
        g.drawRoundedRectangle(box, 4.0f, 1.0f);

        const float arrowX = (float)width - 14.0f;
        const float arrowY = (float)height * 0.5f;
        juce::Path chevron;
        chevron.startNewSubPath(arrowX - 4.0f, arrowY - 2.5f);
        chevron.lineTo(arrowX, arrowY + 2.5f);
        chevron.lineTo(arrowX + 4.0f, arrowY - 2.5f);
        g.setColour(AaColor::textSecond);
        g.strokePath(chevron, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override { return juce::Font(12.0f); }
    juce::Font getLabelFont   (juce::Label&)   override { return juce::Font(10.5f); }
};
