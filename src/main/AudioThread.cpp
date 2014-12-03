#include "audio/AudioSource.hpp"

#include "main/AudioThread.hpp"
#include "main/ThreadSafeQueue.hpp"

#define READ_SIZE   128

void AudioThread(ThreadSafeResource<Audio::AudioSource> &audioResource, ThreadSafeQueue<std::vector<double>> &samplesQueue, std::atomic<bool> &running) {
    std::vector<double> samples(READ_SIZE);

    while (running) {
        {
            std::lock_guard<ThreadSafeResource<Audio::AudioSource>> lg(audioResource);
            audioResource.get().read(samples);
        }

        samplesQueue.push(samples);
    }
}

