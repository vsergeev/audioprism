#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>

#include "ThreadSafeQueue.hpp"

#include "AudioThread.hpp"
#include "RealDft.hpp"
#include "Spectrogram.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, RealDft &dft, std::mutex &dftLock, Spectrogram &spectrogram, std::mutex &spectrogramLock, unsigned int sampleRate, unsigned int width);
    void run();
    std::atomic<bool> running;

  private:
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
    ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue;
    RealDft &dft;
    std::mutex &dftLock;
    Spectrogram &spectrogram;
    std::mutex &spectrogramLock;
    unsigned int sampleRate;
    unsigned int width;
};

#endif

