#ifndef _SPECTROGRAM_HPP
#define _SPECTROGRAM_HPP

#include <vector>
#include <cstdint>

class Spectrogram {
  public:
    Spectrogram();
    void update(uint32_t *pixels, unsigned int width, unsigned int height, const std::vector<double> &dft_magnitudes);
    void setMagnitudeMax(double max);
    double getMagnitudeMax();
    void setMagnitudeMin(double min);
    double getMagnitudeMin();

  private:
    double magnitudeMin;
    double magnitudeMax;
    uint32_t magnitude2pixel(double magnitude);
};

#endif

