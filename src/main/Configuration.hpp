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
    bool fullscreen = false;
    unsigned int width = 640;
    unsigned int height = 480;
    Orientation orientation = Orientation::Vertical;
    /* Audio Settings */
    unsigned int audioSampleRate = 24000;
    /* DFT Settings */
    float samplesOverlap = 0.50;
    unsigned int dftSize = 1024;
    RealDft::WindowFunction dftWindowFunction = RealDft::WindowFunction::Hann;
    /* Spectrogram Settings */
    float magnitudeMin = 0.0;
    float magnitudeMax = 45.0;
    bool magnitudeLog = true;
    SpectrumRenderer::ColorScheme colorScheme = SpectrumRenderer::ColorScheme::Heat;
    /* Initial settings when switching between logarithmic/linear in UI */
    float magnitudeLogMin = 0.0;
    float magnitudeLogMax = 50.0;
    float magnitudeLinearMin = 0.0;
    float magnitudeLinearMax = 50.0;
};

struct Limits {
    /* Linear magnitude min, max, step */
    float magnitudeLinearMin = 0.0;
    float magnitudeLinearMax = 1000.0;
    float magnitudeLinearStep = 25.0;
    /* Logarithmic magnitude min, max, step */
    float magnitudeLogMin = -80.0;
    float magnitudeLogMax = 80.0;
    float magnitudeLogStep = 5.0;
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
