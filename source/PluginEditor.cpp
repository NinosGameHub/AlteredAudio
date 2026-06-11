#include "PluginEditor.h"

AlteredAudioEditor::AlteredAudioEditor(AlteredAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      moduleTileList(p)
{
    setLookAndFeel(&laf);

    // Preset transport
    for (auto* btn : { &prevPresetBtn, &nextPresetBtn })
    {
        btn->setColour(juce::TextButton::buttonColourId,  AaColor::surfaceAlt);
        btn->setColour(juce::TextButton::textColourOffId, AaColor::textSecond);
        addAndMakeVisible(*btn);
    }
    presetLabel.setText("Default", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setFont(juce::Font(12.0f));
    presetLabel.setColour(juce::Label::backgroundColourId, AaColor::bg);
    presetLabel.setColour(juce::Label::outlineColourId,    AaColor::border);
    presetLabel.setColour(juce::Label::textColourId,       AaColor::textPrimary);
    addAndMakeVisible(presetLabel);

    // Channel mode segmented toggle
    stBtn.setClickingTogglesState(false);
    msBtn.setClickingTogglesState(false);
    stBtn.onClick = [this]() {
        processorRef.setChannelMode(ChannelMode::Stereo);
        updateChannelModeButtons();
    };
    msBtn.onClick = [this]() {
        processorRef.setChannelMode(ChannelMode::MidSide);
        updateChannelModeButtons();
    };
    addAndMakeVisible(stBtn);
    addAndMakeVisible(msBtn);
    updateChannelModeButtons();

    addAndMakeVisible(moduleTileList);
    moduleTileList.onModuleSelected = [this](int idx) { showModulePanel(idx); };
    moduleTileList.onOrderChanged   = [this](const std::vector<int>& order) {
        auto& apvts = processorRef.getAPVTS();
        for (int pos = 0; pos < (int)order.size(); ++pos)
        {
            const int logicalIdx = order[pos];
            const char* paramId  = ModuleTileList::modulePosParam(logicalIdx);
            if (auto* param = apvts.getParameter(paramId))
            {
                param->beginChangeGesture();
                param->setValueNotifyingHost(param->convertTo0to1((float)pos));
                param->endChangeGesture();
            }
        }
    };

    addAndMakeVisible(crtDisplay);

    showModulePanel(moduleTileList.getSelectedModule());

    setResizable(false, false);
    setSize(kWindowW, kWindowH);
    startTimerHz(10);
}

AlteredAudioEditor::~AlteredAudioEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void AlteredAudioEditor::timerCallback()
{
    // Keep channel mode buttons in sync if changed from outside
    updateChannelModeButtons();
}

void AlteredAudioEditor::updateChannelModeButtons()
{
    const bool isStereo = (processorRef.getChannelMode() == ChannelMode::Stereo);

    // Selected: catDynamics fill + white text; unselected: surfaceAlt + textSecond
    stBtn.setColour(juce::TextButton::buttonColourId,
                    isStereo ? AaColor::catDynamics : AaColor::surfaceAlt);
    stBtn.setColour(juce::TextButton::textColourOffId,
                    isStereo ? juce::Colours::white : AaColor::textSecond);

    msBtn.setColour(juce::TextButton::buttonColourId,
                    !isStereo ? AaColor::catDynamics : AaColor::surfaceAlt);
    msBtn.setColour(juce::TextButton::textColourOffId,
                    !isStereo ? juce::Colours::white : AaColor::textSecond);
}

void AlteredAudioEditor::showModulePanel(int moduleIdx)
{
    moduleTileList.setSelectedModule(moduleIdx);

    if (currentPanel)
        removeChildComponent(currentPanel.get());

    currentPanel = createModulePanel(processorRef, moduleIdx);
    addAndMakeVisible(*currentPanel);

    if (auto bounds = getPanelBounds(); !bounds.isEmpty())
        currentPanel->setBounds(bounds);
}

void AlteredAudioEditor::paint(juce::Graphics& g)
{
    g.fillAll(AaColor::bg);

    // Header fill + bottom border
    g.setColour(AaColor::surface);
    g.fillRect(0, 0, kWindowW, kHeaderH);
    g.setColour(AaColor::border);
    g.fillRect(0, kHeaderH - 1, kWindowW, 1);

    // Left block — brand dot + wordmark
    const float dotX = 14.f, dotY = (float)kHeaderH * 0.5f, dotR = 3.5f;
    g.setColour(AaColor::catDynamics);
    g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.f, dotR * 2.f);

    const int textX = (int)(dotX + dotR + 6.f);
    const int textW = kListW - (int)(dotX + dotR + 10.f);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.setColour(AaColor::textPrimary);
    g.drawText("ALTERED AUDIO", textX, 4, textW, 16, juce::Justification::centredLeft, false);
    g.setFont(juce::Font(9.5f));
    g.setColour(AaColor::textSecond);
    g.drawText("v" + juce::String(JucePlugin_VersionString), textX, 21, textW, 14,
               juce::Justification::centredLeft, false);

    // Divider between list and content
    g.setColour(AaColor::border);
    g.fillRect(kListW - 1, kHeaderH, 1, kWindowH - kHeaderH);

    // CRT bottom border
    g.fillRect(kListW, kHeaderH + kCRTH, kWindowW - kListW, 1);
}

void AlteredAudioEditor::resized()
{
    moduleTileList.setBounds(0, kHeaderH, kListW - 1, kWindowH - kHeaderH);
    crtDisplay.setBounds(kListW, kHeaderH, kWindowW - kListW, kCRTH);

    if (currentPanel)
        currentPanel->setBounds(getPanelBounds());

    // Header controls layout (right section: kListW → kWindowW)
    constexpr int cy       = (kHeaderH - 24) / 2;  // vertical center for 24px-tall items
    constexpr int btnW     = 24;
    constexpr int presetW  = 180;
    constexpr int modeW    = 40;
    constexpr int modeGap  = 0;

    // Preset transport — left side of the right header section
    const int presetX = kListW + 16;
    prevPresetBtn.setBounds(presetX,              cy, btnW,    24);
    presetLabel  .setBounds(presetX + btnW + 4,   cy, presetW, 24);
    nextPresetBtn.setBounds(presetX + btnW + 4 + presetW + 4, cy, btnW, 24);

    // Channel mode — right-aligned before right edge
    const int modeRightX = kWindowW - 16;
    msBtn.setBounds(modeRightX - modeW,              cy, modeW, 24);
    stBtn.setBounds(modeRightX - modeW * 2 - modeGap, cy, modeW, 24);
}

juce::Rectangle<int> AlteredAudioEditor::getPanelBounds() const
{
    return { kListW, kHeaderH + kCRTH + 1, kWindowW - kListW, kWindowH - kHeaderH - kCRTH - 1 };
}
