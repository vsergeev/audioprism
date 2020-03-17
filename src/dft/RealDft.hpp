#pragma once

#include <stdexcept>
#include <vector>
#include <complex>

#include <fftw3.h>

namespace DFT {

class RealDft {
  public:
    enum class WindowFunction { Hann,
                                Hamming,
                                Bartlett,
                                Rectangular };

    RealDft(unsigned int N, WindowFunction wf);
    ~RealDft();

    /* Compute new DFT magnitude based on samples */
    void compute(std::vector<std::complex<float>> &dft, const std::vector<float> &samples);

    /* Get/Set DFT Size */
    unsigned int getSize();
    void setSize(unsigned int N);

    /* Get/Set Window Function */
    WindowFunction getWindowFunction();
    void setWindowFunction(WindowFunction wf);

  private:
    /* DFT Size */
    unsigned int _N;
    /* Window Function */
    WindowFunction _windowFunction;
    /* Window */
    std::vector<float> _window;
    /* Windowed Samples */
    float *_windowedSamples;
    /* Complex DFT */
    fftwf_complex *_dft;
    /* FFTW Plan */
    fftwf_plan _plan;
};

class AllocationException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class SizeMismatchException : public std::length_error {
  public:
    using std::length_error::length_error;
};

std::ostream &operator<<(std::ostream &os, const RealDft::WindowFunction &wf);
std::string to_string(const RealDft::WindowFunction &wf);

}
