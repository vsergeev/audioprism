#include <cstring>
#include <complex>
#include <unistd.h>

#include "SpectrogramThread.hpp"

SpectrogramThread::SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, RealDft &dft, std::mutex &dftLock, Spectrogram &spectrogram, std::mutex &spectrogramLock, unsigned int sampleRate, unsigned int width) : samplesQueue(samplesQueue), pixelsQueue(pixelsQueue), dft(dft), dftLock(dftLock), spectrogram(spectrogram), spectrogramLock(spectrogramLock), sampleRate(sampleRate), width(width) { }

void SpectrogramThread::run() {
    std::vector<double> samples;
    std::vector<std::complex<double>> dftSamples;
    std::vector<uint32_t> pixels(width);

    running = true;

    while (running) {
        std::vector<double> newSamples(samplesQueue.pop());

        {
            std::lock_guard<std::mutex> dftLg(dftLock);
            std::lock_guard<std::mutex> spectrogramLg(spectrogramLock);

            /* Resize samples buffer and dft samples buffer */
            if (samples.size() != dft.getSize()) {
                samples.resize(dft.getSize());
                dftSamples.resize(dft.getSize());
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
            dft.compute(dftSamples, samples);

            /* Render spectrogram line */
            spectrogram.render(pixels, dftSamples);

            /* Put into pixels queue */
            pixelsQueue.push(pixels);
        }
    }
}

