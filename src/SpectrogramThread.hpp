#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>
#include <atomic>

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "RealDft.hpp"
#include "Spectrogram.hpp"

void SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, ThreadSafeResource<RealDft> &dftResource, ThreadSafeResource<Spectrogram> &spectrogramResource, unsigned int width, std::atomic<bool> &running);

#endif

