#include "CRTDisplay.h"
#include "AaLookAndFeel.h"
#include <cmath>

namespace
{
    // 10 labels along the log frequency axis (20 Hz – 20 kHz)
    struct FreqLabel { const char* text; float hz; };
    constexpr FreqLabel kLabels[] = {
        {"20",20},{"50",50},{"100",100},{"200",200},{"500",500},
        {"1k",1000},{"2k",2000},{"5k",5000},{"10k",10000},{"20k",20000}
    };
    constexpr int kNumLabels = 10;

    // 1/3-octave center frequencies (Hz) for the 32 bars
    constexpr float kBarFreqs[32] = {
        20,25,31.5f,40,50,63,80,100,125,160,
        200,250,315,400,500,630,800,1000,1250,1600,
        2000,2500,3150,4000,5000,6300,8000,10000,12500,16000,
        20000,22400
    };
}

float CRTDisplay::baseShape(int bin)
{
    // Plausible spectral shape: low-shelf bump + presence peak + rolloff
    const float t        = (float)bin / (float)(kNumBars - 1);
    const float bass     = 0.68f * std::exp(-t * 4.2f);
    const float presence = 0.38f * std::exp(-std::pow((t - 0.64f) * 4.8f, 2.0f));
    const float noise    = 0.18f * (1.0f - t);
    return juce::jlimit(0.06f, 0.94f, 0.22f + bass + presence + noise);
}

CRTDisplay::CRTDisplay()
{
    initBars();
    startTimerHz(kFps);
}

CRTDisplay::~CRTDisplay()
{
    stopTimer();
}

void CRTDisplay::initBars()
{
    for (int i = 0; i < kNumBars; ++i)
    {
        barHeights[i] = baseShape(i);
        barTargets[i] = barHeights[i];
        driftPhase[i] = rng.nextFloat() * juce::MathConstants<float>::twoPi;
    }
}

void CRTDisplay::timerCallback()
{
    const float t = (float)(juce::Time::getMillisecondCounter() % 1000000) * 0.001f;

    for (int i = 0; i < kNumBars; ++i)
    {
        const float drift = 0.055f * std::sin(t * 0.71f + driftPhase[i])
                          + 0.028f * std::sin(t * 1.37f + driftPhase[i] * 1.31f)
                          + 0.015f * std::sin(t * 2.60f + driftPhase[i] * 0.78f);
        barTargets[i] = juce::jlimit(0.05f, 0.92f, baseShape(i) + drift);
        barHeights[i] += (barTargets[i] - barHeights[i]) * 0.18f;
    }
    repaint();
}

void CRTDisplay::resized() {}

void CRTDisplay::paint(juce::Graphics& g)
{
    const float w      = (float)getWidth();
    const float h      = (float)getHeight();
    const float labelH = 17.0f;
    const float barAreaH = h - labelH;
    const float barW   = w / (float)kNumBars;

    // Background
    g.fillAll(AaColor::crtBg);

    // Horizontal grid lines (dim amber)
    const int kGridH = 4;
    for (int i = 1; i < kGridH; ++i)
    {
        const float gy = barAreaH * (float)i / (float)kGridH;
        g.setColour(AaColor::crtAmber.withAlpha(0.10f));
        g.drawHorizontalLine((int)gy, 0.0f, w);
    }

    // Vertical grid lines (very dim)
    const int kGridV = 8;
    for (int i = 1; i < kGridV; ++i)
    {
        const float gx = w * (float)i / (float)kGridV;
        g.setColour(AaColor::crtAmber.withAlpha(0.07f));
        g.drawVerticalLine((int)gx, 0.0f, barAreaH);
    }

    // Spectrum bars — gradient top-to-bottom: bright amber → dim amber
    for (int i = 0; i < kNumBars; ++i)
    {
        const float bh   = barHeights[i] * barAreaH;
        const float bx   = (float)i * barW;
        const float by   = barAreaH - bh;
        const float topA = 0.82f + barHeights[i] * 0.15f;
        const float botA = topA * 0.42f;

        juce::ColourGradient grad(
            AaColor::crtAmber.withAlpha(topA), bx, by,
            AaColor::crtAmber.withAlpha(botA), bx, barAreaH,
            false
        );
        g.setGradientFill(grad);
        g.fillRect(bx + 0.8f, by, barW - 1.6f, bh);
    }

    // Horizontal scan lines every 2px (7% dark overlay — drawn last over bars)
    g.setColour(juce::Colours::black.withAlpha(0.07f));
    for (float sy = 0.0f; sy < barAreaH; sy += 2.0f)
        g.fillRect(0.0f, sy, w, 1.0f);

    // Frequency labels (monospace, dim amber)
    g.setColour(AaColor::crtAmber.withAlpha(0.55f));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));

    const float logMin = std::log10(kBarFreqs[0]);
    const float logMax = std::log10(kBarFreqs[kNumBars - 1]);

    for (int li = 0; li < kNumLabels; ++li)
    {
        const float lf  = std::log10(kLabels[li].hz);
        const float t   = (lf - logMin) / (logMax - logMin);
        const float lx  = t * w;
        g.drawText(kLabels[li].text,
                   (int)(lx - 14.0f), (int)barAreaH + 2, 28, (int)labelH - 2,
                   juce::Justification::centred, false);
    }
}
