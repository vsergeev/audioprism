#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>

#include "ThreadSafeQueue.hpp"

#include "AudioThread.hpp"
#include "RealDft.hpp"
#include "Spectrogram.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(AudioThread &audioThread, unsigned int width);
    void run();

    ThreadSafeQueue<std::vector<uint32_t>> pixelsQueue;

    unsigned int getDftSize();
    WindowFunction getWindowFunction();
    double getMagnitudeMin();
    double getMagnitudeMax();

    void setDftSize(unsigned int N);
    void setWindowFunction(WindowFunction wf);
    void setMagnitudeMin(double min);
    void setMagnitudeMax(double max);

  private:
    unsigned int width;
    AudioThread &audioThread;
    RealDft dft;
    Spectrogram spectrogram;
    std::mutex settingsLock;
};

#endif

