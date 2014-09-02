#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>

#include "RealDft.hpp"
#include "Spectrogram.hpp"
#include "ThreadSafeQueue.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(ThreadSafeQueue<std::vector<double>> &sampleQueue, unsigned int pixelsWidth, unsigned int pixelsHeight);
    ~SpectrogramThread();
    void run();

    std::mutex pixels_lock;
    const uint32_t *pixels;

  private:
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
    RealDft dft;
    Spectrogram spectrogram;
};

#endif

