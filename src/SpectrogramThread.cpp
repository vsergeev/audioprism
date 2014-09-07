#include <iostream>
#include <cstring>
#include <complex>
#include <unistd.h>

#include "SpectrogramThread.hpp"

SpectrogramThread::SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, unsigned int sampleRate, unsigned int width, unsigned int dftSize, WindowFunction wf, double magnitudeMin, double magnitudeMax, Spectrogram::ColorScheme colors) : samplesQueue(samplesQueue), pixelsQueue(pixelsQueue), sampleRate(sampleRate), width(width), dft(dftSize, wf), spectrogram(magnitudeMin, magnitudeMax, colors) { }

void SpectrogramThread::run() {
    std::vector<double> samples(dft.getSize());
    std::vector<std::complex<double>> dftSamples;
    std::vector<uint32_t> pixels(width);

    running = true;

    while (running) {
        std::vector<double> newSamples(samplesQueue.pop());

        {
            std::lock_guard<std::mutex> lg(settingsLock);

            if (samples.size() != dft.getSize())
                samples.resize(dft.getSize());

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

std::function<float (int)> SpectrogramThread::getPixelToHz() {
    std::lock_guard<std::mutex> lg(settingsLock);
    return spectrogram.getPixelToHz(width, dft.getSize(), sampleRate);
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

