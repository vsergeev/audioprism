#include <iostream>
#include <cstring>
#include <unistd.h>

#include "SpectrogramThread.hpp"

#define DEFAULT_DFT_SIZE    2048
#define DEFAULT_WINDOW_FUNC WindowFunction::Hanning

SpectrogramThread::SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, unsigned int sampleRate, unsigned int width) : samplesQueue(samplesQueue), pixelsQueue(pixelsQueue), sampleRate(sampleRate), width(width), dft(DEFAULT_DFT_SIZE, DEFAULT_WINDOW_FUNC), spectrogram() { }

void SpectrogramThread::run() {
    std::vector<uint32_t> pixelLine(width);

    while (true) {
        std::lock_guard<std::mutex> lg(settingsLock);

        std::vector<double> samples = samplesQueue.pop();

        /* Move down old samples */
        memmove(dft.samples.data(), dft.samples.data()+samples.size(), sizeof(double)*(dft.getSize()-samples.size()));
        /* Add new samples */
        memcpy(dft.samples.data()+(dft.getSize()-samples.size()), samples.data(), sizeof(double)*samples.size());

        /* Compute DFT */
        dft.compute();

        /* Render spectrogram line */
        spectrogram.render(pixelLine, dft.magnitudes);

        /* Put into pixels queue */
        pixelsQueue.push(pixelLine);
    }
}

unsigned int SpectrogramThread::getDftSize() {
    std::lock_guard<std::mutex> lg(settingsLock);
    return dft.getSize();
}

WindowFunction SpectrogramThread::getWindowFunction() {
    std::lock_guard<std::mutex> lg(settingsLock);
    return dft.getWindowFunction();
}

double SpectrogramThread::getMagnitudeMin() {
    std::lock_guard<std::mutex> lg(settingsLock);
    return spectrogram.settings.magnitudeMin;
}

double SpectrogramThread::getMagnitudeMax() {
    std::lock_guard<std::mutex> lg(settingsLock);
    return spectrogram.settings.magnitudeMax;
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
    spectrogram.settings.magnitudeMin = min;
}

void SpectrogramThread::setMagnitudeMax(double max) {
    std::lock_guard<std::mutex> lg(settingsLock);
    spectrogram.settings.magnitudeMax = max;
}

