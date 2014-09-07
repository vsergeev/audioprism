#ifndef _SPECTROGRAM_HPP
#define _SPECTROGRAM_HPP

#include <vector>
#include <complex>
#include <cstdint>
#include <functional>

class Spectrogram {
  public:
    enum class ColorScheme { Heat, Blue, Grayscale };

    Spectrogram(double magnitudeMin, double magnitudeMax, ColorScheme colors);
    void render(std::vector<uint32_t> &pixels, const std::vector<std::complex<double>> &dft);
    std::function<float (int)> getPixelToHz(unsigned int width, unsigned int dftSize, unsigned int sampleRate);

    struct {
        double magnitudeMin;
        double magnitudeMax;
        ColorScheme colors;
    } settings;
};

#endif

