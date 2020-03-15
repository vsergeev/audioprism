#include <stdexcept>
#include <cmath>
#include <complex>

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

static void calculateWindow(std::vector<double> &window, RealDft::WindowFunction windowFunction) {
    size_t N = window.size();
    if (windowFunction == RealDft::WindowFunction::Hann) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 0.5 * (1 - std::cos((2.0 * M_PI * static_cast<double>(n)) / static_cast<double>(N - 1)));
    } else if (windowFunction == RealDft::WindowFunction::Hamming) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 0.54 - 0.46 * std::cos((2.0 * M_PI * static_cast<double>(n)) / static_cast<double>(N - 1));
    } else if (windowFunction == RealDft::WindowFunction::Bartlett) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 1.0 - std::abs((static_cast<double>(n) - static_cast<double>(N - 1) / 2.0) / (static_cast<double>(N - 1) / 2.0));
    } else if (windowFunction == RealDft::WindowFunction::Rectangular) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 1.0;
    }
}

RealDft::RealDft(unsigned int N, RealDft::WindowFunction wf) : _N(N), _windowFunction(wf), _wsamples(nullptr), _dft(nullptr), _plan(nullptr) {
    setSize(_N);
}

RealDft::~RealDft() {
    if (_plan) {
        fftw_destroy_plan(_plan);
        _plan = nullptr;
    }
    if (_dft) {
        fftw_free(_dft);
        _dft = nullptr;
    }
    if (_wsamples) {
        fftw_free(_wsamples);
        _wsamples = nullptr;
    }
    fftw_cleanup();
}

void RealDft::compute(std::vector<std::complex<double>> &dft, const std::vector<double> &samples) {
    /* Assert sample buffer size */
    if (samples.size() != _N)
        throw SizeMismatchException("Samples size does not match DFT size!");

    /* Size dft buffer correctly */
    dft.resize(_N / 2 + 1);

    /* Window samples first */
    for (unsigned int n = 0; n < _N; n++)
        _wsamples[n] = samples[n] * _window[n];

    /* Execute DFT */
    fftw_execute(_plan);

    /* Compute DFT magnitude */
    for (unsigned int n = 0; n < _N / 2 + 1; n++)
        dft[n] = std::complex<double>(_dft[n][0], _dft[n][1]);
}

unsigned int RealDft::getSize() {
    return _N;
}

void RealDft::setSize(unsigned int N) {
    /* Deallocate FFTW resources we are changing */
    if (_plan) {
        fftw_destroy_plan(_plan);
        _plan = nullptr;
    }
    if (_dft) {
        fftw_free(_dft);
        _dft = nullptr;
    }
    if (_wsamples) {
        fftw_free(_wsamples);
        _wsamples = nullptr;
    }

    /* Resize window */
    _window.resize(N);
    /* Recalculate window function */
    calculateWindow(_window, _windowFunction);

    /* Allocate windowed samples buffer */
    _wsamples = fftw_alloc_real(N);
    if (_wsamples == nullptr)
        throw AllocationException("Allocating sample memory.");

    /* Allocate DFT buffer */
    _dft = fftw_alloc_complex(N / 2 + 1);
    if (_dft == nullptr)
        throw AllocationException("Allocating DFT memory.");

    /* Rebuild our plan */
    _plan = fftw_plan_dft_r2c_1d(static_cast<int>(N), _wsamples, _dft, FFTW_MEASURE);

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
