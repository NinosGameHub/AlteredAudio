#include "PluginEditor.h"

AlteredAudioEditor::AlteredAudioEditor(AlteredAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      moduleTileList(p)
{
    setLookAndFeel(&laf);

    titleLabel.setText("ALTERED AUDIO", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, AaColor::textPrimary);
    addAndMakeVisible(titleLabel);

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

    // Open the default module
    showModulePanel(moduleTileList.getSelectedModule());

    setResizable(false, false);
    setSize(kWindowW, kWindowH);
}

AlteredAudioEditor::~AlteredAudioEditor()
{
    setLookAndFeel(nullptr);
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

    // Header bar
    g.setColour(AaColor::surface);
    g.fillRect(0, 0, kWindowW, kHeaderH);
    g.setColour(AaColor::border);
    g.fillRect(0, kHeaderH - 1, kWindowW, 1);

    // Divider between left list and right content
    g.setColour(AaColor::border);
    g.fillRect(kListW - 1, kHeaderH, 1, kWindowH - kHeaderH);

    // Thin line below CRT display
    g.setColour(AaColor::border);
    g.fillRect(kListW, kHeaderH + kCRTH, kWindowW - kListW, 1);
}

void AlteredAudioEditor::resized()
{
    titleLabel.setBounds(kListW + 12, 0, 240, kHeaderH);

    moduleTileList.setBounds(0, kHeaderH, kListW - 1, kWindowH - kHeaderH);

    crtDisplay.setBounds(kListW, kHeaderH, kWindowW - kListW, kCRTH);

    if (currentPanel)
        currentPanel->setBounds(getPanelBounds());
}

juce::Rectangle<int> AlteredAudioEditor::getPanelBounds() const
{
    return { kListW, kHeaderH + kCRTH + 1, kWindowW - kListW, kWindowH - kHeaderH - kCRTH - 1 };
}
