#pragma once

#include <vector>
#include <complex>
#include <cstdint>
#include <functional>

namespace Spectrogram {

class SpectrumRenderer {
  public:
    enum class ColorScheme { Heat,
                             Blue,
                             Grayscale };

    SpectrumRenderer(float magnitudeMin, float magnitudeMax, bool magnitudeLog, ColorScheme colors);

    /* Render a new pixel row from a DFT vector */
    void render(std::vector<uint32_t> &pixels, const std::vector<std::complex<float>> &dft);

    struct {
        float magnitudeMin;
        float magnitudeMax;
        bool magnitudeLog;
        ColorScheme colors;
    } settings;
};

std::string to_string(const SpectrumRenderer::ColorScheme &colors);

}
