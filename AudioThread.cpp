#include "AudioSource.hpp"
#include "AudioThread.hpp"
#include "ThreadSafeQueue.hpp"

#define DEFAULT_READ_SIZE   128

AudioThread::AudioThread(AudioSource &source) : readSize(DEFAULT_READ_SIZE), source(source) { }

void AudioThread::run() {
    std::vector<double> samples(readSize);
    while (true) {
        if (samples.size() != readSize)
            samples.resize(readSize);

        source.read(samples.data(), samples.size());
        samplesQueue.push(samples);
    }
}

unsigned int AudioThread::getSampleRate() {
    return source.getSampleRate();
}

