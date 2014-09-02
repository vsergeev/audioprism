#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>

#include "AudioThread.hpp"
#include "RealDft.hpp"
#include "Spectrogram.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(AudioThread &audioThread, unsigned int width, unsigned int height);
    void run();

    std::mutex pixels_lock;
    const uint32_t *pixels;

    unsigned int getDftSize();
    WindowFunction getWindowFunction();
    double getMagnitudeMin();
    double getMagnitudeMax();

    void setDftSize(unsigned int N);
    void setWindowFunction(WindowFunction wf);
    void setMagnitudeMin(double min);
    void setMagnitudeMax(double max);

  private:
    AudioThread &audioThread;
    RealDft dft;
    Spectrogram spectrogram;
    std::mutex settingsLock;
};

#endif

