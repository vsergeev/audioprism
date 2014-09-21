#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>
#include <atomic>

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "RealDft.hpp"
#include "Spectrogram.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, ThreadSafeResource<RealDft> &dftResource, ThreadSafeResource<Spectrogram> &spectrogramResource, unsigned int sampleRate, unsigned int width);
    void run();
    std::atomic<bool> running;

  private:
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
    ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue;
    ThreadSafeResource<RealDft> &dftResource;
    ThreadSafeResource<Spectrogram> &spectrogramResource;
    unsigned int sampleRate;
    unsigned int width;
};

#endif

