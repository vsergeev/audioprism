#pragma once

#include "audio/AudioSource.hpp"
#include "dft/RealDft.hpp"
#include "spectrogram/SpectrumRenderer.hpp"

using namespace DFT;
using namespace Spectrogram;

namespace Configuration {

enum class Orientation { Horizontal,
                         Vertical };

struct Settings {
    /* Interface Settings */
    unsigned int width = 640;
    unsigned int height = 480;
    Orientation orientation = Orientation::Vertical;
    /* Audio Settings */
    unsigned int audioSampleRate = 24000;
    /* DFT Settings */
    float samplesOverlap = 0.50;
    unsigned int dftSize = 1024;
    RealDft::WindowFunction dftWf = RealDft::WindowFunction::Hann;
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
    unsigned int dftSizeMin = 64;
    unsigned int dftSizeMax = 8192;
    /* Samples overlap min, max, step */
    float samplesOverlapMin = 0.05f;
    float samplesOverlapMax = 0.95f;
    float samplesOverlapStep = 0.01f;
};

extern Settings InitialSettings;
extern Limits UserLimits;

}
