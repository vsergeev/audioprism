#pragma once

#include <functional>

#include <SDL.h>
#include <SDL_ttf.h>

#include "ThreadSafeQueue.hpp"
#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"
#include "Configuration.hpp"

class InterfaceThread {
  public:
    InterfaceThread(ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, AudioThread &audioThread, SpectrogramThread &spectrogramThread, const Configuration::Settings &initialSettings);
    ~InterfaceThread();

    void run();

  private:
    /* Pixels input queue */
    ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue;
    /* References to other threads for control */
    AudioThread &audioThread;
    SpectrogramThread &spectrogramThread;
    /* Running boolean */
    std::atomic<bool> running;

    /* Owned resources (SDL) */
    SDL_Window *win;
    SDL_Renderer *renderer;
    SDL_Texture *pixelsTexture;
    SDL_Texture *settingsTexture;
    SDL_Texture *cursorTexture;
    SDL_Texture *statisticsTexture;
    SDL_Rect settingsRect;
    SDL_Rect cursorRect;
    SDL_Rect statisticsRect;
    TTF_Font *font;

    /* Interface settings */
    const unsigned int width, height;
    const Configuration::Orientation orientation;
    bool hideInfo, hideStatistics;

    /* Helper functions for SDL */
    void handleKeyDown(const uint8_t *state);
    void updateSettings();
    void renderSettings();
    void renderCursor(int x, int y);
    void renderStatistics();

    /* Cached settings from audio source, dft, and spectrogram classes */
    struct {
        unsigned int audioSampleRate;
        float samplesOverlap;
        DFT::RealDft::WindowFunction dftWf;
        unsigned int dftSize;
        double magnitudeMin;
        double magnitudeMax;
        bool magnitudeLog;
        Spectrogram::SpectrumRenderer::ColorScheme colors;
    } settings;
};

class SDLException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class TTFException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};
