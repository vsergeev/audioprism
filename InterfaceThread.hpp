#ifndef _INTERFACE_THREAD_HPP
#define _INTERFACE_THREAD_HPP

#include <SDL.h>

#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"
#include "ThreadSafeQueue.hpp"

class InterfaceThread {
  public:
    InterfaceThread(ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, AudioThread &audioThread, SpectrogramThread &spectrogramThread, unsigned int width, unsigned int height);
    ~InterfaceThread();

    void run();

  private:
    ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue;
    AudioThread &audioThread;
    SpectrogramThread &spectrogramThread;
    const unsigned int width, height;

    SDL_Window *win;
    SDL_Renderer *renderer;
    SDL_Texture *pixelsTexture;
    SDL_Texture *infoTexture;
};

#endif

