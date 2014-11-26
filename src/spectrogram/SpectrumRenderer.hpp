#ifndef _SPECTRUMRENDERER_HPP
#define _SPECTRUMRENDERER_HPP

#include <vector>
#include <complex>
#include <cstdint>
#include <functional>

namespace Spectrogram {

class SpectrumRenderer {
  public:
    enum class ColorScheme { Heat, Blue, Grayscale };

    SpectrumRenderer(double magnitudeMin, double magnitudeMax, bool magnitudeLog, ColorScheme colors);

    void render(std::vector<uint32_t> &pixels, const std::vector<std::complex<double>> &dft);

    std::function<float (int)> getPixelToHz(unsigned int width, unsigned int dftSize, unsigned int sampleRate);

    struct {
        double magnitudeMin;
        double magnitudeMax;
        bool magnitudeLog;
        ColorScheme colors;
    } settings;
};

}

#endif

