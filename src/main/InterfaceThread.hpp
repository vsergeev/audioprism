#ifndef _INTERFACE_THREAD_HPP
#define _INTERFACE_THREAD_HPP

#include <functional>

#include <SDL.h>
#include <SDL_ttf.h>

#include "ThreadSafeQueue.hpp"
#include "ThreadSafeResource.hpp"

#include "Orientation.hpp"
#include "AudioSource.hpp"
#include "RealDft.hpp"
#include "Spectrogram.hpp"
#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"

class InterfaceThread {
  public:
    InterfaceThread(ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, ThreadSafeResource<AudioSource> &audioResource, ThreadSafeResource<RealDft> &dftResource, ThreadSafeResource<Spectrogram> &spectrogramResource, std::atomic<size_t> &audioReadSize, std::atomic<bool> &running, unsigned int width, unsigned int height, Orientation orientation);
    ~InterfaceThread();

    void run();

  private:
    /* Shared resources */
    ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue;
    ThreadSafeResource<AudioSource> &audioResource;
    ThreadSafeResource<RealDft> &dftResource;
    ThreadSafeResource<Spectrogram> &spectrogramResource;
    std::atomic<size_t> &audioReadSize;
    std::atomic<bool> &running;

    /* Owned resources (SDL) */
    SDL_Window *win;
    SDL_Renderer *renderer;
    SDL_Texture *pixelsTexture;
    SDL_Texture *settingsTexture;
    SDL_Texture *cursorTexture;
    SDL_Rect settingsRect;
    SDL_Rect cursorRect;
    TTF_Font *font;
    /* Interface settings */
    const unsigned int width, height;
    const Orientation orientation;
    bool hideInfo;

    /* Helper functions for SDL */
    void updateSettings();
    void renderSettings();
    void renderCursor(int x, int y);
    void handleKeyDown(const uint8_t *state);

    /* Cached settings from audio source, dft, and spectrogram classes */
    struct {
        unsigned int audioSampleRate;
        unsigned int audioReadSize;
        RealDft::WindowFunction dftWf;
        unsigned int dftSize;
        std::function<float (int)> fPixelToHz;
        double magnitudeMin;
        double magnitudeMax;
        bool magnitudeLog;
        Spectrogram::ColorScheme colors;
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

#endif

