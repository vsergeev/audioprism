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
    ThreadSafeQueue<std::vector<uint32_t>> &_pixelsQueue;
    /* References to other threads for control */
    AudioThread &_audioThread;
    SpectrogramThread &_spectrogramThread;
    /* Running boolean */
    std::atomic<bool> _running;

    /* Owned resources (SDL) */
    SDL_Window *_win;
    SDL_Renderer *_renderer;
    SDL_Texture *_pixelsTexture;
    SDL_Texture *_settingsTexture;
    SDL_Texture *_cursorTexture;
    SDL_Texture *_statisticsTexture;
    SDL_Rect _settingsRect;
    SDL_Rect _cursorRect;
    SDL_Rect _statisticsRect;
    TTF_Font *_font;

    /* Interface settings */
    const unsigned int _width, _height;
    const Configuration::Orientation _orientation;
    bool _hideInfo, _hideStatistics;

    /* Helper functions for SDL */
    void _handleKeyDown(const uint8_t *state);
    void _updateSettings();
    void _renderSettings();
    void _renderCursor(int x, int y);
    void _renderStatistics();

    /* Cached settings from audio source, dft, and spectrogram classes */
    struct {
        unsigned int audioSampleRate;
        float samplesOverlap;
        DFT::RealDft::WindowFunction dftWindowFunction;
        unsigned int dftSize;
        float magnitudeMin;
        float magnitudeMax;
        bool magnitudeLog;
        Spectrogram::SpectrumRenderer::ColorScheme colorScheme;
    } _settings;
};

class SDLException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class TTFException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};
