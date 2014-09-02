#ifndef _SPECTROGRAM_HPP
#define _SPECTROGRAM_HPP

#include <vector>
#include <memory>
#include <cstdint>

class Spectrogram {
  public:
    Spectrogram(unsigned int width, unsigned int height);
    void update(const std::vector<double> &dft_magnitudes);
    const uint32_t *getPixels();

    const unsigned int width;
    const unsigned int height;
    double magnitudeMin;
    double magnitudeMax;

  private:
    std::unique_ptr<uint32_t []> _pixels;
    uint32_t magnitude2pixel(double magnitude);
};

#endif

