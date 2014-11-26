#ifndef _CONFIGURATION_HPP
#define _CONFIGURATION_HPP

#include "dft/RealDft.hpp"
#include "spectrogram/SpectrumRenderer.hpp"
#include "image/Orientation.hpp"

using namespace DFT;
using namespace Image;
using namespace Spectrogram;

namespace Configuration {

struct Settings {
    /* Interface Settings */
    unsigned int width = 640;
    unsigned int height = 480;
    Orientation orientation = Orientation::Vertical;
    /* Audio Settings */
    unsigned int audioSampleRate = 24000;
    unsigned int audioReadSize = 1024;
    /* DFT Settings */
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
    /* DFT Size min, max */
    unsigned int dftSizeMin = 32;
    unsigned int dftSizeMax = 8192;
    /* Audio Read Size step */
    unsigned int audioReadSizeStep = 32;
};

extern Settings InitialSettings;
extern Limits UserLimits;

}

#endif

