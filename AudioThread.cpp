#include "AudioSource.hpp"
#include "AudioThread.hpp"
#include "ThreadSafeQueue.hpp"

#define NUM_SAMPLES 4096

AudioThread::AudioThread(AudioSource &source) : source(source) { }

AudioThread::~AudioThread() { }

void AudioThread::run() {
    std::vector<double> samples(NUM_SAMPLES);
    while (true) {
        source.read(samples.data(), NUM_SAMPLES);
        samplesQueue.push(samples);
    }
}

