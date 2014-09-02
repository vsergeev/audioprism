#ifndef _SPECTROGRAM_HPP
#define _SPECTROGRAM_HPP

#include <vector>
#include <cstdint>

class Spectrogram {
  public:
    Spectrogram();
    void update(uint32_t *pixels, unsigned int width, unsigned int height, const std::vector<double> &dft_magnitudes);
};

#endif

