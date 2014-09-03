#include <memory>

#include <SDL.h>

#include "InterfaceThread.hpp"

InterfaceThread::InterfaceThread(ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, AudioThread &audioThread, SpectrogramThread &spectrogramThread, unsigned int width, unsigned int height) : pixelsQueue(pixelsQueue), audioThread(audioThread), spectrogramThread(spectrogramThread), width(width), height(height) {
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

    pixelsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (pixelsTexture == NULL)
        throw std::runtime_error("Erroring creating SDL renderer: SDL_CreateTexture(): " + std::string(SDL_GetError()));
}

InterfaceThread::~InterfaceThread() {
    SDL_DestroyTexture(pixelsTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void InterfaceThread::run() {
    std::unique_ptr<uint32_t []> pixels = std::unique_ptr<uint32_t []>(new uint32_t[width*height]);
    std::vector<uint32_t> newPixelRows;

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

        /* Collect all new pixel rows */
        while (!pixelsQueue.empty()) {
            std::vector<uint32_t> pixelRow(pixelsQueue.pop());
            newPixelRows.insert(newPixelRows.end(), pixelRow.begin(), pixelRow.end());
        }

        if (newPixelRows.size() > 0) {
            if (newPixelRows.size() >= width*height) {
                /* This should seldom happen. */

                /* Overwrite all pixels */
                memcpy(pixels.get(), newPixelRows.data()+(newPixelRows.size()-width*height), width*height);
                /* Clear new pixels */
                newPixelRows.clear();
            } else if (newPixelRows.size() < width*height) {
                /* Move old pixels up */
                memmove(pixels.get(), pixels.get()+newPixelRows.size(), (width*height-newPixelRows.size())*sizeof(uint32_t));
                /* Copy new pixels over */
                memcpy(pixels.get()+(width*height-newPixelRows.size()), newPixelRows.data(), newPixelRows.size()*sizeof(uint32_t));
                /* Clear new pixels */
                newPixelRows.clear();
            }
            SDL_UpdateTexture(pixelsTexture, NULL, pixels.get(), width * sizeof(uint32_t));
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, pixelsTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
}

