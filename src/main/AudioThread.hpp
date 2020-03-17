#pragma once

#include <vector>
#include <atomic>
#include <thread>

#include "ThreadSafeQueue.hpp"
#include "audio/PulseAudioSource.hpp"
#include "Configuration.hpp"

class AudioThread {
  public:
    AudioThread(ThreadSafeQueue<std::vector<float>> &samplesQueue, const Configuration::Settings &initialSettings);

    void start();
    void stop();

    /* Get AudioSource sample rate in Hz */
    unsigned int getSampleRate();

  private:
    void _run();

    /* Output samples queue */
    ThreadSafeQueue<std::vector<float>> &_samplesQueue;

    Audio::PulseAudioSource _audioSource;
    std::mutex _audioSourceLock;

    std::atomic<bool> _running;
    std::thread _thread;
};
