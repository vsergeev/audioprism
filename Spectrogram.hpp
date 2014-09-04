#ifndef _SPECTROGRAM_HPP
#define _SPECTROGRAM_HPP

#include <vector>
#include <complex>
#include <cstdint>

class Spectrogram {
  public:
    Spectrogram();
    void render(std::vector<uint32_t> &pixels, const std::vector<std::complex<double>> &dft);

    struct {
        double magnitudeMin;
        double magnitudeMax;
    } settings;

  private:
    uint32_t magnitude2pixel(double magnitude);
};

#endif

