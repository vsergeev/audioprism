#include <vector>
#include <complex>
#include <cstdint>
#include <cstring>

#include "Spectrogram.hpp"

#define DEFAULT_MAGNITUDE_MIN   0.0
#define DEFAULT_MAGNITUDE_MAX   60.0

Spectrogram::Spectrogram() : settings({0.0, 60.0}) {}

uint32_t Spectrogram::magnitude2pixel(double magnitude) {
    /* Clamp magnitude at min and max */
    if (magnitude < settings.magnitudeMin)
        magnitude = settings.magnitudeMin;
    if (magnitude > settings.magnitudeMax)
        magnitude = settings.magnitudeMax;

    /* Map magnitude to pixel */
    magnitude = (UINT16_MAX/(settings.magnitudeMax-settings.magnitudeMin))*(magnitude-settings.magnitudeMin);

    return static_cast<uint16_t>(magnitude);
}

void Spectrogram::render(std::vector<uint32_t> &pixels, const std::vector<std::complex<double>> &dft) {
    unsigned int i;

    /* Generate pixel row for this DFT */
    if (dft.size() < pixels.size()) {
        for (i = 0; i < dft.size()-1; i++)
            pixels[i] = magnitude2pixel(20*std::log10(std::abs(dft[i+1])));
        for (i = dft.size()-1; i < pixels.size(); i++)
            pixels[i] = 0x00;
    } else {
        float index_scale = float(dft.size()-1) / float(pixels.size());
        for (i = 0; i < pixels.size(); i++)
            pixels[i] = magnitude2pixel(20*std::log10(std::abs(dft[1+int(index_scale*i)])));
    }
}

float Spectrogram::getHzPerPixel(unsigned int width, unsigned int dftSize, unsigned int sampleRate) {
    return ((static_cast<float>(sampleRate)/2.0)/static_cast<float>(dftSize)) * (static_cast<float>(dftSize)/static_cast<float>(width));
}

