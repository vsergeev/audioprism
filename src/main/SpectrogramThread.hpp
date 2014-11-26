#ifndef _SPECTROGRAMTHREAD_HPP
#define _SPECTROGRAMTHREAD_HPP

#include <vector>
#include <atomic>

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "dft/RealDft.hpp"
#include "spectrogram/SpectrumRenderer.hpp"

void SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, ThreadSafeResource<DFT::RealDft> &dftResource, ThreadSafeResource<Spectrogram::SpectrumRenderer> &spectrogramResource, unsigned int width, std::atomic<bool> &running);

#endif

