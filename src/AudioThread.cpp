#include "AudioSource.hpp"
#include "AudioThread.hpp"
#include "ThreadSafeQueue.hpp"

AudioThread::AudioThread(AudioSource &audioSource, std::mutex &audioSourceLock, ThreadSafeQueue<std::vector<double>> &samplesQueue, size_t readSize) : readSize(readSize), audioSource(audioSource), audioSourceLock(audioSourceLock), samplesQueue(samplesQueue) { }

void AudioThread::run() {
    std::vector<double> samples(readSize);

    running = true;

    while (running) {
        if (samples.size() != readSize)
            samples.resize(readSize);

        {
            std::lock_guard<std::mutex> lg(audioSourceLock);
            audioSource.read(samples);
        }

        samplesQueue.push(samples);
    }
}

