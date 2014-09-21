#ifndef _AUDIO_THREAD_HPP
#define _AUDIO_THREAD_HPP

#include <vector>
#include <atomic>

#include "AudioSource.hpp"
#include "ThreadSafeQueue.hpp"

class AudioThread {
  public:
    AudioThread(AudioSource &audioSource, std::mutex &audioSourceLock, ThreadSafeQueue<std::vector<double>> &samplesQueue, size_t readSize);
    void run();
    std::atomic<bool> running;

    std::atomic<size_t> readSize;

  private:
    AudioSource &audioSource;
    std::mutex &audioSourceLock;
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
};

#endif

