#include <iostream>
#include <thread>
#include <memory>
#include <exception>
#include <vector>
#include <mutex>
#include <cmath>

#include <unistd.h>

#include "SDL.h"

#include "PulseAudioSource.hpp"
#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"

#define SAMPLE_RATE 48000
#define WIDTH       640
#define HEIGHT      480

int main(int argc, char *argv[]) {
    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("Spectrogram", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    if (win == NULL) {
        std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
        exit(1);
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cerr << "Error creating SDL renderer: " << SDL_GetError() << std::endl;
        exit(1);
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
    if (texture == NULL) {
        std::cerr << "Error creating SDL texture: " << SDL_GetError() << std::endl;
        exit(1);
    }

    PulseAudioSource audioSource(SAMPLE_RATE);
    AudioThread audioThread(audioSource);
    SpectrogramThread spectrogramThread(audioThread.samplesQueue, WIDTH, HEIGHT);

    std::thread at(&AudioThread::run, &audioThread);
    std::thread st(&SpectrogramThread::run, &spectrogramThread);

    while (1) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                break;
        }

        spectrogramThread.pixels_lock.lock();
        SDL_UpdateTexture(texture, NULL, spectrogramThread.pixels, WIDTH * sizeof(uint32_t));
        spectrogramThread.pixels_lock.unlock();

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

