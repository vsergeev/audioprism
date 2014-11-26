#include "audio/AudioSource.hpp"

#include "main/AudioThread.hpp"
#include "main/ThreadSafeQueue.hpp"

void AudioThread(ThreadSafeResource<Audio::AudioSource> &audioResource, ThreadSafeQueue<std::vector<double>> &samplesQueue, std::atomic<size_t> &readSize, std::atomic<bool> &running) {
    std::vector<double> samples(readSize);

    while (running) {
        if (samples.size() != readSize)
            samples.resize(readSize);

        {
            std::lock_guard<ThreadSafeResource<Audio::AudioSource>> lg(audioResource);
            audioResource.get().read(samples);
        }

        samplesQueue.push(samples);
    }
}

