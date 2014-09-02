#include <iostream>
#include <thread>
#include <memory>
#include <exception>
#include <vector>
#include <mutex>
#include <cmath>

#include "SDL.h"

#include "PulseAudioSource.hpp"
#include "RealDft.hpp"
#include "Spectrogram.hpp"

#define DFT_SAMPLES     2048
#define READ_SAMPLES    128
#define WIDTH   640
#define HEIGHT  480

std::mutex pixels_lock;
uint32_t pixels[HEIGHT*WIDTH];

void thread_audio() {
    PulseAudioSource audio;
    RealDft dft(DFT_SAMPLES, WindowFunction::Hanning);
    Spectrogram spectrogram;

    while (true) {
        /* Move old samples */
        memmove(dft.samples.data(), dft.samples.data()+READ_SAMPLES, sizeof(double)*(DFT_SAMPLES-READ_SAMPLES));

        /* Read new samples */
        audio.read(dft.samples.data()+(DFT_SAMPLES-READ_SAMPLES), READ_SAMPLES);

        /* Compute DFT */
        dft.compute();

        /* Update pixels */
        pixels_lock.lock();
        spectrogram.update(pixels, WIDTH, HEIGHT, dft.magnitudes);
        pixels_lock.unlock();
    }
}

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

    std::thread audioThread(thread_audio);

    while (1) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                break;
        }

        pixels_lock.lock();
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(Uint32));
        pixels_lock.unlock();

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

