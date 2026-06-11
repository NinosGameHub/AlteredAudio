#include "ModuleTileList.h"
#include "AaLookAndFeel.h"

// ---- Static tables ----------------------------------------------------------

const char* ModuleTileList::moduleName(int i)
{
    static const char* names[kNumModules] = {
        "FILTER","EQ","GAIN","DELAY","REVERB",
        "WAVE","COMP","CLIP","TRANS","LIM",
        "GATE","CHORUS","FLANGE","PHASER","SPATIAL"
    };
    return (i >= 0 && i < kNumModules) ? names[i] : "?";
}

juce::Colour ModuleTileList::moduleColor(int i)
{
    switch (i)
    {
        case  0: return AaColor::catFilterEQ;    // Filter
        case  1: return AaColor::catFilterEQ;    // EQ
        case  2: return AaColor::textSecond;     // Gain — neutral
        case  3: return AaColor::catTime;        // Delay
        case  4: return AaColor::catTime;        // Reverb
        case  5: return AaColor::catDynamics;    // Waveshaper
        case  6: return AaColor::catDynamics;    // Compressor
        case  7: return AaColor::catDynamics;    // Clipper
        case  8: return AaColor::catDynamics;    // Transient Shaper
        case  9: return AaColor::catDynamics;    // Limiter
        case 10: return AaColor::catDynamics;    // Gate
        case 11: return AaColor::catModulation;  // Chorus
        case 12: return AaColor::catModulation;  // Flanger
        case 13: return AaColor::catModulation;  // Phaser
        case 14: return AaColor::catSpatial;     // Spatial
        default: return AaColor::border;
    }
}

const char* ModuleTileList::moduleBypassParam(int i)
{
    static const char* ids[kNumModules] = {
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
        ParamID::phaserBypass,
        ParamID::spatialBypass
    };
    return (i >= 0 && i < kNumModules) ? ids[i] : "";
}

const char* ModuleTileList::modulePosParam(int i)
{
    static const char* ids[kNumModules] = {
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
        ParamID::posPhaser,
        ParamID::posSpatial
    };
    return (i >= 0 && i < kNumModules) ? ids[i] : "";
}

// ---- Lifecycle ---------------------------------------------------------------

ModuleTileList::ModuleTileList(AlteredAudioProcessor& p)
    : proc(p)
{
    moduleOrder.resize(kNumModules);
    std::iota(moduleOrder.begin(), moduleOrder.end(), 0);
    startTimerHz(10);
}

ModuleTileList::~ModuleTileList()
{
    stopTimer();
}

void ModuleTileList::timerCallback()
{
    repaint();  // refresh LED state when bypass params change
}

// ---- Geometry ---------------------------------------------------------------

juce::Rectangle<int> ModuleTileList::tileBounds(int displayPos) const
{
    const int tileH = getHeight() / kNumModules;
    return { 0, displayPos * tileH, getWidth(), tileH };
}

int ModuleTileList::tileAtY(int y) const
{
    const int tileH = getHeight() / kNumModules;
    if (tileH <= 0) return 0;
    return juce::jlimit(0, kNumModules - 1, y / tileH);
}

// ---- Drawing ----------------------------------------------------------------

void ModuleTileList::drawTile(juce::Graphics& g, int displayPos,
                               bool active, bool selected,
                               bool dragging, bool dropTarget) const
{
    const int logicalIdx = moduleOrder[displayPos];
    const auto full      = tileBounds(displayPos);
    const auto fullF     = full.toFloat();
    const juce::Colour cat = moduleColor(logicalIdx);

    // Drop-target: slightly darker warm-grey row background
    if (dropTarget)
    {
        g.setColour(AaColor::surfaceAlt);
        g.fillRect(full);
    }

    // Selection background
    if (selected && !dragging)
    {
        g.setColour(cat.withAlpha(0.14f));
        g.fillRect(full.withTrimmedLeft(4));
    }

    // Drag fade: overlay the tile bg at reduced alpha so content dims
    const float alpha = dragging ? 0.45f : 1.0f;
    if (dragging)
    {
        g.setColour(AaColor::bg.withAlpha(0.5f));
        g.fillRect(full);
    }

    // Category color strip (4px left edge)
    g.setColour(cat.withAlpha(alpha));
    g.fillRect(full.withWidth(4));

    // Number label — shows chain slot (display position + 1)
    const juce::String numStr = juce::String(displayPos + 1).paddedLeft('0', 2);
    g.setColour(AaColor::textSecond.withAlpha(0.7f * alpha));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::plain));
    g.drawText(numStr,
               juce::Rectangle<float>(fullF.getX() + 7.0f, fullF.getY() + 3.0f, 18.0f, 10.0f),
               juce::Justification::centredLeft, false);

    // Module name — centered in tile, logical module's name
    g.setColour((selected ? cat : AaColor::textPrimary).withAlpha(alpha));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(moduleName(logicalIdx),
               full.withTrimmedLeft(6).withTrimmedRight(16),
               juce::Justification::centred, false);

    // LED dot (right side, vertically centered)
    const float dotR = 3.2f;
    const float dotX = fullF.getRight() - 10.0f;
    const float dotY = fullF.getCentreY();
    g.setColour((active ? cat : AaColor::border).withAlpha(alpha));
    g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);

    // Drop-target accent line at top of target row
    if (dropTarget)
    {
        g.setColour(AaColor::border);
        g.fillRect(fullF.getX() + 4.0f, fullF.getY(), fullF.getWidth() - 4.0f, 1.5f);
    }

    // Bottom separator (skip on last tile)
    if (displayPos < kNumModules - 1)
    {
        g.setColour(AaColor::border.withAlpha(0.5f));
        g.fillRect(fullF.getX() + 4.0f, fullF.getBottom() - 1.0f,
                   fullF.getWidth() - 4.0f, 1.0f);
    }
}

void ModuleTileList::paint(juce::Graphics& g)
{
    g.fillAll(AaColor::bg);

    auto& apvts = proc.getAPVTS();

    for (int pos = 0; pos < kNumModules; ++pos)
    {
        const int logicalIdx = moduleOrder[pos];
        const bool active = [&]() {
            const auto* raw = apvts.getRawParameterValue(moduleBypassParam(logicalIdx));
            return raw ? (*raw < 0.5f) : false;
        }();
        const bool selected   = (logicalIdx == selectedModule);
        const bool dragging   = isDragging && (pos == dragSourcePos);
        const bool dropTarget = isDragging && (pos == dragTargetPos) && (pos != dragSourcePos);

        drawTile(g, pos, active, selected, dragging, dropTarget);
    }
}

// ---- Interaction ------------------------------------------------------------

void ModuleTileList::mouseDown(const juce::MouseEvent& e)
{
    const int pos = tileAtY(e.y);
    dragSourcePos = pos;
    dragTargetPos = pos;
    isDragging    = false;

    const int logicalIdx = moduleOrder[pos];
    selectedModule = logicalIdx;
    repaint();

    if (onModuleSelected)
        onModuleSelected(logicalIdx);
}

void ModuleTileList::mouseDrag(const juce::MouseEvent& e)
{
    if (dragSourcePos < 0) return;

    const int newTarget = tileAtY(e.y);
    if (!isDragging || newTarget != dragTargetPos)
    {
        isDragging    = true;
        dragTargetPos = newTarget;
        repaint();
    }
}

void ModuleTileList::mouseUp(const juce::MouseEvent&)
{
    if (isDragging && dragSourcePos >= 0 && dragTargetPos != dragSourcePos)
    {
        const int src        = dragSourcePos;
        const int dst        = dragTargetPos;
        const int logicalIdx = moduleOrder[src];

        moduleOrder.erase(moduleOrder.begin() + src);
        moduleOrder.insert(moduleOrder.begin() + dst, logicalIdx);

        if (onOrderChanged)
            onOrderChanged(moduleOrder);
    }

    isDragging    = false;
    dragSourcePos = -1;
    dragTargetPos = -1;
    repaint();
}

void ModuleTileList::setSelectedModule(int logicalIdx)
{
    selectedModule = juce::jlimit(0, kNumModules - 1, logicalIdx);
    repaint();
}
