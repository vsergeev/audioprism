#include <cstring>
#include <complex>
#include <unistd.h>

#include "SpectrogramThread.hpp"

SpectrogramThread::SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, const Configuration::Settings &initialSettings) : samplesQueue(samplesQueue), pixelsQueue(pixelsQueue), realDft(initialSettings.dftSize, initialSettings.dftWf), spectrumRenderer(initialSettings.magnitudeMin, initialSettings.magnitudeMax, initialSettings.magnitudeLog, initialSettings.colors) {
    dftOverlap = static_cast<unsigned int>(initialSettings.dftOverlap*static_cast<float>(initialSettings.dftSize));
    pixelsWidth = (initialSettings.orientation == Configuration::Orientation::Vertical) ? initialSettings.width : initialSettings.height;
}

void SpectrogramThread::start() {
    thread = std::thread(&SpectrogramThread::run, this);
}

void SpectrogramThread::stop() {
    running = false;
    thread.join();
}

void SpectrogramThread::run() {
    /* Audio Samples Buffer */
    std::vector<double> audioSamples;
    /* Overlapped Samples */
    std::vector<double> overlapSamples;
    /* DFT of Overlapped Samples */
    std::vector<std::complex<double>> dftSamples;
    /* Pixel line */
    std::vector<uint32_t> pixels(pixelsWidth);

    running = true;

    while (running) {
        std::vector<double> newAudioSamples(samplesQueue.pop());

        /* Add new audio samples to our audio samples buffer */
        audioSamples.insert(audioSamples.end(), newAudioSamples.begin(), newAudioSamples.end());

        std::lock_guard<std::mutex> dftLg(realDftLock);

        /* If we don't have enough samples to update overlap window, continue to pop more */
        if (audioSamples.size() < dftOverlap)
            continue;

        /* Resize overlap samples buffer and dft samples buffer if N changed */
        if (overlapSamples.size() != realDft.getSize()) {
            overlapSamples.resize(realDft.getSize());
            dftSamples.resize(realDft.getSize()/2+1);
        }

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
        realDft.compute(dftSamples, overlapSamples);

        std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);

        /* Render spectrogram line */
        spectrumRenderer.render(pixels, dftSamples);

        /* Put into pixels queue */
        pixelsQueue.push(pixels);
    }
}

float SpectrogramThread::getDftOverlap() {
    std::lock_guard<std::mutex> dftLg(realDftLock);
    return static_cast<float>(dftOverlap)/static_cast<float>(realDft.getSize());
}

void SpectrogramThread::setDftOverlap(float overlap) {
    std::lock_guard<std::mutex> dftLg(realDftLock);
    dftOverlap = static_cast<unsigned int>(overlap*static_cast<float>(realDft.getSize()));
}

unsigned int SpectrogramThread::getDftSize() {
    std::lock_guard<std::mutex> dftLg(realDftLock);
    return realDft.getSize();
}

void SpectrogramThread::setDftSize(unsigned int N) {
    float overlap = getDftOverlap();

    {
        std::lock_guard<std::mutex> dftLg(realDftLock);
        realDft.setSize(N);
    }

    /* Preserve overlap percentage */
    setDftOverlap(overlap);
}

DFT::RealDft::WindowFunction SpectrogramThread::getDftWindowFunction() {
    std::lock_guard<std::mutex> dftLg(realDftLock);
    return realDft.getWindowFunction();
}

void SpectrogramThread::setDftWindowFunction(DFT::RealDft::WindowFunction wf) {
    std::lock_guard<std::mutex> dftLg(realDftLock);
    realDft.setWindowFunction(wf);
}

double SpectrogramThread::getMagnitudeMin() {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    return spectrumRenderer.settings.magnitudeMin;
}

void SpectrogramThread::setMagnitudeMin(double min) {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    spectrumRenderer.settings.magnitudeMin = min;
}

double SpectrogramThread::getMagnitudeMax() {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    return spectrumRenderer.settings.magnitudeMax;
}

void SpectrogramThread::setMagnitudeMax(double max) {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    spectrumRenderer.settings.magnitudeMax = max;
}

bool SpectrogramThread::getMagnitudeLog() {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    return spectrumRenderer.settings.magnitudeLog;
}

void SpectrogramThread::setMagnitudeLog(bool logarithmic) {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    spectrumRenderer.settings.magnitudeLog = logarithmic;
}

Spectrogram::SpectrumRenderer::ColorScheme SpectrogramThread::getColors() {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    return spectrumRenderer.settings.colors;
}

void SpectrogramThread::setColors(Spectrogram::SpectrumRenderer::ColorScheme colors) {
    std::lock_guard<std::mutex> spectrumLg(spectrumRendererLock);
    spectrumRenderer.settings.colors = colors;
}

