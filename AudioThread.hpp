#ifndef _AUDIO_THREAD_HPP
#define _AUDIO_THREAD_HPP

#include <vector>

#include "AudioSource.hpp"
#include "ThreadSafeQueue.hpp"

class AudioThread {
  public:
    AudioThread(AudioSource &source);
    ~AudioThread();
    void run();

    ThreadSafeQueue<std::vector<double>> samplesQueue;

  private:
    AudioSource &source;
};

#endif

