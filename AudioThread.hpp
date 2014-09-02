#ifndef _AUDIO_THREAD_HPP
#define _AUDIO_THREAD_HPP

#include <vector>
#include <atomic>

#include "AudioSource.hpp"
#include "ThreadSafeQueue.hpp"

class AudioThread {
  public:
    AudioThread(AudioSource &source);
    void run();

    std::atomic<size_t> readSize;
    ThreadSafeQueue<std::vector<double>> samplesQueue;

    unsigned int getSampleRate();

  private:
    AudioSource &source;
};

#endif

