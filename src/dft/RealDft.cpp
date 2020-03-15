#include <cmath>

#include "RealDft.hpp"

namespace DFT {

std::string to_string(const RealDft::WindowFunction &wf) {
    if (wf == RealDft::WindowFunction::Hann)
        return "Hann";
    else if (wf == RealDft::WindowFunction::Hamming)
        return "Hamming";
    else if (wf == RealDft::WindowFunction::Bartlett)
        return "Bartlett";
    else if (wf == RealDft::WindowFunction::Rectangular)
        return "Rectangular";

    return "";
}

std::ostream &operator<<(std::ostream &os, const RealDft::WindowFunction &wf) {
    os << to_string(wf);
    return os;
}

static void calculateWindow(std::vector<float> &window, RealDft::WindowFunction windowFunction) {
    size_t N = window.size();
    if (windowFunction == RealDft::WindowFunction::Hann) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 0.5f * (1.0f - std::cos((2.0f * static_cast<float>(M_PI) * static_cast<float>(n)) / static_cast<float>(N - 1)));
    } else if (windowFunction == RealDft::WindowFunction::Hamming) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 0.54f - 0.46f * std::cos((2.0f * static_cast<float>(M_PI) * static_cast<float>(n)) / static_cast<float>(N - 1));
    } else if (windowFunction == RealDft::WindowFunction::Bartlett) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 1.0f - std::abs((static_cast<float>(n) - static_cast<float>(N - 1) / 2.0f) / (static_cast<float>(N - 1) / 2.0f));
    } else if (windowFunction == RealDft::WindowFunction::Rectangular) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 1.0f;
    }
}

RealDft::RealDft(unsigned int N, RealDft::WindowFunction wf) : _N(N), _windowFunction(wf) {
    setSize(_N);
}

RealDft::~RealDft() {
    if (_plan)
        fftwf_destroy_plan(_plan);
    if (_dft)
        fftwf_free(_dft);
    if (_windowedSamples)
        fftwf_free(_windowedSamples);

    fftwf_cleanup();
}

void RealDft::compute(std::vector<std::complex<float>> &dft, const std::vector<float> &samples) {
    /* Assert sample buffer size */
    if (samples.size() != _N)
        throw SizeMismatchException("Samples size does not match DFT size!");

    /* Size dft buffer correctly */
    dft.resize(_N / 2 + 1);

    /* Window samples first */
    for (unsigned int n = 0; n < _N; n++)
        _windowedSamples[n] = samples[n] * _window[n];

    /* Execute DFT */
    fftwf_execute(_plan);

    /* Compute DFT magnitude */
    for (unsigned int n = 0; n < _N / 2 + 1; n++)
        dft[n] = std::complex<float>(_dft[n][0], _dft[n][1]);
}

unsigned int RealDft::getSize() {
    return _N;
}

void RealDft::setSize(unsigned int N) {
    /* Deallocate FFTW resources we are changing */
    if (_plan)
        fftwf_destroy_plan(_plan);
    if (_dft)
        fftwf_free(_dft);
    if (_windowedSamples)
        fftwf_free(_windowedSamples);

    /* Resize window */
    _window.resize(N);
    /* Recalculate window function */
    calculateWindow(_window, _windowFunction);

    /* Allocate windowed samples buffer */
    _windowedSamples = fftwf_alloc_real(N);
    if (_windowedSamples == nullptr)
        throw AllocationException("Allocating sample memory.");

    /* Allocate DFT buffer */
    _dft = fftwf_alloc_complex(N / 2 + 1);
    if (_dft == nullptr)
        throw AllocationException("Allocating DFT memory.");

    /* Rebuild our plan */
    _plan = fftwf_plan_dft_r2c_1d(static_cast<int>(N), _windowedSamples, _dft, FFTW_MEASURE);
    if (_plan == nullptr)
        throw AllocationException("Creating FFTW plan.");

    /* Update N */
    _N = N;
}

RealDft::WindowFunction RealDft::getWindowFunction() {
    return _windowFunction;
}

void RealDft::setWindowFunction(WindowFunction wf) {
    _windowFunction = wf;
    calculateWindow(_window, _windowFunction);
}

}
