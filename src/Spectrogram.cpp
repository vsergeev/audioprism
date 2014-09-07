#include <vector>
#include <complex>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <functional>

#include "Spectrogram.hpp"

Spectrogram::Spectrogram(double magnitudeMin, double magnitudeMax, ColorScheme colors) : settings({magnitudeMin, magnitudeMax, colors}) {}

template <typename T>
static constexpr T normalize(T value, T min, T max) {
    /* Clamp value to min/max, then linearly normalize to 0.0 to 1.0 */
    return (std::max(std::min(value, max), min) - min)/(max - min);
}

static uint32_t valueToPixel_Heat(double value) {
    uint8_t r, g, b;

    if (value < (1.0/5.0)) {
        /* Black (0,0,0) - Blue (0,0,1) */
        r = 0;
        g = 0;
        b = static_cast<uint8_t>(255.0*normalize(value, 0.0, 1.0/5.0));
    } else if (value < (2.0/5.0)) {
        /* Blue (0,0,1) - Green (0,1,0) */
        uint8_t c = static_cast<uint8_t>(255.0*normalize(value, 1.0/5.0, 2.0/5.0));
        r = 0;
        g = c;
        b = 255-c;
    } else if (value < (3.0/5.0)) {
        /* Green (0,1,0) - Yellow (1,1,0) */
        r = static_cast<uint8_t>(255.0*normalize(value, 2.0/5.0, 3.0/5.0));
        g = 255;
        b = 0;
    } else if (value < (4.0/5.0)) {
        /* Yellow (1,1,0) - Red (1,0,0) */
        r = 255;
        g = 255 - static_cast<uint8_t>(255.0*normalize(value, 3.0/5.0, 4.0/5.0));
        b = 0;
    } else {
        /* Red (1,0,0) - White (1,1,1) */
        uint8_t c = static_cast<uint8_t>(255.0*normalize(value, 4.0/5.0, 5.0/5.0));
        r = 255;
        g = c;
        b = c;
    }

    return (r << 16) | (g << 8) | (b);
}

static uint32_t valueToPixel_Blue(double value) {
    uint8_t r, g, b;

    if (value < 0.5) {
        /* Black (0,0,0) - Blue (0,0,1) */
        r = 0;
        g = 0;
        b = static_cast<uint8_t>(255.0*normalize(value, 0.0, 0.5));
    } else {
        /* Blue (0,0,1) - White (1,1,1)  */
        uint8_t c = static_cast<uint8_t>(255.0*normalize(value, 0.5, 1.0));
        r = c;
        g = c;
        b = 255;
    }

    return (r << 16) | (g << 8) | (b);
}

static uint32_t valueToPixel_Grayscale(double value) {
    uint8_t c = static_cast<uint8_t>(255.0*value);
    return (c << 16) | (c << 8) | (c);
}

void Spectrogram::render(std::vector<uint32_t> &pixels, const std::vector<std::complex<double>> &dft) {
    unsigned int i;
    uint32_t (*valueToPixel)(double) = nullptr;

    if (settings.colors == Spectrogram::ColorScheme::Heat)
        valueToPixel = valueToPixel_Heat;
    else if (settings.colors == Spectrogram::ColorScheme::Blue)
        valueToPixel = valueToPixel_Blue;
    else if (settings.colors == Spectrogram::ColorScheme::Grayscale)
        valueToPixel = valueToPixel_Grayscale;

    /* Generate pixel row for this DFT */
    float index_scale = static_cast<float>(dft.size()) / static_cast<float>(pixels.size());
    for (i = 0; i < pixels.size(); i++) {
        double logMagnitude = 20*std::log10(std::abs(dft[static_cast<int>(index_scale*i)]));
        pixels[i] = valueToPixel(normalize(logMagnitude, settings.magnitudeMin, settings.magnitudeMax));
    }
}

std::function<float (int)> Spectrogram::getPixelToHz(unsigned int width, unsigned int dftSize, unsigned int sampleRate) {
    float binPerPixel, hzPerBin;

    binPerPixel = static_cast<float>((dftSize/2 + 1))/static_cast<float>(width);
    hzPerBin = ((static_cast<float>(sampleRate))/2.0)/static_cast<float>((dftSize/2 + 1));

    return [=] (int x) -> float { return std::floor(static_cast<float>(x)*binPerPixel)*hzPerBin; };
}

