#include <ftw.h>
#include <fnmatch.h>
#include <map>

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
            std::string filename = path.substr(slashpos + 1);

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

InterfaceThread::InterfaceThread(ThreadSafeQueue<std::vector<uint32_t>> &pixelsQueue, AudioThread &audioThread, SpectrogramThread &spectrogramThread, const Settings &initialSettings) : _pixelsQueue(pixelsQueue), _audioThread(audioThread), _spectrogramThread(spectrogramThread), _width(initialSettings.width), _height(initialSettings.height), _orientation(initialSettings.orientation), _hideInfo(false), _hideStatistics(true) {
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
    _win = SDL_CreateWindow("audioprism", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<int>(_width), static_cast<int>(_height), SDL_WINDOW_OPENGL);
    if (_win == nullptr)
        throw SDLException("Creating SDL window: SDL_CreateWindow(): " + std::string(SDL_GetError()));

    /* Create Renderer */
    _renderer = SDL_CreateRenderer(_win, -1, SDL_RENDERER_ACCELERATED);
    if (_renderer == nullptr)
        throw SDLException("Creating SDL renderer: SDL_CreateRenderer(): " + std::string(SDL_GetError()));

    /* Create main texture */
    _pixelsTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, static_cast<int>(_width), static_cast<int>(_height));
    if (_pixelsTexture == nullptr)
        throw SDLException("Creating SDL texture: SDL_CreateTexture(): " + std::string(SDL_GetError()));

    _statisticsTexture = nullptr;
    _settingsTexture = nullptr;
    _cursorTexture = nullptr;

    /* Find a compatible font */
    std::string fontPath = findFontPath();
    if (fontPath == "")
        throw TTFException("Could not find a compatible TTF font.");

    /* Open font */
    _font = TTF_OpenFont(fontPath.c_str(), 11);
    if (_font == nullptr)
        throw TTFException("Opening TTF font: TTF_OpenFont(): " + std::string(TTF_GetError()));
}

InterfaceThread::~InterfaceThread() {
    TTF_CloseFont(_font);
    if (_pixelsTexture)
        SDL_DestroyTexture(_pixelsTexture);
    if (_settingsTexture)
        SDL_DestroyTexture(_settingsTexture);
    if (_cursorTexture)
        SDL_DestroyTexture(_cursorTexture);
    if (_statisticsTexture)
        SDL_DestroyTexture(_statisticsTexture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_win);
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

enum class Alignment { Left,
                       Center,
                       Right };

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
            targetRect.x = (targetSurfaceWidth - surface->w) / 2;
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

void InterfaceThread::_updateSettings() {
    _settings.audioSampleRate = _audioThread.getSampleRate();
    _settings.samplesOverlap = _spectrogramThread.getSamplesOverlap();
    _settings.dftSize = _spectrogramThread.getDftSize();
    _settings.dftWindowFunction = _spectrogramThread.getDftWindowFunction();
    _settings.magnitudeMin = _spectrogramThread.getMagnitudeMin();
    _settings.magnitudeMax = _spectrogramThread.getMagnitudeMax();
    _settings.magnitudeLog = _spectrogramThread.getMagnitudeLog();
    _settings.colorScheme = _spectrogramThread.getColorScheme();
}

void InterfaceThread::_renderSettings() {
    std::vector<SDL_Surface *> textSurfaces;
    SDL_Surface *settingsSurface;
    SDL_Color settingsColor = {0xff, 0x00, 0x00, 0x00};

    unsigned int overlap = static_cast<unsigned int>(_settings.samplesOverlap * 100.0);

    textSurfaces.push_back(renderString(format("Sample Rate: %d Hz", _settings.audioSampleRate), _font, settingsColor));
    textSurfaces.push_back(renderString(format("Overlap: %d%%", overlap), _font, settingsColor));
    textSurfaces.push_back(renderString("Window: " + to_string(_settings.dftWindowFunction), _font, settingsColor));
    textSurfaces.push_back(renderString(format("DFT Size: %d", _settings.dftSize), _font, settingsColor));
    textSurfaces.push_back(renderString(format("Colors: %s", to_string(_settings.colorScheme).c_str()), _font, settingsColor));
    if (_settings.magnitudeLog) {
        textSurfaces.push_back(renderString(format("Mag. min: %.2f dB", _settings.magnitudeMin), _font, settingsColor));
        textSurfaces.push_back(renderString(format("Mag. max: %.2f dB", _settings.magnitudeMax), _font, settingsColor));
        textSurfaces.push_back(renderString(format("Mag. Logarithmic"), _font, settingsColor));
    } else {
        textSurfaces.push_back(renderString(format("Mag. min: %.2f", _settings.magnitudeMin), _font, settingsColor));
        textSurfaces.push_back(renderString(format("Mag. max: %.2f", _settings.magnitudeMax), _font, settingsColor));
        textSurfaces.push_back(renderString(format("Mag. Linear"), _font, settingsColor));
    }

    settingsSurface = vcatSurfaces(textSurfaces, Alignment::Right);

    /* Update settings rectangle destination for screen rendering */
    _settingsRect.x = static_cast<int>(_width) - settingsSurface->w - 5;
    _settingsRect.y = 2;
    _settingsRect.w = settingsSurface->w;
    _settingsRect.h = settingsSurface->h;

    /* Destroy old settings texture */
    if (_settingsTexture)
        SDL_DestroyTexture(_settingsTexture);

    /* Create new texture from the target surface */
    _settingsTexture = SDL_CreateTextureFromSurface(_renderer, settingsSurface);
    if (_settingsTexture == nullptr)
        throw SDLException("Error creating texture for text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(settingsSurface);
}

void InterfaceThread::_renderCursor(int x, int y) {
    SDL_Surface *cursorSurface;
    SDL_Color settingsColor = {0xff, 0x00, 0x00, 0x00};

    float frequency;

    float hzPerBin = ((static_cast<float>(_settings.audioSampleRate)) / 2.0f) / static_cast<float>((_settings.dftSize / 2 + 1));

    if (_orientation == Orientation::Vertical) {
        float binPerPixel = static_cast<float>((_settings.dftSize / 2 + 1)) / static_cast<float>(_width);
        frequency = std::floor(static_cast<float>(x) * binPerPixel) * hzPerBin;
    } else {
        float binPerPixel = static_cast<float>((_settings.dftSize / 2 + 1)) / static_cast<float>(_height);
        frequency = std::floor(static_cast<float>(static_cast<int>(_height) - y) * binPerPixel) * hzPerBin;
    }

    cursorSurface = renderString(format("%.0f Hz", frequency), _font, settingsColor);

    /* Update cursor rectangle destination for screen rendering */
    _cursorRect.x = static_cast<int>(_width) - cursorSurface->w - 5;
    _cursorRect.y = _settingsRect.y + _settingsRect.h + cursorSurface->h;
    _cursorRect.w = cursorSurface->w;
    _cursorRect.h = cursorSurface->h;

    /* Destroy old settings texture */
    if (_cursorTexture)
        SDL_DestroyTexture(_cursorTexture);

    /* Create new texture from the target surface */
    _cursorTexture = SDL_CreateTextureFromSurface(_renderer, cursorSurface);
    if (_cursorTexture == nullptr)
        throw SDLException("Error creating texture for cursor text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(cursorSurface);
}

void InterfaceThread::_renderStatistics() {
    std::vector<SDL_Surface *> textSurfaces;
    SDL_Surface *statisticsSurface;
    SDL_Color statisticsColor = {0xff, 0x00, 0x00, 0x00};

    size_t samplesQueueCount = _spectrogramThread.getDebugSamplesQueueCount();
    size_t pixelsQueueCount = _pixelsQueue.count();

    textSurfaces.push_back(renderString(format("Audio Queue: %u", samplesQueueCount), _font, statisticsColor));
    textSurfaces.push_back(renderString(format("Pixels Queue: %u", pixelsQueueCount), _font, statisticsColor));
    statisticsSurface = vcatSurfaces(textSurfaces, Alignment::Right);

    /* Update statistics rectangle destination for screen rendering */
    _statisticsRect.x = static_cast<int>(_width) - statisticsSurface->w - 5;
    _statisticsRect.y = _cursorRect.y + _cursorRect.h * 2;
    _statisticsRect.w = statisticsSurface->w;
    _statisticsRect.h = statisticsSurface->h;

    /* Destroy old settings texture */
    if (_statisticsTexture)
        SDL_DestroyTexture(_statisticsTexture);

    /* Create new texture from the target surface */
    _statisticsTexture = SDL_CreateTextureFromSurface(_renderer, statisticsSurface);
    if (_statisticsTexture == nullptr)
        throw SDLException("Error creating texture for statistics text: SDL_CreateTextureFromSurface(): " + std::string(SDL_GetError()));

    SDL_FreeSurface(statisticsSurface);
}

void InterfaceThread::_handleKeyDown(const uint8_t *state) {
    if (state[SDL_SCANCODE_Q]) {
        _running = false;
    } else if (state[SDL_SCANCODE_C]) {
        /* Change color scheme */
        SpectrumRenderer::ColorScheme next_colorScheme = SpectrumRenderer::ColorScheme::Heat;

        if (_settings.colorScheme == SpectrumRenderer::ColorScheme::Heat)
            next_colorScheme = SpectrumRenderer::ColorScheme::Blue;
        else if (_settings.colorScheme == SpectrumRenderer::ColorScheme::Blue)
            next_colorScheme = SpectrumRenderer::ColorScheme::Grayscale;
        else if (_settings.colorScheme == SpectrumRenderer::ColorScheme::Grayscale)
            next_colorScheme = SpectrumRenderer::ColorScheme::Heat;

        _spectrogramThread.setColorScheme(next_colorScheme);
        _settings.colorScheme = _spectrogramThread.getColorScheme();
    } else if (state[SDL_SCANCODE_W]) {
        /* Change window function */
        RealDft::WindowFunction next_wf = RealDft::WindowFunction::Hann;

        if (_settings.dftWindowFunction == RealDft::WindowFunction::Hann)
            next_wf = RealDft::WindowFunction::Hamming;
        else if (_settings.dftWindowFunction == RealDft::WindowFunction::Hamming)
            next_wf = RealDft::WindowFunction::Bartlett;
        else if (_settings.dftWindowFunction == RealDft::WindowFunction::Bartlett)
            next_wf = RealDft::WindowFunction::Rectangular;
        else if (_settings.dftWindowFunction == RealDft::WindowFunction::Rectangular)
            next_wf = RealDft::WindowFunction::Hann;

        _spectrogramThread.setDftWindowFunction(next_wf);
        _settings.dftWindowFunction = _spectrogramThread.getDftWindowFunction();
    } else if (state[SDL_SCANCODE_L]) {
        /* Toggle between Logarithimic/Linear */
        bool next_magnitudeLog = !_settings.magnitudeLog;

        _spectrogramThread.setMagnitudeLog(next_magnitudeLog);
        _settings.magnitudeLog = next_magnitudeLog;
        if (next_magnitudeLog) {
            _spectrogramThread.setMagnitudeMin(InitialSettings.magnitudeLogMin);
            _spectrogramThread.setMagnitudeMax(InitialSettings.magnitudeLogMax);
        } else {
            _spectrogramThread.setMagnitudeMin(InitialSettings.magnitudeLinearMin);
            _spectrogramThread.setMagnitudeMax(InitialSettings.magnitudeLinearMax);
        }
        _settings.magnitudeMin = _spectrogramThread.getMagnitudeMin();
        _settings.magnitudeMax = _spectrogramThread.getMagnitudeMax();
    } else if (state[SDL_SCANCODE_RIGHT]) {
        /* DFT N up */
        unsigned int next_dftSize = std::min<unsigned int>(_settings.dftSize * 2, UserLimits.dftSizeMax);

        if (next_dftSize == _settings.dftSize)
            return;

        _spectrogramThread.setDftSize(next_dftSize);

        /* Reset samples overlap to 50% */
        _spectrogramThread.setSamplesOverlap(0.50);

        _settings.dftSize = _spectrogramThread.getDftSize();
        _settings.samplesOverlap = _spectrogramThread.getSamplesOverlap();
    } else if (state[SDL_SCANCODE_LEFT]) {
        /* DFT N down */
        unsigned int next_dftSize = std::max<unsigned int>(_settings.dftSize / 2, UserLimits.dftSizeMin);

        if (next_dftSize == _settings.dftSize)
            return;

        _spectrogramThread.setDftSize(next_dftSize);

        /* Reset samples overlap to 50% */
        _spectrogramThread.setSamplesOverlap(0.50);

        _settings.dftSize = _spectrogramThread.getDftSize();
        _settings.samplesOverlap = _spectrogramThread.getSamplesOverlap();
    } else if (state[SDL_SCANCODE_DOWN]) {
        /* Samples Overlap Up */
        float next_samplesOverlap = std::max<float>(_settings.samplesOverlap - UserLimits.samplesOverlapStep, UserLimits.samplesOverlapMin);

        _spectrogramThread.setSamplesOverlap(next_samplesOverlap);
        _settings.samplesOverlap = _spectrogramThread.getSamplesOverlap();
    } else if (state[SDL_SCANCODE_UP]) {
        /* Samples Overlap Down */
        float next_samplesOverlap = std::min<float>(_settings.samplesOverlap + UserLimits.samplesOverlapStep, UserLimits.samplesOverlapMax);

        _spectrogramThread.setSamplesOverlap(next_samplesOverlap);
        _settings.samplesOverlap = _spectrogramThread.getSamplesOverlap();
    } else if (state[SDL_SCANCODE_MINUS]) {
        /* Magnitude min down */
        float next_magnitudeMin;

        if (_settings.magnitudeLog)
            next_magnitudeMin = std::max<float>(_settings.magnitudeMin - UserLimits.magnitudeLogStep, UserLimits.magnitudeLogMin);
        else
            next_magnitudeMin = std::max<float>(_settings.magnitudeMin - UserLimits.magnitudeLinearStep, UserLimits.magnitudeLinearMin);

        _spectrogramThread.setMagnitudeMin(next_magnitudeMin);
        _settings.magnitudeMin = _spectrogramThread.getMagnitudeMin();
    } else if (state[SDL_SCANCODE_EQUALS]) {
        /* Magnitude min up */
        float next_magnitudeMin;

        if (_settings.magnitudeLog)
            next_magnitudeMin = std::min<float>(_settings.magnitudeMin + UserLimits.magnitudeLogStep, _settings.magnitudeMax - UserLimits.magnitudeLogStep);
        else
            next_magnitudeMin = std::min<float>(_settings.magnitudeMin + UserLimits.magnitudeLinearStep, _settings.magnitudeMax - UserLimits.magnitudeLinearStep);

        _spectrogramThread.setMagnitudeMin(next_magnitudeMin);
        _settings.magnitudeMin = _spectrogramThread.getMagnitudeMin();
    } else if (state[SDL_SCANCODE_LEFTBRACKET]) {
        /* Magnitude max down */
        float next_magnitudeMax;

        if (_settings.magnitudeLog)
            next_magnitudeMax = std::max<float>(_settings.magnitudeMax - UserLimits.magnitudeLogStep, _settings.magnitudeMin + UserLimits.magnitudeLogStep);
        else
            next_magnitudeMax = std::max<float>(_settings.magnitudeMax - UserLimits.magnitudeLinearStep, _settings.magnitudeMin + UserLimits.magnitudeLinearStep);

        _spectrogramThread.setMagnitudeMax(next_magnitudeMax);
        _settings.magnitudeMax = _spectrogramThread.getMagnitudeMax();
    } else if (state[SDL_SCANCODE_RIGHTBRACKET]) {
        /* Magnitude max up */
        float next_magnitudeMax;

        if (_settings.magnitudeLog)
            next_magnitudeMax = std::min<float>(_settings.magnitudeMax + UserLimits.magnitudeLogStep, UserLimits.magnitudeLogMax);
        else
            next_magnitudeMax = std::min<float>(_settings.magnitudeMax + UserLimits.magnitudeLinearStep, UserLimits.magnitudeLinearMax);

        _spectrogramThread.setMagnitudeMax(next_magnitudeMax);
        _settings.magnitudeMax = _spectrogramThread.getMagnitudeMax();
    } else if (state[SDL_SCANCODE_H]) {
        /* Hide info */
        _hideInfo = !_hideInfo;
        if (!_hideInfo)
            _renderSettings();
        return;
    } else if (state[SDL_SCANCODE_D]) {
        /* Hide statistics */
        _hideStatistics = !_hideStatistics;
        if (!_hideStatistics)
            _renderStatistics();
        return;
    } else {
        return;
    }

    _renderSettings();
}

void InterfaceThread::run() {
    std::unique_ptr<uint32_t[]> pixels = std::unique_ptr<uint32_t[]>(new uint32_t[_width * _height]);
    std::vector<uint32_t> newPixels;

    auto statisticsTic = std::chrono::system_clock::now();

    _running = true;

    /* Initialize pixels */
    memset(pixels.get(), 0x00, _width * _height * sizeof(uint32_t));

    /* Poll current settings */
    _updateSettings();
    /* Render settings text */
    _renderSettings();
    /* Render statistics */
    _renderStatistics();

    while (_running) {
        /* Handle SDL events */
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                _running = false;
            } else if (e.type == SDL_KEYDOWN) {
                const uint8_t *state = SDL_GetKeyboardState(nullptr);
                _handleKeyDown(state);
            } else if (e.type == SDL_MOUSEMOTION) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                _renderCursor(mx, my);
            }
        }

        /* Update statistics every 500ms */
        if (!_hideStatistics && (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - statisticsTic).count() > 500)) {
            _renderStatistics();
            statisticsTic = std::chrono::system_clock::now();
        }

        /* Collect all new pixel rows */
        while (!_pixelsQueue.empty()) {
            std::vector<uint32_t> pixelRow(_pixelsQueue.pop());
            newPixels.insert(newPixels.end(), pixelRow.begin(), pixelRow.end());
        }

        /* Update pixel buffer with new pixels */
        if (newPixels.size() > 0) {
            uint32_t *data = newPixels.data();
            size_t size = newPixels.size();

            if (size > _width * _height) {
                /* Pixel buffer overrun (this should seldom happen). */
                /* Use the last width*height pixels */
                data = newPixels.data() + (_width * _height - size);
                size = _width * _height;
            }

            if (_orientation == Orientation::Vertical) {
                /* Move old pixels up */
                memmove(pixels.get(), pixels.get() + size, (_width * _height - size) * sizeof(uint32_t));

                /* Copy new pixels over */
                memcpy(pixels.get() + (_width * _height - size), data, size * sizeof(uint32_t));
            } else {
                unsigned int colsToShift = std::min(static_cast<unsigned int>(size / _height), _width);

                /* Move old pixels to the left */
                for (unsigned int x = 0; x < (_width - colsToShift); x++)
                    for (unsigned int y = 0; y < _height; y++)
                        pixels[y * _width + x] = pixels[y * _width + x + colsToShift];

                /* Copy new pixels over */
                unsigned int i = 0;
                for (unsigned int x = _width - colsToShift; x < _width; x++)
                    for (unsigned int y = 0; y < _height; y++)
                        pixels[(_height - y) * _width + x] = data[i++];
            }

            /* Clear new pixels */
            newPixels.clear();

            SDL_UpdateTexture(_pixelsTexture, nullptr, pixels.get(), static_cast<int>(_width * sizeof(uint32_t)));
        }

        SDL_RenderClear(_renderer);
        /* Render pixels */
        SDL_RenderCopy(_renderer, _pixelsTexture, nullptr, nullptr);
        /* Render settings and cursor */
        if (!_hideInfo) {
            SDL_RenderCopy(_renderer, _settingsTexture, nullptr, &_settingsRect);
            SDL_RenderCopy(_renderer, _cursorTexture, nullptr, &_cursorRect);
        }
        /* Render statistics */
        if (!_hideStatistics) {
            SDL_RenderCopy(_renderer, _statisticsTexture, nullptr, &_statisticsRect);
        }
        SDL_RenderPresent(_renderer);
        SDL_Delay(5);
    }
}
