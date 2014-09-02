#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>

#include "RealDft.hpp"
#include "Spectrogram.hpp"
#include "ThreadSafeQueue.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, unsigned int pixelsWidth, unsigned int pixelsHeight);
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
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
    RealDft dft;
    Spectrogram spectrogram;
    std::mutex settingsLock;
};

#endif

