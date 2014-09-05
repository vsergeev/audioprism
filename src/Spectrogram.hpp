#ifndef _SPECTROGRAM_HPP
#define _SPECTROGRAM_HPP

#include <vector>
#include <complex>
#include <cstdint>
#include <functional>

class Spectrogram {
  public:
    Spectrogram(double magnitudeMin, double magnitudeMax);
    void render(std::vector<uint32_t> &pixels, const std::vector<std::complex<double>> &dft);
    std::function<float (int)> getPixelToHz(unsigned int width, unsigned int dftSize, unsigned int sampleRate);

    struct {
        double magnitudeMin;
        double magnitudeMax;
    } settings;

  private:
    uint32_t magnitude2pixel(double magnitude);
};

#endif

