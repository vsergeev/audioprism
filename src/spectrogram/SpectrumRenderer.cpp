#include <vector>
#include <complex>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <functional>

#include "SpectrumRenderer.hpp"

namespace Spectrogram {

SpectrumRenderer::SpectrumRenderer(double magnitudeMin, double magnitudeMax, bool magnitudeLog, ColorScheme colors) : settings({magnitudeMin, magnitudeMax, magnitudeLog, colors}) {}

template <typename T>
static constexpr T normalize(T value, T min, T max) {
    /* Clamp value to min/max, then linearly normalize to 0.0 to 1.0 */
    return (std::max(std::min(value, max), min) - min) / (max - min);
}

static uint32_t valueToPixel_Heat(double value) {
    uint8_t r, g, b;

    if (value < (1.0 / 5.0)) {
        /* Black (0,0,0) - Blue (0,0,1) */
        r = 0;
        g = 0;
        b = static_cast<uint8_t>(255.0 * normalize(value, 0.0, 1.0 / 5.0));
    } else if (value < (2.0 / 5.0)) {
        /* Blue (0,0,1) - Green (0,1,0) */
        uint8_t c = static_cast<uint8_t>(255.0 * normalize(value, 1.0 / 5.0, 2.0 / 5.0));
        r = 0;
        g = c;
        b = static_cast<uint8_t>(255 - c);
    } else if (value < (3.0 / 5.0)) {
        /* Green (0,1,0) - Yellow (1,1,0) */
        r = static_cast<uint8_t>(255.0 * normalize(value, 2.0 / 5.0, 3.0 / 5.0));
        g = 255;
        b = 0;
    } else if (value < (4.0 / 5.0)) {
        /* Yellow (1,1,0) - Red (1,0,0) */
        r = 255;
        g = static_cast<uint8_t>(255 - static_cast<uint8_t>(255.0 * normalize(value, 3.0 / 5.0, 4.0 / 5.0)));
        b = 0;
    } else {
        /* Red (1,0,0) - White (1,1,1) */
        uint8_t c = static_cast<uint8_t>(255.0 * normalize(value, 4.0 / 5.0, 5.0 / 5.0));
        r = 255;
        g = c;
        b = c;
    }

    return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(b));
}

static uint32_t valueToPixel_Blue(double value) {
    uint8_t r, g, b;

    if (value < 0.5) {
        /* Black (0,0,0) - Blue (0,0,1) */
        r = 0;
        g = 0;
        b = static_cast<uint8_t>(255.0 * normalize(value, 0.0, 0.5));
    } else {
        /* Blue (0,0,1) - White (1,1,1)  */
        uint8_t c = static_cast<uint8_t>(255.0 * normalize(value, 0.5, 1.0));
        r = c;
        g = c;
        b = 255;
    }

    return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(b));
}

static uint32_t valueToPixel_Grayscale(double value) {
    uint8_t c = static_cast<uint8_t>(255.0 * value);
    return (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(c) << 8) | (static_cast<uint32_t>(c));
}

void SpectrumRenderer::render(std::vector<uint32_t> &pixels, const std::vector<std::complex<double>> &dft) {
    unsigned int i;
    uint32_t (*valueToPixel)(double) = nullptr;
    double (*processMagnitude)(double) = nullptr;

    if (settings.colors == SpectrumRenderer::ColorScheme::Heat)
        valueToPixel = valueToPixel_Heat;
    else if (settings.colors == SpectrumRenderer::ColorScheme::Blue)
        valueToPixel = valueToPixel_Blue;
    else if (settings.colors == SpectrumRenderer::ColorScheme::Grayscale)
        valueToPixel = valueToPixel_Grayscale;

    if (settings.magnitudeLog)
        processMagnitude = [](double x) -> double { return 20*std::log10(x); };
    else
        processMagnitude = [](double x) -> double { return x; };

    /* Generate pixel row for this DFT */
    float index_scale = static_cast<float>(dft.size()) / static_cast<float>(pixels.size());
    for (i = 0; i < pixels.size(); i++) {
        double magnitude = processMagnitude(std::abs(dft[static_cast<unsigned int>(index_scale * static_cast<float>(i))]));
        pixels[i] = valueToPixel(normalize(magnitude, settings.magnitudeMin, settings.magnitudeMax));
    }
}

std::string to_string(const SpectrumRenderer::ColorScheme &colors) {
    if (colors == SpectrumRenderer::ColorScheme::Heat)
        return "Heat";
    else if (colors == SpectrumRenderer::ColorScheme::Blue)
        return "Blue";
    else if (colors == SpectrumRenderer::ColorScheme::Grayscale)
        return "Grayscale";

    return "Unknown";
}

}
