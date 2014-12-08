#include <ftw.h>
#include <fnmatch.h>
#include <sstream>
#include <algorithm>
#include <map>

#include <iostream>

#include <SDL.h>
#include <SDL_ttf.h>

#include "InterfaceThread.hpp"
#include "Configuration.hpp"

using namespace Audio;
using namespace DFT;
using namespace Spectrogram;
using namespace Configuration;

static const std::string FontDirectory = "/usr/share/fonts";

static const std::vector<std::string> FontFilesSearch = {
    "DejaVuSansMono-Bold.ttf",
    "VeraMoBd.ttf",
    "UbuntuMono-R.ttf",
    "LiberationMono-Regular.ttf",
    "FreeMono.ttf",
};

static std::map<std::string, std::string> FontFilesAvailable;

static int fontCrawlCallback(const char *fpath, const struct stat *sb, int typeflag) {
    (void)sb;

    if (typeflag == FTW_F) {
        std::string path = std::string(fpath);

        /* Find the last slash */
        auto slashpos = path.rfind("/");

        if (slashpos != std::string::npos) {
            /* Extract filename */
            std::string filename = path.substr(slashpos+1);

            /* If it's a TTF file, add it to our available font files map */
            if (fnmatch("*.ttf", filename.c_str(), FNM_CASEFOLD) == 0)
                FontFilesAvailable[filename] = path;
        }
    }

    return 0;
}

static std::string findFontPath() {
    /* Crawl font directory to build a map of all TTF fonts */
    if (ftw(FontDirectory.c_str(), fontCrawlCallback, 5) < 0)
        throw std::runtime_error("Unable to crawl font directory " + FontDirectory);

    /* Look for any matches with our desired font files */
    for (const auto &fontFile : FontFilesSearch) {
        if (FontFilesAvailable.find(fontFile) != FontFilesAvailable.end())
            return FontFilesAvailable[fontFile];
    }

    return "";
}

InterfaceThread::InterfaceThread(ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, AudioThread &audioThread, SpectrogramThread &spectrogramThread, const Settings &initialSettings) : pixelsQueue(pixelsQueue), audioThread(audioThread), spectrogramThread(spectrogramThread), width(initialSettings.width), height(initialSettings.height), orientation(initialSettings.orientation), hideInfo(false), hideStatistics(true) {
    int ret;

    /* Initialize SDL */
    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0)
        throw SDLException("Unable to initialize SDL: SDL_Init(): " + std::string(SDL_GetError()));

    /* Initialize TTF */
    ret = TTF_Init();
    if (ret < 0)
        throw TTFException("Unable to initialize TTF: TTF_Init(): " + std::string(TTF_GetError()));

    /* Create Window */
    win = SDL_CreateWindow("spectrogram", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_OPENGL);
    if (win == nullptr)
        throw SDLException("Creating SDL window: SDL_CreateWindow(): " + std::string(SDL_GetError()));

    /* Create Renderer */
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
        throw SDLException("Creating SDL renderer: SDL_CreateRenderer(): " + std::string(SDL_GetError()));

    /* Create main texture */
    pixelsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, static_cast<int>(width), static_cast<int>(height));
    if (pixelsTexture == nullptr)
        throw SDLException("Creating SDL texture: SDL_CreateTexture(): " + std::string(SDL_GetError()));

    settingsTexture = nullptr;
    cursorTexture = nullptr;
    font = nullptr;

    /* Find a compatible font */
    std::string fontPath = findFontPath();
    if (fontPath == "")
        throw TTFException("Could not find a compatible TTF font.");

    /* Open font */
    font = TTF_OpenFont(fontPath.c_str(), 11);
    if (font == nullptr)
        throw TTFException("Opening TTF font: TTF_OpenFont(): " + std::string(TTF_GetError()));
}

InterfaceThread::~InterfaceThread() {
    TTF_CloseFont(font);
    if (pixelsTexture)
        SDL_DestroyTexture(pixelsTexture);
    if (settingsTexture)
        SDL_DestroyTexture(settingsTexture);
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
        throw TTFException("Error rendering text: TTF_RenderText_Solid(): " + std::string(TTF_GetError()));

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
    targetSurface = SDL_CreateRGBSurface(0, targetSurfaceWidth, targetSurfaceHeight, 32, 0xffu << 24, 0xffu << 16, 0xffu << 8, 0xffu);
    #else
    targetSurface = SDL_CreateRGBSurface(0, targetSurfaceWidth, targetSurfaceHeight, 32, 0xffu, 0xffu << 8, 0xffu << 16, 0xffu << 24);
    #endif
    if (targetSurface == nullptr)
        throw SDLException("Error creating target surface: SDL_CreateRGBSurface(): " + std::string(SDL_GetError()));

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
            throw SDLException("Error blitting text surfaces: SDL_BlitSurface(): " + std::string(SDL_GetError()));
        SDL_FreeSurface(surface);
    }

    return targetSurface;
}

void InterfaceThread::updateSettings() {
    settings.audioSampleRate = audioThread.getSampleRate();
    settings.samplesOverlap = spectrogramThread.getSamplesOverlap();
    settings.dftSize = spectrogramThread.getDftSize();
    settings.dftWf = spectrogramThread.getDftWindowFunction();
    settings.magnitudeMin = spectrogramThread.getMagnitudeMin();
    settings.magnitudeMax = spectrogramThread.getMagnitudeMax();
    settings.magnitudeLog = spectrogramThread.getMagnitudeLog();
    settings.colors = spectrogramThread.getColors();
}

void InterfaceThread::renderSettings() {
    std::vector<SDL_Surface *> textSurfaces;
    SDL_Surface *settingsSurface;
    SDL_Color settingsColor = {0xff, 0x00, 0x00, 0x00};

    unsigned int overlap = static_cast<unsigned int>(settings.samplesOverlap*100.0);

    textSurfaces.push_back(renderString(format("Sample Rate: %d Hz", settings.audioSampleRate), font, settingsColor));
    textSurfaces.push_back(renderString(format("Overlap: %d%%", overlap), font, settingsColor));
    textSurfaces.push_back(renderString("Window: " + to_string(settings.dftWf), font, settingsColor));
    textSurfaces.push_back(renderString(format("DFT Size: %d", settings.dftSize), font, settingsColor));
    textSurfaces.push_back(renderString(format("Colors: %s", to_string(settings.colors).c_str()), font, settingsColor));
    if (settings.magnitudeLog) {
        textSurfaces.push_back(renderString(format("Mag. min: %.2f dB", settings.magnitudeMin), font, settingsColor));
        textSurfaces.push_back(renderString(format("Mag. max: %.2f dB", settings.magnitudeMax), font, settingsColor));
        textSurfaces.push_back(renderString(format("Logarithmic"), font, settingsColor));
    } else {
        textSurfaces.push_back(renderString(format("Mag. min: %.2f", settings.magnitudeMin), font, settingsColor));
        textSurfaces.push_back(renderString(format("Mag. max: %.2f", settings.magnitudeMax), font, settingsColor));
        textSurfaces.push_back(renderString(format("Linear"), font, settingsColor));
    }

    settingsSurface = vcatSurfaces(textSurfaces, Alignment::Right);

    /* Update settings rectangle destination for screen rendering */
    settingsRect.x = static_cast<int>(width) - settingsSurface->w - 5;
    settingsRect.y = 2;
    settingsRect.w = settingsSurface->w;
    settingsRect.h = settingsSurface->h;

    /* Destroy old settings texture */
    if (settingsTexture)
        SDL_DestroyTexture(settingsTexture);

    /* Create new texture from the target surface */
    settingsTexture = SDL_CreateTextureFromSurface(renderer, settingsSurface);
    if (settingsTexture == nullptr)
        throw SDLException("Error creating texture for text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(settingsSurface);
}

void InterfaceThread::renderCursor(int x, int y) {
    SDL_Surface *cursorSurface;
    SDL_Color settingsColor = {0xff, 0x00, 0x00, 0x00};

    float frequency;

    float hzPerBin = ((static_cast<float>(settings.audioSampleRate))/2.0f)/static_cast<float>((settings.dftSize/2 + 1));

    if (orientation == Orientation::Vertical) {
        float binPerPixel = static_cast<float>((settings.dftSize/2 + 1))/static_cast<float>(width);
        frequency = std::floor(static_cast<float>(x)*binPerPixel)*hzPerBin;
    } else {
        float binPerPixel = static_cast<float>((settings.dftSize/2 + 1))/static_cast<float>(height);
        frequency = std::floor(static_cast<float>(static_cast<int>(height)-y)*binPerPixel)*hzPerBin;
    }

    cursorSurface = renderString(format("%.0f Hz", frequency), font, settingsColor);

    /* Update cursor rectangle destination for screen rendering */
    cursorRect.x = static_cast<int>(width) - cursorSurface->w - 5;
    cursorRect.y = settingsRect.y + settingsRect.h + cursorSurface->h;
    cursorRect.w = cursorSurface->w;
    cursorRect.h = cursorSurface->h;

    if (cursorTexture)
        SDL_DestroyTexture(cursorTexture);

    cursorTexture = SDL_CreateTextureFromSurface(renderer, cursorSurface);
    if (cursorTexture == nullptr)
        throw SDLException("Error creating texture for cursor text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(cursorSurface);
}

void InterfaceThread::renderStatistics() {
    std::vector<SDL_Surface *> textSurfaces;
    SDL_Surface *statisticsSurface;
    SDL_Color statisticsColor = {0xff, 0x00, 0x00, 0x00};

    size_t samplesQueueCount = spectrogramThread.getDebugSamplesQueueCount();
    size_t pixelsQueueCount = pixelsQueue.count();

    textSurfaces.push_back(renderString(format("Audio Queue: %u", samplesQueueCount), font, statisticsColor));
    textSurfaces.push_back(renderString(format("Pixels Queue: %u", pixelsQueueCount), font, statisticsColor));
    statisticsSurface = vcatSurfaces(textSurfaces, Alignment::Right);

    /* Update statistics rectangle destination for screen rendering */
    statisticsRect.x = static_cast<int>(width) - statisticsSurface->w - 5;
    statisticsRect.y = cursorRect.y + cursorRect.h + statisticsSurface->h;
    statisticsRect.w = statisticsSurface->w;
    statisticsRect.h = statisticsSurface->h;

    if (statisticsTexture)
        SDL_DestroyTexture(statisticsTexture);

    statisticsTexture = SDL_CreateTextureFromSurface(renderer, statisticsSurface);
    if (statisticsTexture == nullptr)
        throw SDLException("Error creating texture for statistics text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(statisticsSurface);
}

void InterfaceThread::handleKeyDown(const uint8_t *state) {
    if (state[SDL_SCANCODE_Q]) {
        running = false;
    } else if (state[SDL_SCANCODE_C]) {
        /* Change color scheme */
        SpectrumRenderer::ColorScheme next_colors = SpectrumRenderer::ColorScheme::Heat;

        if (settings.colors == SpectrumRenderer::ColorScheme::Heat)
            next_colors = SpectrumRenderer::ColorScheme::Blue;
        else if (settings.colors == SpectrumRenderer::ColorScheme::Blue)
            next_colors = SpectrumRenderer::ColorScheme::Grayscale;
        else if (settings.colors == SpectrumRenderer::ColorScheme::Grayscale)
            next_colors = SpectrumRenderer::ColorScheme::Heat;

        spectrogramThread.setColors(next_colors);
        settings.colors = spectrogramThread.getColors();
    } else if (state[SDL_SCANCODE_W]) {
        /* Change window function */
        RealDft::WindowFunction next_wf = RealDft::WindowFunction::Hanning;

        if (settings.dftWf == RealDft::WindowFunction::Hanning)
            next_wf = RealDft::WindowFunction::Hamming;
        else if (settings.dftWf == RealDft::WindowFunction::Hamming)
            next_wf = RealDft::WindowFunction::Rectangular;
        else if (settings.dftWf == RealDft::WindowFunction::Rectangular)
            next_wf = RealDft::WindowFunction::Hanning;

        spectrogramThread.setDftWindowFunction(next_wf);
        settings.dftWf = spectrogramThread.getDftWindowFunction();
    } else if (state[SDL_SCANCODE_L]) {
        /* Toggle between Logarithimic/Linear */
        bool next_magnitudeLog = !settings.magnitudeLog;

        spectrogramThread.setMagnitudeLog(next_magnitudeLog);
        settings.magnitudeLog = next_magnitudeLog;
        if (next_magnitudeLog) {
            spectrogramThread.setMagnitudeMin(InitialSettings.magnitudeLogMin);
            spectrogramThread.setMagnitudeMax(InitialSettings.magnitudeLogMax);
        } else {
            spectrogramThread.setMagnitudeMin(InitialSettings.magnitudeLinearMin);
            spectrogramThread.setMagnitudeMax(InitialSettings.magnitudeLinearMax);
        }
        settings.magnitudeMin = spectrogramThread.getMagnitudeMin();
        settings.magnitudeMax = spectrogramThread.getMagnitudeMax();
    } else if (state[SDL_SCANCODE_RIGHT]) {
        /* DFT N up */
        unsigned int next_dftSize = std::min<unsigned int>(settings.dftSize*2, UserLimits.dftSizeMax);

        if (next_dftSize != settings.dftSize) {
            spectrogramThread.setDftSize(next_dftSize);
            /* Reset samples overlap to 50% */
            spectrogramThread.setSamplesOverlap(0.50);

            settings.dftSize = spectrogramThread.getDftSize();
            settings.samplesOverlap = spectrogramThread.getSamplesOverlap();
        }
    } else if (state[SDL_SCANCODE_LEFT]) {
        /* DFT N down */
        unsigned int next_dftSize = std::max<unsigned int>(settings.dftSize/2, UserLimits.dftSizeMin);

        /* Set Samples Overlap for 50% overlap */
        if (next_dftSize != settings.dftSize) {
            spectrogramThread.setDftSize(next_dftSize);
            /* Reset samples overlap to 50% */
            spectrogramThread.setSamplesOverlap(0.50);

            settings.dftSize = spectrogramThread.getDftSize();
            settings.samplesOverlap = spectrogramThread.getSamplesOverlap();
        }
    } else if (state[SDL_SCANCODE_DOWN]) {
        /* Samples Overlap Up */
        float next_samplesOverlap = std::max<float>(settings.samplesOverlap - UserLimits.samplesOverlapStep, UserLimits.samplesOverlapMin);

        spectrogramThread.setSamplesOverlap(next_samplesOverlap);
        settings.samplesOverlap = spectrogramThread.getSamplesOverlap();
    } else if (state[SDL_SCANCODE_UP]) {
        /* Samples Overlap Down */
        float next_samplesOverlap = std::min<float>(settings.samplesOverlap + UserLimits.samplesOverlapStep, UserLimits.samplesOverlapMax);

        spectrogramThread.setSamplesOverlap(next_samplesOverlap);
        settings.samplesOverlap = spectrogramThread.getSamplesOverlap();
    } else if (state[SDL_SCANCODE_MINUS]) {
        /* Magnitude min down */
        double next_magnitudeMin;

        if (settings.magnitudeLog)
            next_magnitudeMin = std::max<double>(settings.magnitudeMin - UserLimits.magnitudeLogStep, UserLimits.magnitudeLogMin);
        else
            next_magnitudeMin = std::max<double>(settings.magnitudeMin - UserLimits.magnitudeLinearStep, UserLimits.magnitudeLinearMin);

        spectrogramThread.setMagnitudeMin(next_magnitudeMin);
        settings.magnitudeMin = spectrogramThread.getMagnitudeMin();
    } else if (state[SDL_SCANCODE_EQUALS]) {
        /* Magnitude min up */
        double next_magnitudeMin;

        if (settings.magnitudeLog)
            next_magnitudeMin = std::min<double>(settings.magnitudeMin + UserLimits.magnitudeLogStep, settings.magnitudeMax - UserLimits.magnitudeLogStep);
        else
            next_magnitudeMin = std::min<double>(settings.magnitudeMin + UserLimits.magnitudeLinearStep, settings.magnitudeMax - UserLimits.magnitudeLinearStep);

        spectrogramThread.setMagnitudeMin(next_magnitudeMin);
        settings.magnitudeMin = spectrogramThread.getMagnitudeMin();
    } else if (state[SDL_SCANCODE_LEFTBRACKET]) {
        /* Magnitude max down */
        double next_magnitudeMax;

        if (settings.magnitudeLog)
            next_magnitudeMax = std::max<double>(settings.magnitudeMax - UserLimits.magnitudeLogStep, settings.magnitudeMin + UserLimits.magnitudeLogStep);
        else
            next_magnitudeMax = std::max<double>(settings.magnitudeMax - UserLimits.magnitudeLinearStep, settings.magnitudeMin + UserLimits.magnitudeLinearStep);

        spectrogramThread.setMagnitudeMax(next_magnitudeMax);
        settings.magnitudeMax = spectrogramThread.getMagnitudeMax();
    } else if (state[SDL_SCANCODE_RIGHTBRACKET]) {
        /* Magnitude max up */
        double next_magnitudeMax;

        if (settings.magnitudeLog)
            next_magnitudeMax = std::min<double>(settings.magnitudeMax + UserLimits.magnitudeLogStep, UserLimits.magnitudeLogMax);
        else
            next_magnitudeMax = std::min<double>(settings.magnitudeMax + UserLimits.magnitudeLinearStep, UserLimits.magnitudeLinearMax);

        spectrogramThread.setMagnitudeMax(next_magnitudeMax);
        settings.magnitudeMax = spectrogramThread.getMagnitudeMax();
    } else if (state[SDL_SCANCODE_H]) {
        /* Hide info */
        hideInfo = !hideInfo;
        if (!hideInfo)
            renderSettings();
        return;
    } else if (state[SDL_SCANCODE_D]) {
        /* Hide statistics */
        hideStatistics = !hideStatistics;
        if (!hideStatistics)
            renderStatistics();
        return;
    } else {
        return;
    }

    renderSettings();
}

void InterfaceThread::run() {
    std::unique_ptr<uint32_t []> pixels = std::unique_ptr<uint32_t []>(new uint32_t[width*height]);
    std::vector<uint32_t> newPixels;

    auto statisticsTic = std::chrono::system_clock::now();

    running = true;

    /* Initialize pixels */
    for (unsigned int i = 0; i < width*height; i++)
        pixels.get()[i] = 0x00;

    /* Poll current settings */
    updateSettings();
    /* Render settings text */
    renderSettings();
    /* Render statistics */
    renderStatistics();

    while (running) {
        /* Handle SDL events */
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                const uint8_t *state = SDL_GetKeyboardState(nullptr);
                handleKeyDown(state);
            } else if (e.type == SDL_MOUSEMOTION) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                renderCursor(mx, my);
            }
        }

        /* Update statistics every 500ms */
        if (!hideStatistics && (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-statisticsTic).count() > 500)) {
            renderStatistics();
            statisticsTic = std::chrono::system_clock::now();
        }

        /* Collect all new pixel rows */
        while (!pixelsQueue.empty()) {
            std::vector<uint32_t> pixelRow(pixelsQueue.pop());
            newPixels.insert(newPixels.end(), pixelRow.begin(), pixelRow.end());
        }

        /* Update pixel buffer with new pixels */
        if (newPixels.size() > 0) {
            uint32_t *data = newPixels.data();
            size_t size = newPixels.size();

            if (size > width*height) {
                /* Pixel buffer overrun (this should seldom happen). */
                /* Use the last width*height pixels */
                data = newPixels.data() + (width*height - size);
                size = width*height;
            }

            if (orientation == Orientation::Vertical) {
                /* Move old pixels up */
                memmove(pixels.get(), pixels.get()+size, (width*height-size)*sizeof(uint32_t));

                /* Copy new pixels over */
                memcpy(pixels.get()+(width*height-size), data, size*sizeof(uint32_t));
            } else {
                unsigned int colsToShift = std::min(static_cast<unsigned int>(size/height), width);

                /* Move old pixels to the left */
                for (unsigned int x = 0; x < (width - colsToShift); x++)
                    for (unsigned int y = 0; y < height; y++)
                        pixels[y*width + x] = pixels[y*width + x + colsToShift];

                /* Copy new pixels over */
                unsigned int i = 0;
                for (unsigned int x = width - colsToShift; x < width; x++)
                    for (unsigned int y = 0; y < height; y++)
                        pixels[(height-y)*width + x] = data[i++];
            }

            /* Clear new pixels */
            newPixels.clear();

            SDL_UpdateTexture(pixelsTexture, nullptr, pixels.get(), static_cast<int>(width * sizeof(uint32_t)));
        }

        SDL_RenderClear(renderer);
        /* Render pixels */
        SDL_RenderCopy(renderer, pixelsTexture, nullptr, nullptr);
        /* Render settings and cursor */
        if (!hideInfo) {
            SDL_RenderCopy(renderer, settingsTexture, nullptr, &settingsRect);
            SDL_RenderCopy(renderer, cursorTexture, nullptr, &cursorRect);
        }
        /* Render statistics */
        if (!hideStatistics) {
            SDL_RenderCopy(renderer, statisticsTexture, nullptr, &statisticsRect);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(5);
    }
}

