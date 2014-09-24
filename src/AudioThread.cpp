#include "AudioSource.hpp"
#include "AudioThread.hpp"
#include "ThreadSafeQueue.hpp"

void AudioThread(ThreadSafeResource<AudioSource> &audioResource, ThreadSafeQueue<std::vector<double>> &samplesQueue, std::atomic<size_t> &readSize, std::atomic<bool> &running) {
    std::vector<double> samples(readSize);

    while (running) {
        if (samples.size() != readSize)
            samples.resize(readSize);

        {
            std::lock_guard<ThreadSafeResource<AudioSource>> lg(audioResource);
            audioResource.get().read(samples);
        }

        samplesQueue.push(samples);
    }
}

