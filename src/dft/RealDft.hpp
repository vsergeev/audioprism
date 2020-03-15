#pragma once

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
    void compute(std::vector<std::complex<double>> &dft, const std::vector<double> &samples);

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
    std::vector<double> _window;
    /* Windowed Samples */
    double *_wsamples;
    /* Complex DFT */
    fftw_complex *_dft;
    /* FFTW Plan */
    fftw_plan _plan;
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
