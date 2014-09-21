#ifndef _AUDIO_THREAD_HPP
#define _AUDIO_THREAD_HPP

#include <vector>
#include <atomic>

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "AudioSource.hpp"

class AudioThread {
  public:
    AudioThread(ThreadSafeResource<AudioSource> &audioSourceResource, ThreadSafeQueue<std::vector<double>> &samplesQueue, size_t readSize);
    void run();
    std::atomic<bool> running;

    std::atomic<size_t> readSize;

  private:
    ThreadSafeResource<AudioSource> &audioSourceResource;
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
};

#endif

