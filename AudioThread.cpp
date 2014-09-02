#include "AudioSource.hpp"
#include "AudioThread.hpp"
#include "ThreadSafeQueue.hpp"

#define NUM_SAMPLES 2048

AudioThread::AudioThread(AudioSource &source) : readSize(128), source(source) { }

void AudioThread::run() {
    std::vector<double> samples(readSize);
    while (true) {
        if (samples.size() != readSize)
            samples.resize(readSize);

        source.read(samples.data(), samples.size());
        samplesQueue.push(samples);
    }
}

