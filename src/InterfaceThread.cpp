#include <sstream>

#include <SDL.h>
#include <SDL_ttf.h>

#include "InterfaceThread.hpp"

InterfaceThread::InterfaceThread(ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, AudioThread &audioThread, SpectrogramThread &spectrogramThread, unsigned int width, unsigned int height) : pixelsQueue(pixelsQueue), audioThread(audioThread), spectrogramThread(spectrogramThread), width(width), height(height) {
    int ret;

    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0)
        throw std::runtime_error("Unable to initialize SDL: SDL_Init(): " + std::string(SDL_GetError()));

    ret = TTF_Init();
    if (ret < 0)
        throw std::runtime_error("Unable to initialize TTF: TTF_Init(): " + std::string(TTF_GetError()));

    win = SDL_CreateWindow("Spectrogram", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    if (win == nullptr)
        throw std::runtime_error("Erroring creating SDL window: SDL_CreateWindow(): " + std::string(SDL_GetError()));

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
        throw std::runtime_error("Erroring creating SDL renderer: SDL_CreateRenderer(): " + std::string(SDL_GetError()));

    pixelsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);
    if (pixelsTexture == nullptr)
        throw std::runtime_error("Erroring creating SDL texture: SDL_CreateTexture(): " + std::string(SDL_GetError()));

    infoTexture = nullptr;
    cursorTexture = nullptr;

    font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSansMono-Bold.ttf", 11);
    if (font == nullptr)
        throw std::runtime_error("Erroring opening TTF font: TTF_OpenFont(): " + std::string(TTF_GetError()));
}

InterfaceThread::~InterfaceThread() {
    TTF_CloseFont(font);
    if (pixelsTexture)
        SDL_DestroyTexture(pixelsTexture);
    if (infoTexture)
        SDL_DestroyTexture(infoTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
}

static std::string format(const char *fmt, ...) {
    char buf[64];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return std::string(buf);
}

static SDL_Surface *renderString(std::string s, TTF_Font *font, const SDL_Color &color) {
    SDL_Surface *surface;

    surface = TTF_RenderText_Solid(font, s.c_str(), color);
    if (surface == nullptr)
        throw std::runtime_error("Error rendering text: TTF_RenderText_Solid(): " + std::string(TTF_GetError()));

    return surface;
}

enum class Alignment { Left, Center, Right };

static SDL_Surface *vcatSurfaces(std::vector<SDL_Surface *> surfaces, Alignment aligned) {
    SDL_Surface *targetSurface = nullptr;
    int targetSurfaceWidth = 0, targetSurfaceHeight = 0;

    /* Compute size of target surface */
    for (SDL_Surface *surface : surfaces) {
        if (surface->w > targetSurfaceWidth)
            targetSurfaceWidth = surface->w;
        targetSurfaceHeight += surface->h;
    }

    /* Create target surface */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN /* ghetto */
    targetSurface = SDL_CreateRGBSurface(0, targetSurfaceWidth, targetSurfaceHeight, 32, 0xff << 24, 0xff << 16, 0xff << 8, 0xff);
    #else
    targetSurface = SDL_CreateRGBSurface(0, targetSurfaceWidth, targetSurfaceHeight, 32, 0xff, 0xff << 8, 0xff << 16, 0xff << 24);
    #endif
    if (targetSurface == nullptr)
        throw std::runtime_error("Error creating target surface: SDL_CreateRGBSurface(): " + std::string(SDL_GetError()));

    /* Blit each text surface onto the target surface */
    int offset = 0;
    for (SDL_Surface *surface : surfaces) {
        SDL_Rect targetRect;

        if (aligned == Alignment::Left)
            targetRect.x = 0;
        else if (aligned == Alignment::Center)
            targetRect.x = (targetSurfaceWidth - surface->w)/2;
        else if (aligned == Alignment::Right)
            targetRect.x = targetSurfaceWidth - surface->w;

        targetRect.y = offset;
        targetRect.w = surface->w;
        targetRect.h = surface->h;

        offset += surface->h;

        if (SDL_BlitSurface(surface, nullptr, targetSurface, &targetRect) < 0)
            throw std::runtime_error("Error blitting text surfaces: SDL_BlitSurface(): " + std::string(SDL_GetError()));
        SDL_FreeSurface(surface);
    }

    return targetSurface;
}

void InterfaceThread::renderInfo() {
    std::vector<SDL_Surface *> textSurfaces;
    SDL_Surface *targetSurface;
    SDL_Color infoColor = {0xff, 0x00, 0x00, 0x00};

    unsigned int dftSize = spectrogramThread.getDftSize();
    unsigned int overlap = static_cast<unsigned int>((1.0-(static_cast<float>(audioThread.readSize)/static_cast<float>(dftSize)))*100.0);
    fPixelToHz = spectrogramThread.getPixelToHz();

    textSurfaces.push_back(renderString(format("Sample Rate: %d Hz", audioThread.getSampleRate()), font, infoColor));
    textSurfaces.push_back(renderString(format("Sample Overlap: %d%%", overlap), font, infoColor));
    textSurfaces.push_back(renderString("Window: " + to_string(spectrogramThread.getWindowFunction()), font, infoColor));
    textSurfaces.push_back(renderString(format("DFT Size: %d", dftSize), font, infoColor));
    textSurfaces.push_back(renderString(format("Mag. min: %.2f dB", spectrogramThread.getMagnitudeMin()), font, infoColor));
    textSurfaces.push_back(renderString(format("Mag. max: %.2f dB", spectrogramThread.getMagnitudeMax()), font, infoColor));

    targetSurface = vcatSurfaces(textSurfaces, Alignment::Right);

    /* Update info rectangle destination for screen rendering */
    infoRect.x = width - targetSurface->w - 5;
    infoRect.y = 2;
    infoRect.w = targetSurface->w;
    infoRect.h = targetSurface->h;

    /* Destroy old info texture */
    if (infoTexture)
        SDL_DestroyTexture(infoTexture);

    /* Create new texture from the target surface */
    infoTexture = SDL_CreateTextureFromSurface(renderer, targetSurface);
    if (infoTexture == NULL)
        throw std::runtime_error("Error creating texture for text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(targetSurface);
}

void InterfaceThread::renderCursor(int x) {
    SDL_Surface *cursorSurface;
    SDL_Color infoColor = {0xff, 0x00, 0x00, 0x00};
    float frequency;

    frequency = fPixelToHz(x);

    cursorSurface = renderString(format("%.0f Hz", frequency), font, infoColor);

    cursorRect.x = width - cursorSurface->w - 5;
    cursorRect.y = infoRect.h + cursorSurface->h;
    cursorRect.w = cursorSurface->w;
    cursorRect.h = cursorSurface->h;

    if (cursorTexture)
        SDL_DestroyTexture(cursorTexture);

    cursorTexture = SDL_CreateTextureFromSurface(renderer, cursorSurface);
    if (cursorTexture == NULL)
        throw std::runtime_error("Error creating texture for cursor text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(cursorSurface);
}

void InterfaceThread::run() {
    std::unique_ptr<uint32_t []> pixels = std::unique_ptr<uint32_t []>(new uint32_t[width*height]);
    std::vector<uint32_t> newPixelRows;

    renderInfo();

    while (true) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            int mx;

            if (e.type == SDL_QUIT)
                break;
            if (e.type == SDL_KEYDOWN) {
                const uint8_t *state = SDL_GetKeyboardState(nullptr);
                if (state[SDL_SCANCODE_Q]) {
                    break;
                }
            }

            SDL_GetMouseState(&mx, NULL);
            renderCursor(mx);
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
            SDL_UpdateTexture(pixelsTexture, nullptr, pixels.get(), width * sizeof(uint32_t));
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, pixelsTexture, nullptr, nullptr);
        SDL_RenderCopy(renderer, infoTexture, nullptr, &infoRect);
        SDL_RenderCopy(renderer, cursorTexture, nullptr, &cursorRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
}

