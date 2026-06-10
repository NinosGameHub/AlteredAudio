#include "ModulationMatrix.h"

void ModulationMatrix::update(float lfo1Val, float lfo2Val, float envVal) noexcept
{
    src1   = lfo1Val;
    src2   = lfo2Val;
    srcEnv = envVal;
}

float ModulationMatrix::getModulation(const juce::String& paramId) const noexcept
{
    float total = 0.0f;
    for (int i = 0; i < numConnections; ++i)
    {
        const auto& c = connections[i];
        if (!c.enabled || c.targetId != paramId) continue;

        float srcVal = 0.0f;
        switch (c.source)
        {
            case Source::LFO1:     srcVal = src1;   break;
            case Source::LFO2:     srcVal = src2;   break;
            case Source::Envelope: srcVal = srcEnv; break;
        }
        total += srcVal * c.depth;
    }
    return juce::jlimit(-1.0f, 1.0f, total);
}

int ModulationMatrix::addConnection(Source src, const juce::String& paramId, float depth)
{
    if (numConnections >= kMaxConnections) return -1;
    auto& c  = connections[numConnections];
    c.source  = src;
    c.targetId = paramId;
    c.depth   = depth;
    c.enabled = true;
    return numConnections++;
}

void ModulationMatrix::removeConnection(int index)
{
    if (index < 0 || index >= numConnections) return;
    for (int i = index; i < numConnections - 1; ++i)
        connections[i] = connections[i + 1];
    --numConnections;
}

void ModulationMatrix::setDepth(int index, float depth)
{
    if (index >= 0 && index < numConnections)
        connections[index].depth = depth;
}
