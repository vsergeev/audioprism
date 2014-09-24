#ifndef _AUDIO_THREAD_HPP
#define _AUDIO_THREAD_HPP

#include <vector>
#include <atomic>

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "AudioSource.hpp"

void AudioThread(ThreadSafeResource<AudioSource> &audioResource, ThreadSafeQueue<std::vector<double>> &samplesQueue, std::atomic<size_t> &readSize, std::atomic<bool> &running);

#endif

