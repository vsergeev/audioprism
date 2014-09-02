#ifndef _REAL_DFT_HPP
#define _REAL_DFT_HPP

#include <vector>

#include <fftw3.h>

enum class WindowFunction { Hanning, Hamming, Rectangular };

std::ostream &operator<<(std::ostream &os, const WindowFunction &wf);

class RealDft {
  public:
    RealDft(unsigned int N, WindowFunction wf);
    ~RealDft();

    /* Compute new DFT magnitude based on samples */
    void compute();

    /* Get DFT size */
    unsigned int getSize();
    /* Get Window Function */
    WindowFunction getWindowFunction();

    /* Set DFT size */
    void setSize(unsigned int N);
    /* Set Window Function */
    void setWindowFunction(WindowFunction wf);

    std::vector<double> samples;
    std::vector<double> magnitudes;

  private:
    unsigned int N;
    WindowFunction windowFunction;
    fftw_plan plan;
    double *wsamples;
    fftw_complex *dft;
    std::vector<double> window;
};

#endif

