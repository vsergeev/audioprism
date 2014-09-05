#include <vector>
#include <complex>
#include <cstdint>
#include <cstring>
#include <functional>

#include "Spectrogram.hpp"

Spectrogram::Spectrogram(double magnitudeMin, double magnitudeMax) : settings({magnitudeMin, magnitudeMax}) {}

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
    float index_scale = static_cast<float>(dft.size()) / static_cast<float>(pixels.size());
    for (i = 0; i < pixels.size(); i++)
        pixels[i] = magnitude2pixel(20*std::log10(std::abs(dft[int(index_scale*i)])));
}

std::function<float (int)> Spectrogram::getPixelToHz(unsigned int width, unsigned int dftSize, unsigned int sampleRate) {
    float binPerPixel, hzPerBin;

    binPerPixel = static_cast<float>((dftSize/2 + 1))/static_cast<float>(width);
    hzPerBin = ((static_cast<float>(sampleRate))/2.0)/static_cast<float>((dftSize/2 + 1));

    return [=] (int x) -> float { return std::floor(static_cast<float>(x)*binPerPixel)*hzPerBin; };
}

