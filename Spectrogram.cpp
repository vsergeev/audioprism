#include <vector>
#include <cstdint>
#include <cstring>

#include "Spectrogram.hpp"

Spectrogram::Spectrogram() : magnitudeMin(0.0), magnitudeMax(60.0) { }

void Spectrogram::setMagnitudeMax(double max) {
    magnitudeMax = max;
}

double Spectrogram::getMagnitudeMax() {
    return magnitudeMax;
}

void Spectrogram::setMagnitudeMin(double min) {
    magnitudeMin = min;
}

double Spectrogram::getMagnitudeMin() {
    return magnitudeMin;
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

void Spectrogram::update(uint32_t *pixels, unsigned int width, unsigned int height, const std::vector<double> &dft_magnitudes) {
    unsigned int i;

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

