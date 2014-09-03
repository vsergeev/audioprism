#ifndef _AUDIO_THREAD_HPP
#define _AUDIO_THREAD_HPP

#include <vector>
#include <atomic>

#include "AudioSource.hpp"
#include "ThreadSafeQueue.hpp"

class AudioThread {
  public:
    AudioThread(AudioSource &source, ThreadSafeQueue<std::vector<double>> &samplesQueue);
    void run();

    std::atomic<size_t> readSize;

    unsigned int getSampleRate();

  private:
    AudioSource &source;
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
};

#endif

