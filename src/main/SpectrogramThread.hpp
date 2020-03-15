#pragma once

#include <vector>
#include <atomic>
#include <thread>

#include "ThreadSafeQueue.hpp"
#include "dft/RealDft.hpp"
#include "spectrogram/SpectrumRenderer.hpp"
#include "Configuration.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, const Configuration::Settings &initialSettings);

    void start();
    void stop();

    /* Get/Set Samples Overlap (0.00 - 1.00) */
    float getSamplesOverlap();
    void setSamplesOverlap(float overlap);

    /* Get/Set DFT Size (power of two) */
    unsigned int getDftSize();
    void setDftSize(unsigned int N);

    /* Get/Set DFT Window Function */
    DFT::RealDft::WindowFunction getDftWindowFunction();
    void setDftWindowFunction(DFT::RealDft::WindowFunction wf);

    /* Get/Set Spectrogram Magnitude Minimum */
    double getMagnitudeMin();
    void setMagnitudeMin(double min);

    /* Get/Set Spectrogram Magnitude Maximum */
    double getMagnitudeMax();
    void setMagnitudeMax(double max);

    /* Get/Set Spectrogram Magnitude Logarithmic/Linear */
    bool getMagnitudeLog();
    void setMagnitudeLog(bool logarithmic);

    /* Get/Set Spectrogram Color Scheme */
    Spectrogram::SpectrumRenderer::ColorScheme getColors();
    void setColors(Spectrogram::SpectrumRenderer::ColorScheme colors);

    /* Debug Statistics */
    size_t getDebugSamplesQueueCount();

  private:
    void _run();

    /* Input samples queue */
    ThreadSafeQueue<std::vector<double>> &_samplesQueue;
    /* Output pixels queue */
    ThreadSafeQueue<std::vector<uint32_t>> &_pixelsQueue;

    std::atomic<bool> _running;

    DFT::RealDft _realDft;
    std::mutex _realDftLock;

    Spectrogram::SpectrumRenderer _spectrumRenderer;
    std::mutex _spectrumRendererLock;

    unsigned int _samplesOverlap;
    unsigned int _pixelsWidth;

    std::thread _thread;

    std::atomic<size_t> _samplesQueueCount;
};
