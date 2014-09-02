#include <iostream>
#include <cstring>
#include <unistd.h>

#include "SpectrogramThread.hpp"

#define DEFAULT_DFT_SIZE    1024
#define DEFAULT_WINDOW_FUNC WindowFunction::Hanning

SpectrogramThread::SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, unsigned int width, unsigned int height) :
  samplesQueue(samplesQueue), dft(DEFAULT_DFT_SIZE, DEFAULT_WINDOW_FUNC), spectrogram(width, height) {
    pixels = spectrogram.getPixels();
}

void SpectrogramThread::run() {
    while (true) {
        if (!samplesQueue.empty()) {
            std::vector<double> samples = samplesQueue.pop();

            /* Move down old samples */
            memmove(dft.samples.data(), dft.samples.data()+samples.size(), sizeof(double)*(dft.getSize()-samples.size()));
            /* Add new samples */
            memcpy(dft.samples.data()+(dft.getSize()-samples.size()), samples.data(), sizeof(double)*samples.size());

            /* Compute DFT */
            dft.compute();

            /* Update spectrogram */
            pixels_lock.lock();
            spectrogram.update(dft.magnitudes);
            pixels_lock.unlock();
        }

        std::cout << "Sample queue: " << samplesQueue.count() << "\r";
        usleep(5);
    }
}

