#pragma once
#include <JuceHeader.h>
#include <array>

class ModulationMatrix
{
public:
    enum class Source { LFO1, LFO2, Envelope };

    struct Connection
    {
        Source       source  = Source::LFO1;
        juce::String targetId;
        float        depth   = 0.0f; // -1..+1
        bool         enabled = true;
    };

    static constexpr int kMaxConnections = 16;

    // Call once per block with current source values (LFO outputs in [-1,+1], env in [0,1])
    void  update(float lfo1Val, float lfo2Val, float envVal) noexcept;

    // Returns total modulation offset in [-1, +1] for the given parameter ID
    float getModulation(const juce::String& paramId) const noexcept;

    // Returns total modulation offset for a pre-hashed param; use with fixed const char* IDs
    float getModulation(const char* paramId) const noexcept
    { return getModulation(juce::String(paramId)); }

    int  addConnection   (Source src, const juce::String& paramId, float depth);
    void removeConnection(int index);
    void setDepth        (int index, float depth);

    int               getNumConnections()    const { return numConnections; }
    const Connection& getConnection(int idx) const { return connections[idx]; }

private:
    std::array<Connection, kMaxConnections> connections;
    int   numConnections = 0;
    float src1   = 0.0f;
    float src2   = 0.0f;
    float srcEnv = 0.0f;
};
