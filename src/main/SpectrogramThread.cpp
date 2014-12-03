#include <cstring>
#include <complex>
#include <unistd.h>

#include <iostream>

#include "SpectrogramThread.hpp"

void SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, ThreadSafeResource<DFT::RealDft> &dftResource, ThreadSafeResource<Spectrogram::SpectrumRenderer> &spectrogramResource, std::atomic<unsigned int> &dftOverlap, unsigned int width, std::atomic<bool> &running) {
    /* Audio Sample Buffer */
    std::vector<double> audioSamples;
    /* Overlapped Samples */
    std::vector<double> overlapSamples;
    /* DFT of overlapped samples */
    std::vector<std::complex<double>> dftSamples;
    /* Pixel line */
    std::vector<uint32_t> pixels(width);

    while (running) {
        std::vector<double> newSamples(samplesQueue.pop());
        audioSamples.insert(audioSamples.end(), newSamples.begin(), newSamples.end());

        {
            std::lock_guard<ThreadSafeResource<DFT::RealDft>> dftLg(dftResource);
            std::lock_guard<ThreadSafeResource<Spectrogram::SpectrumRenderer>> spectrogramLg(spectrogramResource);

            /* Resize samples buffer and dft samples buffer */
            if (overlapSamples.size() != dftResource.get().getSize()) {
                overlapSamples.resize(dftResource.get().getSize());
                dftSamples.resize(dftResource.get().getSize()/2+1);
            }

            if (audioSamples.size() < dftOverlap)
                continue;

            #if 0
            unsigned int x = dftOverlap;
            printf("before overlap %u audioSamples %zu overlapSamples %zu dftSamples %zu\n", x, audioSamples.size(), overlapSamples.size(), dftSamples.size());
            #endif

            /* Move down old samples */
            memmove(overlapSamples.data(), overlapSamples.data()+dftOverlap, sizeof(double)*(overlapSamples.size()-dftOverlap));
            /* Copy new samples */
            memcpy(overlapSamples.data()+dftOverlap, audioSamples.data(), sizeof(double)*dftOverlap);
            /* Erase used audio samples */
            audioSamples.erase(audioSamples.begin(), audioSamples.begin()+dftOverlap);

            #if 0
            printf("after  overlap %u audioSamples %zu overlapSamples %zu dftSamples %zu\n", x, audioSamples.size(), overlapSamples.size(), dftSamples.size());
            #endif

            /* Compute DFT */
            dftResource.get().compute(dftSamples, overlapSamples);

            /* Render spectrogram line */
            spectrogramResource.get().render(pixels, dftSamples);

            /* Put into pixels queue */
            pixelsQueue.push(pixels);
        }
    }
}

