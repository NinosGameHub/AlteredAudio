#include "BiquadFilter.h"

static constexpr double kPi = 3.14159265358979323846;

void BiquadFilter::prepare(double sr)
{
    sampleRate = sr;
    needsUpdate = true;
    reset();
}

void BiquadFilter::reset()
{
    for (int ch = 0; ch < kMaxChannels; ++ch)
        z1[ch] = z2[ch] = 0.0;
}

float BiquadFilter::processSample(float input, int channel)
{
    if (needsUpdate) { updateCoefficients(); needsUpdate = false; }

    const double x = input;
    const double y = b0 * x + z1[channel];
    z1[channel] = b1 * x - a1 * y + z2[channel];
    z2[channel] = b2 * x - a2 * y;
    return (float) y;
}

void BiquadFilter::updateCoefficients()
{
    double c[5];
    calcCoefficients(type, sampleRate, frequency, q, gainDb, c);
    b0 = c[0]; b1 = c[1]; b2 = c[2];
    a1 = c[3]; a2 = c[4];
}

void BiquadFilter::calcCoefficients(Type type, double sampleRate,
                                    float frequency, float q, float gainDb, double out[5])
{
    // Clamp frequency below Nyquist so tan/sin stay well-behaved
    const double fc    = juce::jlimit(10.0, sampleRate * 0.49, (double)frequency);
    const double omega = 2.0 * kPi * fc / sampleRate;
    const double sinW  = std::sin(omega);
    const double cosW  = std::cos(omega);
    const double alpha = sinW / (2.0 * juce::jmax(0.05, (double)q));
    const double A     = std::pow(10.0, (double)gainDb / 40.0);

    double b0_, b1_, b2_, a0_, a1_, a2_;

    switch (type)
    {
        case Type::LowPass:
            b0_ = (1.0 - cosW) / 2.0;  b1_ = 1.0 - cosW;        b2_ = b0_;
            a0_ =  1.0 + alpha;          a1_ = -2.0 * cosW;        a2_ = 1.0 - alpha;
            break;
        case Type::HighPass:
            b0_ =  (1.0 + cosW) / 2.0; b1_ = -(1.0 + cosW);     b2_ = b0_;
            a0_ =   1.0 + alpha;         a1_ = -2.0 * cosW;        a2_ = 1.0 - alpha;
            break;
        case Type::BandPass:
            b0_ =  sinW / 2.0;           b1_ = 0.0;                b2_ = -sinW / 2.0;
            a0_ =  1.0 + alpha;          a1_ = -2.0 * cosW;        a2_ = 1.0 - alpha;
            break;
        case Type::Notch:
            b0_ =  1.0;                  b1_ = -2.0 * cosW;        b2_ = 1.0;
            a0_ =  1.0 + alpha;          a1_ = -2.0 * cosW;        a2_ = 1.0 - alpha;
            break;
        case Type::AllPass:
            b0_ =  1.0 - alpha;          b1_ = -2.0 * cosW;        b2_ = 1.0 + alpha;
            a0_ =  1.0 + alpha;          a1_ = -2.0 * cosW;        a2_ = 1.0 - alpha;
            break;
        case Type::Peak:
            b0_ =  1.0 + alpha * A;      b1_ = -2.0 * cosW;        b2_ = 1.0 - alpha * A;
            a0_ =  1.0 + alpha / A;      a1_ = -2.0 * cosW;        a2_ = 1.0 - alpha / A;
            break;
        case Type::LowShelf:
        {
            const double s = 2.0 * std::sqrt(A) * alpha;
            b0_ =  A * ((A+1.0) - (A-1.0)*cosW + s);
            b1_ =  2.0*A * ((A-1.0) - (A+1.0)*cosW);
            b2_ =  A * ((A+1.0) - (A-1.0)*cosW - s);
            a0_ =      (A+1.0) + (A-1.0)*cosW + s;
            a1_ = -2.0 * ((A-1.0) + (A+1.0)*cosW);
            a2_ =      (A+1.0) + (A-1.0)*cosW - s;
            break;
        }
        case Type::HighShelf:
        {
            const double s = 2.0 * std::sqrt(A) * alpha;
            b0_ =   A * ((A+1.0) + (A-1.0)*cosW + s);
            b1_ = -2.0*A * ((A-1.0) + (A+1.0)*cosW);
            b2_ =   A * ((A+1.0) + (A-1.0)*cosW - s);
            a0_ =       (A+1.0) - (A-1.0)*cosW + s;
            a1_ =  2.0 * ((A-1.0) - (A+1.0)*cosW);
            a2_ =       (A+1.0) - (A-1.0)*cosW - s;
            break;
        }
        case Type::LowPass1:
        {
            // Bilinear-transform first-order lowpass: 6 dB/oct
            const double K = std::tan(kPi * fc / sampleRate);
            b0_ = K;   b1_ = K;    b2_ = 0.0;
            a0_ = K + 1.0;  a1_ = K - 1.0;  a2_ = 0.0;
            break;
        }
        case Type::HighPass1:
        {
            const double K = std::tan(kPi * fc / sampleRate);
            b0_ = 1.0;  b1_ = -1.0;  b2_ = 0.0;
            a0_ = K + 1.0;  a1_ = K - 1.0;  a2_ = 0.0;
            break;
        }
        default:
            b0_ = 1.0; b1_ = b2_ = a1_ = a2_ = 0.0; a0_ = 1.0;
    }

    out[0] = b0_/a0_;  out[1] = b1_/a0_;  out[2] = b2_/a0_;
    out[3] = a1_/a0_;  out[4] = a2_/a0_;
}

double BiquadFilter::magnitudeDb(const double c[5], double freqHz, double sampleRate)
{
    // |H(e^jw)|^2 evaluated directly from the difference-equation coefficients
    const double w   = 2.0 * kPi * freqHz / sampleRate;
    const double cw  = std::cos(w);
    const double c2w = std::cos(2.0 * w);
    const double b0 = c[0], b1 = c[1], b2 = c[2], a1 = c[3], a2 = c[4];

    const double num = b0*b0 + b1*b1 + b2*b2 + 2.0*(b0*b1 + b1*b2)*cw + 2.0*b0*b2*c2w;
    const double den = 1.0   + a1*a1 + a2*a2 + 2.0*(a1   + a1*a2)*cw + 2.0*a2*c2w;

    return 10.0 * std::log10(juce::jmax(1e-12, num) / juce::jmax(1e-12, den));
}
