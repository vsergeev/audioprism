#ifndef _AUDIOTHREAD_HPP
#define _AUDIOTHREAD_HPP

#include <vector>
#include <atomic>

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "audio/AudioSource.hpp"

void AudioThread(ThreadSafeResource<Audio::AudioSource> &audioResource, ThreadSafeQueue<std::vector<double>> &samplesQueue, std::atomic<bool> &running);

#endif

