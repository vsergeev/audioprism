#ifndef _AUDIOTHREAD_HPP
#define _AUDIOTHREAD_HPP

#include <vector>
#include <atomic>
#include <thread>

#include "ThreadSafeQueue.hpp"
#include "audio/PulseAudioSource.hpp"
#include "Configuration.hpp"

class AudioThread {
  public:
    AudioThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, const Configuration::Settings &initialSettings);

    void start();
    void stop();

    /* Get AudioSource sample rate in Hz */
    unsigned int getSampleRate();

  private:
    void run();

    /* Output samples queue */
    ThreadSafeQueue<std::vector<double>> &samplesQueue;

    std::atomic<bool> running;
    Audio::PulseAudioSource audioSource;
    std::mutex audioSourceLock;

    std::thread thread;
};

#endif
