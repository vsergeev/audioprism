#include <stdexcept>
#include <cmath>

#include "RealDft.hpp"

static void calculateWindow(std::vector<double> &window, WindowFunction windowFunction) {
    unsigned int N = window.size();
    if (windowFunction == WindowFunction::Hanning) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 0.5*(1-std::cos((2*M_PI*n)/(N-1)));
    } else if (windowFunction == WindowFunction::Hamming) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 0.54 - 0.46*std::cos((2*M_PI*n)/(N-1));
    } else if (windowFunction == WindowFunction::Rectangular) {
        for (unsigned int n = 0; n < N; n++)
            window[n] = 1.0;
    }
}

RealDft::RealDft(unsigned int N, WindowFunction wf) : N(N), windowFunction(wf), plan(nullptr), wsamples(nullptr), dft(nullptr) {
    setSize(N);
}

RealDft::~RealDft() {
    if (plan) {
        fftw_destroy_plan(plan);
        plan = nullptr;
    }
    if (wsamples) {
        fftw_free(wsamples);
        wsamples = nullptr;
    }
    if (dft) {
        fftw_free(dft);
        dft = nullptr;
    }
    fftw_cleanup();
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
    if (wsamples) {
        fftw_free(wsamples);
        wsamples = nullptr;
    }
    if (dft) {
        fftw_free(dft);
        dft = nullptr;
    }

    /* Resize external sample buffer */
    samples.resize(N);
    /* Resize window */
    window.resize(N);
    /* Recalculate window function */
    calculateWindow(window, windowFunction);

    /* Allocate DFT buffer */
    dft = fftw_alloc_complex(N/2+1);
    if (dft == NULL)
        throw std::runtime_error("Allocating DFT memory.");

    /* Allocate windowed samples buffer */
    wsamples = fftw_alloc_real(N);
    if (wsamples == NULL)
        throw std::runtime_error("Allocating sample memory.");

    /* Resize DFT magnitude buffer */
    magnitudes.resize(N/2+1);

    /* Rebuild our plan */
    plan = fftw_plan_dft_r2c_1d(N, wsamples, dft, FFTW_MEASURE);

    /* Update N */
    this->N = N;
}

WindowFunction RealDft::getWindowFunction() {
    return windowFunction;
}

void RealDft::setWindowFunction(WindowFunction wf) {
    windowFunction = wf;
    calculateWindow(window, windowFunction);
}

void RealDft::compute() {
    /* Window samples first */
    for (unsigned int n = 0; n < N; n++)
        wsamples[n] = samples[n]*window[n];

    /* Execute DFT */
    fftw_execute(plan);

    /* Compute DFT magnitude */
    for (unsigned int n = 0; n < N/2+1; n++)
        magnitudes[n] = 20*log10(sqrt(dft[n][0]*dft[n][0] + dft[n][1]*dft[n][1]));
}

