#include "AudioThread.hpp"

#define AUDIO_READ_SIZE 128

AudioThread::AudioThread(ThreadSafeQueue<std::vector<float>> &samplesQueue, const Configuration::Settings &initialSettings) : _samplesQueue(samplesQueue), _audioSource(initialSettings.audioSampleRate) {}

void AudioThread::start() {
    _running = true;
    _thread = std::thread(&AudioThread::_run, this);
}

void AudioThread::stop() {
    _running = false;
    _thread.join();
}

void AudioThread::_run() {
    std::vector<float> samples(AUDIO_READ_SIZE);

    while (_running) {
        {
            std::lock_guard<std::mutex> lg(_audioSourceLock);
            _audioSource.read(samples);
        }

        _samplesQueue.push(samples);
    }
}

unsigned int AudioThread::getSampleRate() {
    std::lock_guard<std::mutex> lg(_audioSourceLock);
    return _audioSource.getSampleRate();
}
