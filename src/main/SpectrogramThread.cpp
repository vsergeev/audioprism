#include <cstring>
#include <complex>
#include <unistd.h>

#include "SpectrogramThread.hpp"

void SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, ThreadSafeResource<DFT::RealDft> &dftResource, ThreadSafeResource<Spectrogram::SpectrumRenderer> &spectrogramResource, unsigned int width, std::atomic<bool> &running) {
    std::vector<double> samples;
    std::vector<std::complex<double>> dftSamples;
    std::vector<uint32_t> pixels(width);

    while (running) {
        std::vector<double> newSamples(samplesQueue.pop());

        {
            std::lock_guard<ThreadSafeResource<DFT::RealDft>> dftLg(dftResource);
            std::lock_guard<ThreadSafeResource<Spectrogram::SpectrumRenderer>> spectrogramLg(spectrogramResource);

            /* Resize samples buffer and dft samples buffer */
            if (samples.size() != dftResource.get().getSize()) {
                samples.resize(dftResource.get().getSize());
                dftSamples.resize(dftResource.get().getSize());
            }

            if (newSamples.size() >= samples.size()) {
                /* Copy over new samples */
                memcpy(samples.data(), newSamples.data(), samples.size()*sizeof(double));
            } else {
                /* Move down old samples */
                memmove(samples.data(), samples.data()+newSamples.size(), sizeof(double)*(samples.size()-newSamples.size()));
                /* Copy new samples */
                memcpy(samples.data()+(samples.size()-newSamples.size()), newSamples.data(), sizeof(double)*newSamples.size());
            }

            /* Compute DFT */
            dftResource.get().compute(dftSamples, samples);

            /* Render spectrogram line */
            spectrogramResource.get().render(pixels, dftSamples);

            /* Put into pixels queue */
            pixelsQueue.push(pixels);
        }
    }
}

