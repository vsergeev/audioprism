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
            window[n] = 0.5*(1-std::cos((2.0*M_PI*static_cast<double>(n))/static_cast<double>(N-1)));
    } else if (windowFunction == RealDft::WindowFunction::Hamming) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 0.54 - 0.46*std::cos((2.0*M_PI*static_cast<double>(n))/static_cast<double>(N-1));
    } else if (windowFunction == RealDft::WindowFunction::Bartlett) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 1.0 - std::abs((static_cast<double>(n) - static_cast<double>(N-1)/2.0)/(static_cast<double>(N-1)/2.0));
    } else if (windowFunction == RealDft::WindowFunction::Rectangular) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 1.0;
    }
}

RealDft::RealDft(unsigned int N, RealDft::WindowFunction wf) : N(N), windowFunction(wf), wsamples(nullptr), dft(nullptr), plan(nullptr) {
    setSize(N);
}

RealDft::~RealDft() {
    if (plan) {
        fftw_destroy_plan(plan);
        plan = nullptr;
    }
    if (dft) {
        fftw_free(dft);
        dft = nullptr;
    }
    if (wsamples) {
        fftw_free(wsamples);
        wsamples = nullptr;
    }
    fftw_cleanup();
}

void RealDft::compute(std::vector<std::complex<double>> &dft, const std::vector<double> &samples) {
    /* Assert sample buffer size */
    if (samples.size() != N)
        throw SizeMismatchException("Samples size does not match DFT size!");

    /* Size dft buffer correctly */
    dft.resize(N/2+1);

    /* Window samples first */
    for (unsigned int n = 0; n < N; n++)
        wsamples[n] = samples[n]*window[n];

    /* Execute DFT */
    fftw_execute(plan);

    /* Compute DFT magnitude */
    for (unsigned int n = 0; n < N/2+1; n++)
        dft[n] = std::complex<double>(this->dft[n][0], this->dft[n][1]);
}

unsigned int RealDft::getSize() {
    return N;
}

void RealDft::setSize(unsigned int N) {
    /* Deallocate FFTW resources we are changing */
    if (plan) {
        fftw_destroy_plan(plan);
        plan = nullptr;
    }
    if (dft) {
        fftw_free(dft);
        dft = nullptr;
    }
    if (wsamples) {
        fftw_free(wsamples);
        wsamples = nullptr;
    }

    /* Resize window */
    window.resize(N);
    /* Recalculate window function */
    calculateWindow(window, windowFunction);

    /* Allocate windowed samples buffer */
    wsamples = fftw_alloc_real(N);
    if (wsamples == nullptr)
        throw AllocationException("Allocating sample memory.");

    /* Allocate DFT buffer */
    dft = fftw_alloc_complex(N/2+1);
    if (dft == nullptr)
        throw AllocationException("Allocating DFT memory.");

    /* Rebuild our plan */
    plan = fftw_plan_dft_r2c_1d(static_cast<int>(N), wsamples, dft, FFTW_MEASURE);

    /* Update N */
    this->N = N;
}

RealDft::WindowFunction RealDft::getWindowFunction() {
    return windowFunction;
}

void RealDft::setWindowFunction(WindowFunction wf) {
    windowFunction = wf;
    calculateWindow(window, windowFunction);
}

}

