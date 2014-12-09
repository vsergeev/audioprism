#include "AudioThread.hpp"

AudioThread::AudioThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, const Configuration::Settings &initialSettings) : samplesQueue(samplesQueue), audioSource(initialSettings.audioSampleRate) {}

void AudioThread::start() {
    thread = std::thread(&AudioThread::run, this);
}

void AudioThread::stop() {
    running = false;
    thread.join();
}

#define AUDIO_READ_SIZE 128

void AudioThread::run() {
    std::vector<double> samples(AUDIO_READ_SIZE);

    running = true;

    while (running) {
        {
            std::lock_guard<std::mutex> lg(audioSourceLock);
            audioSource.read(samples);
        }

        samplesQueue.push(samples);
    }
}

unsigned int AudioThread::getSampleRate() {
    std::lock_guard<std::mutex> lg(audioSourceLock);
    return audioSource.getSampleRate();
}
