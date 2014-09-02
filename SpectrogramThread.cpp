#include <iostream>
#include <cstring>
#include <unistd.h>

#include "SpectrogramThread.hpp"

#define DEFAULT_DFT_SIZE    1024
#define DEFAULT_WINDOW_FUNC WindowFunction::Hanning

SpectrogramThread::SpectrogramThread(AudioThread &audioThread, unsigned int width, unsigned int height) :
  audioThread(audioThread), dft(DEFAULT_DFT_SIZE, DEFAULT_WINDOW_FUNC), spectrogram(width, height) {
    pixels = spectrogram.getPixels();
}

void SpectrogramThread::run() {
    while (true) {
        settingsLock.lock();

        if (!audioThread.samplesQueue.empty()) {
            std::vector<double> samples = audioThread.samplesQueue.pop();

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

        std::cout << "Audio sample queue: " << audioThread.samplesQueue.count() << "\r";

        settingsLock.unlock();

        usleep(5);

    }
}

unsigned int SpectrogramThread::getDftSize() {
    return dft.getSize();
}

WindowFunction SpectrogramThread::getWindowFunction() {
    return dft.getWindowFunction();
}

double SpectrogramThread::getMagnitudeMin() {
    return spectrogram.magnitudeMin;
}

double SpectrogramThread::getMagnitudeMax() {
    return spectrogram.magnitudeMax;
}

void SpectrogramThread::setDftSize(unsigned int N) {
    std::lock_guard<std::mutex> lg(settingsLock);
    dft.setSize(N);
}

void SpectrogramThread::setWindowFunction(WindowFunction wf) {
    std::lock_guard<std::mutex> lg(settingsLock);
    dft.setWindowFunction(wf);
}

void SpectrogramThread::setMagnitudeMin(double min) {
    std::lock_guard<std::mutex> lg(settingsLock);
    spectrogram.magnitudeMin = min;
}

void SpectrogramThread::setMagnitudeMax(double max) {
    std::lock_guard<std::mutex> lg(settingsLock);
    spectrogram.magnitudeMax = max;
}

