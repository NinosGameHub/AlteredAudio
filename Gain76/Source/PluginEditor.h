#pragma once
#include <JuceHeader.h>
#include "AlteredAudioLookAndFeel.h"

// Forward declaration — full definition is in GainProcessor.cpp before this
// header is included, so all method bodies below have access to the complete type.
class Gain76Processor;

//==============================================================================
// VU Meter Component
//==============================================================================
class VUMeterComponent : public juce::Component, public juce::Timer
{
public:
    VUMeterComponent (Gain76Processor& p, bool isInput)
        : processor (p), input (isInput)
    {
        startTimerHz (30);
    }

    void timerCallback() override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        float level = input ? processor.getInputLevel() : processor.getOutputLevel();
        laf.paintVUMeter (g, getLocalBounds(), level);
    }

private:
    Gain76Processor& processor;
    AlteredAudioLookAndFeel laf;
    bool input;
};

//==============================================================================
// Stereo Meter Bar (horizontal L/R strip at bottom)
//==============================================================================
class StereoMeterBar : public juce::Component, public juce::Timer
{
public:
    StereoMeterBar (Gain76Processor& p) : processor (p)
    {
        startTimerHz (30);
    }

    void timerCallback() override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour (juce::Colour (AlteredAudioLookAndFeel::COL_INSET_BG));
        g.fillRoundedRectangle (bounds.toFloat(), 3.0f);

        int numSegments = 40;
        float segW = ((float) bounds.getWidth() * 0.48f) / numSegments;
        float segH = (float) bounds.getHeight() - 6.0f;
        float segGap = 1.5f;
        float levelL = processor.getInputLevel();
        float levelR = processor.getOutputLevel();

        for (int i = 0; i < numSegments; ++i)
        {
            float segLevel = (float) i / numSegments;
            bool activeL = levelL > segLevel;
            bool activeR = levelR > segLevel;

            auto colActive = segLevel > 0.85f
                ? juce::Colour (0xFFFF3B1A)
                : juce::Colour (AlteredAudioLookAndFeel::COL_AMBER).brighter (segLevel * 0.4f);
            auto colOff = juce::Colour (AlteredAudioLookAndFeel::COL_METER_OFF);

            float lx = (float) bounds.getX() + 4.0f + i * (segW + segGap);
            g.setColour (activeL ? colActive : colOff);
            g.fillRect (lx, (float) bounds.getY() + 3.0f, segW, segH);

            float rx = (float) bounds.getRight() - 4.0f - (i + 1) * (segW + segGap);
            g.setColour (activeR ? colActive : colOff);
            g.fillRect (rx, (float) bounds.getY() + 3.0f, segW, segH);
        }

        g.setColour (juce::Colour (AlteredAudioLookAndFeel::COL_TEXT_SECONDARY));
        g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f)));
        g.drawText ("L", bounds.getX() - 14, bounds.getY(), 12, bounds.getHeight(),
                    juce::Justification::centredRight);
        g.drawText ("R", bounds.getRight() + 2, bounds.getY(), 12, bounds.getHeight(),
                    juce::Justification::centredLeft);
    }

private:
    Gain76Processor& processor;
};

//==============================================================================
// Main Plugin Editor
//==============================================================================
class Gain76AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    Gain76AudioProcessorEditor (Gain76Processor& p)
        : AudioProcessorEditor (&p), processor (p),
          inputMeter (p, true), outputMeter (p, false),
          stereoBar (p)
    {
        setLookAndFeel (&laf);
        setSize (520, 480);

        gainKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        gainKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        gainKnob.setName ("Gain");
        gainKnob.setRange (-24.0, 24.0, 0.1);
        gainKnob.setValue (0.0);
        addAndMakeVisible (gainKnob);

        gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (processor.getAPVTS(), "gain_db", gainKnob);

        addAndMakeVisible (inputMeter);
        addAndMakeVisible (outputMeter);
        addAndMakeVisible (stereoBar);

        modeCombo.addItem ("STEREO", 1);
        modeCombo.addItem ("MONO", 2);
        modeCombo.addItem ("MID/SIDE", 3);
        modeCombo.setSelectedId (1);
        addAndMakeVisible (modeCombo);

        linkButton.setButtonText ("ON");
        linkButton.setToggleState (true, juce::dontSendNotification);
        addAndMakeVisible (linkButton);

        for (auto* lbl : { &inputLabel, &outputLabel, &modeLabel,
                            &linkLabel, &peakLabel, &lufsLabel, &oversampLabel })
            addAndMakeVisible (lbl);

        inputLabel.setText ("INPUT\ndB", juce::dontSendNotification);
        outputLabel.setText ("OUTPUT\ndB", juce::dontSendNotification);
        modeLabel.setText ("MODE", juce::dontSendNotification);
        linkLabel.setText ("LINK", juce::dontSendNotification);
        peakLabel.setText ("PEAK", juce::dontSendNotification);
        lufsLabel.setText ("LUFS", juce::dontSendNotification);
        oversampLabel.setText ("OVERSAMPLING", juce::dontSendNotification);

        for (auto* lbl : { &inputLabel, &outputLabel, &modeLabel,
                            &linkLabel, &peakLabel, &lufsLabel, &oversampLabel })
            lbl->setJustificationType (juce::Justification::centred);
    }

    ~Gain76AudioProcessorEditor() override
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        laf.paintBackground (g, getWidth(), getHeight());

        g.setColour (juce::Colour (AlteredAudioLookAndFeel::COL_TEXT_PRIMARY));
        g.setFont (juce::Font (juce::FontOptions().withHeight (18.0f).withStyle ("Bold")));
        g.drawText ("ALTERED AUDIO", 20, 14, 200, 22, juce::Justification::centredLeft);

        g.setColour (juce::Colour (AlteredAudioLookAndFeel::COL_AMBER));
        g.setFont (juce::Font (juce::FontOptions().withHeight (11.0f).withStyle ("Bold")));
        g.drawText ("GAIN 76", 20, 36, 100, 14, juce::Justification::centredLeft);

        g.setColour (juce::Colour (AlteredAudioLookAndFeel::COL_AMBER_GLOW));
        g.fillEllipse ((float) getWidth() - 20.0f, 16.0f, 8.0f, 8.0f);

        g.setColour (juce::Colour (AlteredAudioLookAndFeel::COL_PANEL_DARK));
        g.drawRoundedRectangle (mainPanelBounds.toFloat(), 4.0f, 1.0f);
        g.drawRoundedRectangle (bottomStripBounds.toFloat(), 4.0f, 1.0f);

        g.setColour (juce::Colour (AlteredAudioLookAndFeel::COL_TEXT_SECONDARY));
        g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f)));
        g.drawText ("v1.0.1", getLocalBounds().reduced (8), juce::Justification::bottomRight);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        int margin = 12;
        int headerH = 56;

        mainPanelBounds = area.reduced (margin).withTrimmedTop (headerH - margin).withTrimmedBottom (80);
        bottomStripBounds = area.reduced (margin).withTop (mainPanelBounds.getBottom() + 6).withBottom (area.getBottom() - margin);

        int meterW = 28;
        int meterH = mainPanelBounds.getHeight() - 40;
        int meterY = mainPanelBounds.getY() + 30;

        inputMeter.setBounds (mainPanelBounds.getX() + 16, meterY, meterW, meterH);
        outputMeter.setBounds (mainPanelBounds.getRight() - 16 - meterW, meterY, meterW, meterH);

        inputLabel.setBounds (inputMeter.getX() - 4, mainPanelBounds.getY() + 8, meterW + 8, 24);
        outputLabel.setBounds (outputMeter.getX() - 4, mainPanelBounds.getY() + 8, meterW + 8, 24);

        int knobSize = juce::jmin (mainPanelBounds.getWidth() - 140, mainPanelBounds.getHeight() - 20);
        int knobX = mainPanelBounds.getCentreX() - knobSize / 2;
        int knobY = mainPanelBounds.getCentreY() - knobSize / 2;
        gainKnob.setBounds (knobX, knobY, knobSize, knobSize);

        stereoBar.setBounds (mainPanelBounds.getX() + margin,
                             mainPanelBounds.getBottom() - 28,
                             mainPanelBounds.getWidth() - margin * 2, 22);

        int stripY  = bottomStripBounds.getCentreY();
        int ctrlH   = 22;
        int ctrlW   = 70;
        int labelH  = 14;
        int spacing = (bottomStripBounds.getWidth() - 5 * ctrlW) / 6;
        int bx      = bottomStripBounds.getX() + spacing;

        auto placeControl = [&] (juce::Component& ctrl, juce::Label& lbl, int x)
        {
            lbl.setBounds (x, stripY - labelH - 4, ctrlW, labelH);
            ctrl.setBounds (x, stripY, ctrlW, ctrlH);
        };

        placeControl (modeCombo,  modeLabel,  bx);  bx += ctrlW + spacing;
        placeControl (linkButton, linkLabel,   bx);  bx += ctrlW + spacing;

        peakLabel.setBounds   (bx, stripY - labelH - 4, ctrlW, labelH + ctrlH);  bx += ctrlW + spacing;
        lufsLabel.setBounds   (bx, stripY - labelH - 4, ctrlW, labelH + ctrlH);  bx += ctrlW + spacing;
        oversampLabel.setBounds (bx, stripY - labelH - 4, ctrlW, labelH + ctrlH);
    }

private:
    Gain76Processor& processor;
    AlteredAudioLookAndFeel laf;

    juce::Slider gainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    VUMeterComponent inputMeter, outputMeter;
    StereoMeterBar   stereoBar;

    juce::ComboBox    modeCombo;
    juce::ToggleButton linkButton;

    juce::Label inputLabel, outputLabel, modeLabel, linkLabel,
                peakLabel, lufsLabel, oversampLabel;

    juce::Rectangle<int> mainPanelBounds, bottomStripBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Gain76AudioProcessorEditor)
};
