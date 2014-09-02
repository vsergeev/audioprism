#include <vector>
#include <cstdint>
#include <cstring>

#include "Spectrogram.hpp"

Spectrogram::Spectrogram(unsigned int width, unsigned int height) :
  width(width), height(height), magnitudeMin(0.0), magnitudeMax(60.0) {
    _pixels = std::unique_ptr<uint32_t []>(new uint32_t[width*height]);
}

const uint32_t *Spectrogram::getPixels() {
    return _pixels.get();
}

uint32_t Spectrogram::magnitude2pixel(double magnitude) {
    /* Clamp magnitude at min and max */
    if (magnitude < magnitudeMin)
        magnitude = magnitudeMin;
    if (magnitude > magnitudeMax)
        magnitude = magnitudeMax;

    /* Map magnitude to pixel */
    magnitude = (UINT16_MAX/(magnitudeMax-magnitudeMin))*(magnitude-magnitudeMin);

    return static_cast<uint16_t>(magnitude);
}

void Spectrogram::update(const std::vector<double> &dft_magnitudes) {
    unsigned int i;
    uint32_t *pixels = _pixels.get();

    /* Move pixels up one */
    for (i = 1; i < height; i++)
        memcpy(pixels + width*(i-1), pixels + width*i, width*sizeof(uint32_t));

    /* Generate pixel row for this DFT */
    if (dft_magnitudes.size() < width) {
        for (i = 0; i < dft_magnitudes.size()-1; i++)
            pixels[width*(height-1) + i] = magnitude2pixel(dft_magnitudes[i+1]);
        for (i = dft_magnitudes.size()-1; i < width; i++)
            pixels[width*(height-1) + i] = 0x00;
    } else {
        float index_scale = float(dft_magnitudes.size()-1) / float(width);
        for (i = 0; i < width; i++)
            pixels[width*(height-1) + i] = magnitude2pixel(dft_magnitudes[1+int(index_scale*i)]);
    }
}

