#include "AudioSource.hpp"
#include "AudioThread.hpp"
#include "ThreadSafeQueue.hpp"

AudioThread::AudioThread(ThreadSafeResource<AudioSource> &audioSourceResource, ThreadSafeQueue<std::vector<double>> &samplesQueue, size_t readSize) : readSize(readSize), audioSourceResource(audioSourceResource), samplesQueue(samplesQueue) { }

void AudioThread::run() {
    std::vector<double> samples(readSize);

    running = true;

    while (running) {
        if (samples.size() != readSize)
            samples.resize(readSize);

        {
            std::lock_guard<ThreadSafeResource<AudioSource>> lg(audioSourceResource);
            audioSourceResource.get().read(samples);
        }

        samplesQueue.push(samples);
    }
}

