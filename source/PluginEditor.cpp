#include "PluginEditor.h"

AlteredAudioEditor::AlteredAudioEditor(AlteredAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      chainStrip(p)
{
    setLookAndFeel(&laf);

    // Title label
    titleLabel.setText("ALTERED AUDIO", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, AaColor::textPrimary);
    addAndMakeVisible(titleLabel);

    // Chain strip
    addAndMakeVisible(chainStrip);
    chainStrip.onModuleSelected = [this](int idx) { showModulePanel(idx); };

    // Show the Clipper panel (index 7) by default so there's something to look at
    showModulePanel(7);
    chainStrip.setSelectedModule(7);

    setResizable(false, false);
    setSize(kWindowW, kWindowH);
}

AlteredAudioEditor::~AlteredAudioEditor()
{
    setLookAndFeel(nullptr);
}

void AlteredAudioEditor::showModulePanel(int moduleIdx)
{
    if (currentPanel)
        removeChildComponent(currentPanel.get());

    currentPanel = createModulePanel(processorRef, moduleIdx);
    addAndMakeVisible(*currentPanel);

    // Position the panel in the detail area
    const int detailY = kHeaderH + kStripH;
    const int detailH = kWindowH - detailY;
    currentPanel->setBounds(0, detailY, kWindowW, detailH);
}

void AlteredAudioEditor::paint(juce::Graphics& g)
{
    g.fillAll(AaColor::bg);

    // Header background
    const auto header = getLocalBounds().removeFromTop(kHeaderH).toFloat();
    g.setColour(AaColor::surface);
    g.fillRect(header);
    g.setColour(AaColor::border);
    g.fillRect(0.0f, (float)kHeaderH - 1.0f, (float)getWidth(), 1.0f);

    // Strip/panel separator — already drawn by the strip's bottom border
}

void AlteredAudioEditor::resized()
{
    // Header
    titleLabel.setBounds(16, 0, 240, kHeaderH);

    // Chain strip immediately below header
    chainStrip.setBounds(0, kHeaderH, kWindowW, kStripH);

    // Detail panel (repositioned here in case of future resize support)
    if (currentPanel)
    {
        const int detailY = kHeaderH + kStripH;
        currentPanel->setBounds(0, detailY, kWindowW, kWindowH - detailY);
    }
}
