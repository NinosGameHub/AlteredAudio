#include "ChainStrip.h"
#include "AaLookAndFeel.h"

// ---- Static tables (indexed by module index 0..13) -------------------------

const char* ChainStripComponent::moduleName(int i)
{
    static const char* names[] = {
        "FILTER","EQ","GAIN","DELAY","REVERB","WAVE",
        "COMP","CLIP","TRANS","LIM","GATE",
        "CHORUS","FLANGE","PHASER"
    };
    return (i >= 0 && i < 14) ? names[i] : "?";
}

const char* ChainStripComponent::modulePosParamId(int i)
{
    static const char* ids[] = {
        ParamID::posFilter,
        ParamID::posEq,
        ParamID::posGain,
        ParamID::posDelay,
        ParamID::posReverb,
        ParamID::posWaveshaper,
        ParamID::posCompressor,
        ParamID::posClipper,
        ParamID::posTransient,
        ParamID::posLimiter,
        ParamID::posGate,
        ParamID::posChorus,
        ParamID::posFlanger,
        ParamID::posPhaser
    };
    return (i >= 0 && i < 14) ? ids[i] : "";
}

const char* ChainStripComponent::moduleBypassParamId(int i)
{
    static const char* ids[] = {
        ParamID::filterBypass,
        ParamID::eqBypass,
        ParamID::gainBypass,
        ParamID::delayBypass,
        ParamID::reverbBypass,
        ParamID::wsBypass,
        ParamID::compBypass,
        ParamID::clipBypass,
        ParamID::transBypass,
        ParamID::limBypass,
        ParamID::gateBypass,
        ParamID::chorusBypass,
        ParamID::flangerBypass,
        ParamID::phaserBypass
    };
    return (i >= 0 && i < 14) ? ids[i] : "";
}

// ---- Lifecycle ---------------------------------------------------------------

ChainStripComponent::ChainStripComponent(AlteredAudioProcessor& p)
    : proc(p)
{
    startTimerHz(10);  // refresh active indicators
}

ChainStripComponent::~ChainStripComponent()
{
    stopTimer();
}

// ---- Sorted order -----------------------------------------------------------

std::array<int, 14> ChainStripComponent::sortedOrder() const
{
    auto& apvts = proc.getAPVTS();
    std::array<std::pair<int,int>, 14> pairs;
    for (int i = 0; i < 14; ++i)
    {
        const std::atomic<float>* raw = apvts.getRawParameterValue(modulePosParamId(i));
        pairs[i] = { i, raw ? (int)*raw : i };
    }
    std::stable_sort(pairs.begin(), pairs.end(),
        [](const auto& a, const auto& b){ return a.second < b.second; });
    std::array<int, 14> order;
    for (int i = 0; i < 14; ++i)
        order[i] = pairs[i].first;
    return order;
}

// ---- Geometry ---------------------------------------------------------------

juce::Rectangle<float> ChainStripComponent::slotBounds(int slotIdx) const
{
    const float total   = (float)getWidth();
    const float margin  = 4.0f;
    const float usable  = total - 2.0f * margin;
    const float slotW   = usable / 14.0f;
    const float slotH   = (float)getHeight() - 2.0f * margin;
    return { margin + slotIdx * slotW, margin, slotW - 1.0f, slotH };
}

int ChainStripComponent::slotAtX(float x) const
{
    const float total  = (float)getWidth();
    const float margin = 4.0f;
    const float usable = total - 2.0f * margin;
    const float slotW  = usable / 14.0f;
    const int slot = (int)((x - margin) / slotW);
    return juce::jlimit(0, 13, slot);
}

// ---- Drawing ----------------------------------------------------------------

void ChainStripComponent::drawSlot(juce::Graphics& g,
                                    juce::Rectangle<float> r,
                                    const char* name,
                                    bool active, bool selected) const
{
    // Background
    if (selected)
    {
        g.setColour(AaColor::accentFaint);
        g.fillRoundedRectangle(r, 8.0f);
        g.setColour(AaColor::accent.withAlpha(0.5f));
        g.drawRoundedRectangle(r.reduced(0.5f), 8.0f, 1.0f);
    }
    else
    {
        g.setColour(AaColor::surface);
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(AaColor::border);
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 0.8f);
    }

    // Active dot
    const float dotR = 4.0f;
    const float dotX = r.getCentreX();
    const float dotY = r.getBottom() - 14.0f;
    g.setColour(active ? AaColor::green : AaColor::border);
    g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);

    // Module name
    g.setColour(selected ? AaColor::accent : AaColor::textPrimary);
    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.drawText(name, r.withTrimmedBottom(16.0f),
               juce::Justification::centred, false);
}

void ChainStripComponent::paint(juce::Graphics& g)
{
    // Strip background
    g.setColour(AaColor::bg);
    g.fillAll();

    // Bottom separator line
    g.setColour(AaColor::border);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);

    const auto order = sortedOrder();
    auto& apvts = proc.getAPVTS();

    for (int slot = 0; slot < 14; ++slot)
    {
        const int  moduleIdx = order[slot];
        const auto r         = slotBounds(slot);
        const bool active    = [&]() {
            auto* raw = apvts.getRawParameterValue(moduleBypassParamId(moduleIdx));
            return raw ? (*raw < 0.5f) : false;
        }();
        const bool selected = (moduleIdx == selectedModule);
        drawSlot(g, r, moduleName(moduleIdx), active, selected);
    }
}

void ChainStripComponent::resized() {}

// ---- Interaction ------------------------------------------------------------

void ChainStripComponent::mouseDown(const juce::MouseEvent& e)
{
    const int slot       = slotAtX((float)e.x);
    const auto order     = sortedOrder();
    const int moduleIdx  = order[slot];
    selectedModule       = moduleIdx;
    repaint();

    if (onModuleSelected)
        onModuleSelected(moduleIdx);
}

void ChainStripComponent::setSelectedModule(int idx)
{
    selectedModule = idx;
    repaint();
}

void ChainStripComponent::timerCallback()
{
    repaint();  // refresh active indicators when bypass params change
}
