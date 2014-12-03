#ifndef _CONFIGURATION_HPP
#define _CONFIGURATION_HPP

#include "audio/AudioSource.hpp"
#include "dft/RealDft.hpp"
#include "spectrogram/SpectrumRenderer.hpp"

using namespace DFT;
using namespace Spectrogram;

namespace Configuration {

enum class Orientation { Horizontal, Vertical };

struct Settings {
    /* Interface Settings */
    unsigned int width = 640;
    unsigned int height = 480;
    Orientation orientation = Orientation::Vertical;
    /* Audio Settings */
    unsigned int audioSampleRate = 24000;
    /* DFT Settings */
    float dftOverlap = 0.50;
    unsigned int dftSize = 2048;
    RealDft::WindowFunction dftWf = RealDft::WindowFunction::Hanning;
    /* Spectrogram Settings */
    double magnitudeMin = 0.0;
    double magnitudeMax = 45.0;
    bool magnitudeLog = true;
    SpectrumRenderer::ColorScheme colors = SpectrumRenderer::ColorScheme::Heat;
    /* Initial settings when switching between logarithmic/linear in UI */
    double magnitudeLogMin = 0.0;
    double magnitudeLogMax = 50.0;
    double magnitudeLinearMin = 0.0;
    double magnitudeLinearMax = 50.0;
};

struct Limits {
    /* Linear magnitude min, max, step */
    double magnitudeLinearMin = 0.0;
    double magnitudeLinearMax = 1000.0;
    double magnitudeLinearStep = 25.0;
    /* Logarithmic magnitude min, max, step */
    double magnitudeLogMin = -80.0;
    double magnitudeLogMax = 80.0;
    double magnitudeLogStep = 5.0;
    /* DFT size min, max */
    unsigned int dftSizeMin = 32;
    unsigned int dftSizeMax = 8192;
    /* DFT overlap min, max, step */
    float dftOverlapMin = 0.05;
    float dftOverlapMax = 0.95;
    float dftOverlapStep = 0.025;
};

extern Settings InitialSettings;
extern Limits UserLimits;

}

#endif

