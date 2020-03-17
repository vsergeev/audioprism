#pragma once

#include <vector>
#include <complex>
#include <cstdint>

namespace Spectrogram {

class SpectrumRenderer {
  public:
    enum class ColorScheme { Heat,
                             Blue,
                             Grayscale };

    SpectrumRenderer(float magnitudeMin, float magnitudeMax, bool magnitudeLog, ColorScheme colorScheme);

    /* Render a new pixel row from a DFT vector */
    void render(std::vector<uint32_t> &pixels, const std::vector<std::complex<float>> &dft);

    /* Get/Set Min Magnitude */
    float getMagnitudeMin();
    void setMagnitudeMin(float min);

    /* Get/Set Max Magnitude */
    float getMagnitudeMax();
    void setMagnitudeMax(float max);

    /* Get/Set Magnitude Linear/Log */
    bool getMagnitudeLog();
    void setMagnitudeLog(bool logarithmic);

    /* Get/Set Color Scheme */
    ColorScheme getColorScheme();
    void setColorScheme(ColorScheme colorScheme);

  private:
    float _magnitudeMin;
    float _magnitudeMax;
    bool _magnitudeLog;
    ColorScheme _colorScheme;
};

std::string to_string(const SpectrumRenderer::ColorScheme &colorScheme);

}
