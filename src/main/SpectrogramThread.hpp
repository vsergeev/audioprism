#ifndef _SPECTROGRAMTHREAD_HPP
#define _SPECTROGRAMTHREAD_HPP

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

    /* Get/Set DFT Overlap (0.00 - 1.00) */
    float getDftOverlap();
    void setDftOverlap(float overlap);

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

    /* Get Spectrogram pixelPerHz lambda */
    std::function<float (int)> getPixelToHz(unsigned int width, unsigned int dftSize, unsigned int sampleRate);

  private:
    void run();

    /* Input samples queue */
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
    /* Output pixels queue */
    ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue;

    std::atomic<bool> running;

    DFT::RealDft realDft;
    std::mutex realDftLock;

    Spectrogram::SpectrumRenderer spectrumRenderer;
    std::mutex spectrumRendererLock;

    unsigned int dftOverlap;
    unsigned int pixelsWidth;

    std::thread thread;
};

#endif

