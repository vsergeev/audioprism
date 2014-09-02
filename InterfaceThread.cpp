#include <SDL.h>

#include "InterfaceThread.hpp"

InterfaceThread::InterfaceThread(AudioThread &audioThread, SpectrogramThread &spectrogramThread, unsigned int width, unsigned int height) :
  audioThread(audioThread), spectrogramThread(spectrogramThread), width(width), height(height) {
    int ret;

    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0)
        throw std::runtime_error("Unable to initialize SDL: SDL_Init(): " + std::string(SDL_GetError()));

    win = SDL_CreateWindow("Spectrogram", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    if (win == NULL)
        throw std::runtime_error("Erroring creating SDL window: SDL_CreateWindow(): " + std::string(SDL_GetError()));

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        throw std::runtime_error("Erroring creating SDL renderer: SDL_CreateRenderer(): " + std::string(SDL_GetError()));

    pixelsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);
    if (pixelsTexture == NULL)
        throw std::runtime_error("Erroring creating SDL renderer: SDL_CreateTexture(): " + std::string(SDL_GetError()));
}

InterfaceThread::~InterfaceThread() {
    SDL_DestroyTexture(pixelsTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

#include <iostream>

void InterfaceThread::run() {
    while (true) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                break;
            if (e.type == SDL_KEYDOWN) {
                const uint8_t *state = SDL_GetKeyboardState(NULL);
                if (state[SDL_SCANCODE_Q]) {
                    break;
                }
            }
        }

        spectrogramThread.pixels_lock.lock();
        SDL_UpdateTexture(pixelsTexture, NULL, spectrogramThread.pixels, width * sizeof(uint32_t));
        spectrogramThread.pixels_lock.unlock();

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, pixelsTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }
}

