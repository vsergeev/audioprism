#ifndef _SPECTROGRAM_THREAD_HPP
#define _SPECTROGRAM_THREAD_HPP

#include <vector>

#include "ThreadSafeQueue.hpp"

#include "AudioThread.hpp"
#include "RealDft.hpp"
#include "Spectrogram.hpp"

class SpectrogramThread {
  public:
    SpectrogramThread(ThreadSafeQueue<std::vector<double>> &samplesQueue, ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, unsigned int sampleRate, unsigned int width, unsigned int dftSize, WindowFunction wf, double magnitudeMin, double magnitudeMax, bool magnitudeLog, Spectrogram::ColorScheme colors);
    void run();
    std::atomic<bool> running;

    unsigned int getDftSize();
    WindowFunction getWindowFunction();
    double getMagnitudeMin();
    double getMagnitudeMax();
    bool getMagnitudeLog();
    std::function<float (int)> getPixelToHz();
    Spectrogram::ColorScheme getColorScheme();

    void setDftSize(unsigned int N);
    void setWindowFunction(WindowFunction wf);
    void setMagnitudeMin(double min);
    void setMagnitudeMax(double max);
    void setMagnitudeLog(bool value);
    void setColorScheme(Spectrogram::ColorScheme colors);

  private:
    ThreadSafeQueue<std::vector<double>> &samplesQueue;
    ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue;
    unsigned int sampleRate;
    unsigned int width;
    RealDft dft;
    Spectrogram spectrogram;
    std::mutex settingsLock;
};

#endif

