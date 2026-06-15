#pragma once
#include <JuceHeader.h>

//==============================================================================
// AlteredAudioLookAndFeel
// Vintage worn hardware aesthetic — aged cream panels, bakelite knobs, amber VU
//==============================================================================

class AlteredAudioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==========================================================================
    // Colour palette
    //==========================================================================
    static constexpr uint32_t COL_PANEL_BG      = 0xFFD4C9A8; // aged cream
    static constexpr uint32_t COL_PANEL_DARK     = 0xFF9E9278; // shadowed areas
    static constexpr uint32_t COL_KNOB_FACE      = 0xFFB8A98A; // bakelite knob
    static constexpr uint32_t COL_KNOB_RIM       = 0xFF7A6E5F; // worn metal rim
    static constexpr uint32_t COL_AMBER          = 0xFFD4820A; // amber accent
    static constexpr uint32_t COL_AMBER_GLOW     = 0xFFFF9F1C; // bright amber glow
    static constexpr uint32_t COL_METER_OFF      = 0xFF2A2520; // dark meter segment
    static constexpr uint32_t COL_TEXT_PRIMARY   = 0xFF3A3228; // dark warm text
    static constexpr uint32_t COL_TEXT_SECONDARY = 0xFF8A7D6A; // muted label text
    static constexpr uint32_t COL_INSET_BG       = 0xFF1E1C18; // recessed panel

    AlteredAudioLookAndFeel()
    {
        // Seed scratch positions once — reused every repaint
        juce::Random rng (42);
        for (auto& s : scratches)
        {
            s.x1 = rng.nextFloat();
            s.y1 = rng.nextFloat();
            s.len = 0.02f + rng.nextFloat() * 0.06f;
            s.angle = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            s.alpha = 0.03f + rng.nextFloat() * 0.05f;
        }
    }

    //==========================================================================
    // Background — aged cream panel with grain and vignette
    //==========================================================================
    void drawDocumentWindowTitleBar (juce::DocumentWindow&, juce::Graphics&,
                                     int, int, int, int,
                                     const juce::Image*, bool) override {}

    void paintBackground (juce::Graphics& g, int width, int height)
    {
        // Base cream fill
        g.fillAll (juce::Colour (COL_PANEL_BG));

        // Grain texture — randomized dots at very low alpha
        {
            juce::Random rng (99);
            g.setColour (juce::Colours::black.withAlpha (0.04f));
            for (int i = 0; i < (width * height) / 12; ++i)
            {
                float x = rng.nextFloat() * (float) width;
                float y = rng.nextFloat() * (float) height;
                g.fillRect (juce::Rectangle<float> (x, y, 1.0f, 1.0f));
            }
        }

        // Scratches
        for (auto& s : scratches)
        {
            float x1 = s.x1 * (float) width;
            float y1 = s.y1 * (float) height;
            float dx = std::cos (s.angle) * s.len * (float) width;
            float dy = std::sin (s.angle) * s.len * (float) height;
            g.setColour (juce::Colours::black.withAlpha (s.alpha));
            g.drawLine (x1, y1, x1 + dx, y1 + dy, 0.5f);
        }

        // Vignette — radial gradient dark edges
        {
            juce::ColourGradient vignette (
                juce::Colours::transparentBlack,
                (float) width * 0.5f, (float) height * 0.5f,
                juce::Colours::black.withAlpha (0.35f),
                0.0f, 0.0f,
                true); // radial
            g.setGradientFill (vignette);
            g.fillRect (0, 0, width, height);
        }
    }

    //==========================================================================
    // Rotary knob — bakelite face, worn metal rim, amber dot indicator
    //==========================================================================
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

        float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // --- Outer ring tick marks ---
        {
            int numTicks = 21;
            for (int i = 0; i <= numTicks; ++i)
            {
                float tickAngle = rotaryStartAngle + ((float) i / numTicks) * (rotaryEndAngle - rotaryStartAngle);
                float innerR = radius + 4.0f;
                float outerR = radius + (i % 5 == 0 ? 12.0f : 7.0f);
                float tx1 = cx + std::cos (tickAngle - juce::MathConstants<float>::halfPi) * innerR;
                float ty1 = cy + std::sin (tickAngle - juce::MathConstants<float>::halfPi) * innerR;
                float tx2 = cx + std::cos (tickAngle - juce::MathConstants<float>::halfPi) * outerR;
                float ty2 = cy + std::sin (tickAngle - juce::MathConstants<float>::halfPi) * outerR;
                g.setColour (juce::Colour (COL_TEXT_SECONDARY));
                g.drawLine (tx1, ty1, tx2, ty2, i % 5 == 0 ? 1.5f : 0.8f);
            }
        }

        // --- Metal rim (outermost circle) ---
        {
            juce::ColourGradient rimGrad (
                juce::Colour (0xFFAA9E8E), cx - radius, cy - radius,
                juce::Colour (0xFF5A5040), cx + radius, cy + radius,
                false);
            g.setGradientFill (rimGrad);
            g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        }

        // --- Knob face (bakelite) ---
        float faceRadius = radius * 0.88f;
        {
            juce::ColourGradient faceGrad (
                juce::Colour (COL_KNOB_FACE).brighter (0.1f), cx - faceRadius * 0.3f, cy - faceRadius * 0.3f,
                juce::Colour (COL_KNOB_FACE).darker (0.2f), cx + faceRadius * 0.3f, cy + faceRadius * 0.3f,
                false);
            g.setGradientFill (faceGrad);
            g.fillEllipse (cx - faceRadius, cy - faceRadius, faceRadius * 2.0f, faceRadius * 2.0f);
        }

        // --- Subtle radial brush texture on face ---
        {
            juce::Random rng (77);
            for (int i = 0; i < 12; ++i)
            {
                float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
                float r1 = faceRadius * 0.3f;
                float r2 = faceRadius * 0.9f;
                g.setColour (juce::Colours::black.withAlpha (0.03f));
                g.drawLine (
                    cx + std::cos (a) * r1, cy + std::sin (a) * r1,
                    cx + std::cos (a) * r2, cy + std::sin (a) * r2,
                    0.5f);
            }
        }

        // --- Inset value display (recessed oval) ---
        {
            float insetW = faceRadius * 0.9f;
            float insetH = faceRadius * 0.4f;
            float insetY = cy + faceRadius * 0.15f;
            juce::Rectangle<float> insetRect (cx - insetW * 0.5f, insetY - insetH * 0.5f, insetW, insetH);
            g.setColour (juce::Colour (COL_INSET_BG));
            g.fillRoundedRectangle (insetRect, 4.0f);
            g.setColour (juce::Colour (COL_PANEL_DARK));
            g.drawRoundedRectangle (insetRect.expanded (1.0f), 4.0f, 1.0f);

            // Value text
            juce::String valueText = juce::String (slider.getValue(), 1);
            g.setColour (juce::Colour (COL_AMBER_GLOW));
            g.setFont (juce::Font (juce::FontOptions().withHeight (insetH * 0.55f)
                                                       .withStyle ("Bold")));
            g.drawFittedText (valueText, insetRect.toNearestInt(), juce::Justification::centred, 1);

            // "GAIN · DB" sub-label
            g.setColour (juce::Colour (COL_TEXT_SECONDARY));
            g.setFont (juce::Font (juce::FontOptions().withHeight (insetH * 0.28f)));
            juce::Rectangle<float> subLabel (insetRect.getX(), insetRect.getBottom() + 2.0f,
                                             insetRect.getWidth(), 12.0f);
            g.drawFittedText (slider.getName().toUpperCase() + " \xc2\xb7 DB",
                              subLabel.toNearestInt(), juce::Justification::centred, 1);
        }

        // --- Amber indicator dot ---
        {
            float dotRadius = faceRadius * 0.07f;
            float dotDist   = faceRadius * 0.72f;
            float dotX = cx + std::cos (angle - juce::MathConstants<float>::halfPi) * dotDist;
            float dotY = cy + std::sin (angle - juce::MathConstants<float>::halfPi) * dotDist;

            // Glow halo
            juce::ColourGradient glow (
                juce::Colour (COL_AMBER_GLOW).withAlpha (0.6f), dotX, dotY,
                juce::Colours::transparentBlack, dotX + dotRadius * 3.5f, dotY,
                true);
            g.setGradientFill (glow);
            g.fillEllipse (dotX - dotRadius * 3.5f, dotY - dotRadius * 3.5f,
                           dotRadius * 7.0f, dotRadius * 7.0f);

            // Solid dot
            g.setColour (juce::Colour (COL_AMBER_GLOW));
            g.fillEllipse (dotX - dotRadius, dotY - dotRadius,
                           dotRadius * 2.0f, dotRadius * 2.0f);
        }

        // --- Rim shadow (inner shadow illusion) ---
        g.setColour (juce::Colours::black.withAlpha (0.2f));
        g.drawEllipse (cx - faceRadius, cy - faceRadius,
                       faceRadius * 2.0f, faceRadius * 2.0f, 1.5f);
    }

    //==========================================================================
    // VU meter segment bar — amber segmented strips
    // Call paintVUMeter() from your meter Component's paint() method
    //==========================================================================
    void paintVUMeter (juce::Graphics& g, juce::Rectangle<int> bounds, float level)
    {
        // level: 0.0 to 1.0
        int numSegments = 24;
        float segGap = 2.0f;
        float segH = ((float) bounds.getHeight() - segGap * (numSegments - 1)) / numSegments;
        float segW = (float) bounds.getWidth();

        for (int i = 0; i < numSegments; ++i)
        {
            float segY = (float) bounds.getBottom() - (i + 1) * (segH + segGap) + segGap;
            float segLevel = (float) i / (float) numSegments;
            bool active = level > segLevel;

            juce::Colour col;
            if (!active)
            {
                col = juce::Colour (COL_METER_OFF);
            }
            else if (segLevel > 0.85f)
            {
                col = juce::Colour (0xFFFF3B1A); // red top
            }
            else if (segLevel > 0.70f)
            {
                col = juce::Colour (0xFFFFAA00); // orange
            }
            else
            {
                // amber gradient bottom to top
                col = juce::Colour (COL_AMBER).brighter (segLevel * 0.5f);
            }

            g.setColour (col);
            g.fillRect (juce::Rectangle<float> ((float) bounds.getX(), segY, segW, segH));
        }

        // Inset border
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.drawRect (bounds.toFloat(), 1.0f);
    }

    //==========================================================================
    // Label
    //==========================================================================
    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll (juce::Colours::transparentBlack);
        g.setColour (juce::Colour (COL_TEXT_SECONDARY));
        g.setFont (getLabelFont (label));
        g.drawFittedText (label.getText(), label.getLocalBounds(),
                          label.getJustificationType(), 1, 1.0f);
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return juce::Font (juce::FontOptions().withHeight (11.0f));
    }

    //==========================================================================
    // ComboBox — flat dark style matching bottom strip
    //==========================================================================
    void drawComboBox (juce::Graphics& g, int width, int height, bool,
                       int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();
        g.setColour (juce::Colour (COL_INSET_BG));
        g.fillRoundedRectangle (bounds, 2.0f);
        g.setColour (juce::Colour (COL_PANEL_DARK));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 2.0f, 1.0f);

        // Arrow
        float arrowX = (float) width - 14.0f;
        float arrowY = (float) height * 0.5f;
        juce::Path arrow;
        arrow.addTriangle (arrowX, arrowY - 3.0f,
                           arrowX + 6.0f, arrowY - 3.0f,
                           arrowX + 3.0f, arrowY + 3.0f);
        g.setColour (juce::Colour (COL_TEXT_SECONDARY));
        g.fillPath (arrow);
    }

    void drawComboBoxTextWhenNothingSelected (juce::Graphics& g,
                                              juce::ComboBox& box,
                                              juce::Label& label) override
    {
        g.setColour (juce::Colour (COL_TEXT_SECONDARY));
        g.setFont (getLabelFont (label));
        g.drawFittedText (box.getTextWhenNothingSelected(),
                          label.getBounds(), juce::Justification::centredLeft, 1);
    }

    //==========================================================================
    // Toggle button — lit amber when ON
    //==========================================================================
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool highlighted, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        bool on = button.getToggleState();

        g.setColour (on ? juce::Colour (COL_AMBER) : juce::Colour (COL_INSET_BG));
        g.fillRoundedRectangle (bounds, 2.0f);
        g.setColour (juce::Colour (COL_PANEL_DARK));
        g.drawRoundedRectangle (bounds, 2.0f, 1.0f);

        g.setColour (on ? juce::Colour (COL_INSET_BG) : juce::Colour (COL_TEXT_SECONDARY));
        g.setFont (juce::Font (juce::FontOptions().withHeight (11.0f).withStyle ("Bold")));
        g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                          juce::Justification::centred, 1);
    }

private:
    struct Scratch { float x1, y1, len, angle, alpha; };
    std::array<Scratch, 18> scratches;
};
