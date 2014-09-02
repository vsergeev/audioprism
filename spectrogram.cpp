#include <iostream>
#include <thread>
#include <memory>
#include <exception>
#include <vector>
#include <mutex>
#include <cmath>

#include "SDL.h"

#include <pulse/simple.h>
#include <pulse/error.h>

#include <fftw3.h>

#define DFT_SAMPLES     1024*2
#define READ_SAMPLES    512
#define WIDTH   640
#define HEIGHT  480

class AudioSource {
  public:
    AudioSource() : s(NULL) {
        open();
    }
    ~AudioSource() {
        close();
    }

    void open() {
        pa_sample_spec ss;
        int error;

        ss.format = PA_SAMPLE_S16LE;
        ss.rate = 48000;
        ss.channels = 1;

        s = pa_simple_new(NULL, "spectrogram", PA_STREAM_RECORD, NULL, "audio in", &ss, NULL, NULL, &error);
        if (s == NULL)
            throw std::runtime_error("pa_simple_new(): " + std::string(pa_strerror(error)));
    }

    void close() {
        if (s) {
            pa_simple_free(s);
            s = NULL;
        }
    }

    std::vector<uint16_t> read(size_t num_samples) {
        std::vector<uint16_t> samples(num_samples);
        int error;

        if (pa_simple_read(s, samples.data(), num_samples*2, &error) < 0)
            throw std::runtime_error("pa_simple_read(): " + std::string(pa_strerror(error)));

        return samples;
    }

    void read(std::vector<uint16_t> &samples) {
        int error;

        if (pa_simple_read(s, samples.data(), samples.size()*2, &error) < 0)
            throw std::runtime_error("pa_simple_read(): " + std::string(pa_strerror(error)));
    }

    void read(std::vector<double> &samples) {
        std::vector<uint16_t> usamples(samples.size());
        int error;

        if (pa_simple_read(s, usamples.data(), usamples.size()*2, &error) < 0)
            throw std::runtime_error("pa_simple_read(): " + std::string(pa_strerror(error)));

        for (unsigned int i = 0; i < samples.size(); i++)
            samples[i] = static_cast<double>(usamples[i])/INT16_MAX;
    }

    void read(double *samples, size_t num_samples) {
        std::vector<uint16_t> usamples(num_samples);
        int error;

        if (pa_simple_read(s, usamples.data(), usamples.size()*2, &error) < 0)
            throw std::runtime_error("pa_simple_read(): " + std::string(pa_strerror(error)));

        for (unsigned int i = 0; i < num_samples; i++)
            samples[i] = static_cast<double>(usamples[i])/INT16_MAX;
    }

 private:
    pa_simple *s;
};

template <unsigned int N>
std::array<double, N> window_hanning(void) {
    std::array<double, N> window;

    for (unsigned int n = 0; n < N; n++)
        window[n] = 0.5*(1-std::cos((2*M_PI*n)/(N-1)));

    return window;
}

std::mutex pixels_lock;
uint32_t pixels[HEIGHT][WIDTH];

uint32_t magnitude_to_pixel(double magnitude) {
    if (magnitude > UINT16_MAX)
        magnitude = UINT16_MAX;

    return static_cast<uint16_t>(magnitude);
}

void dft_magnitude_to_pixels(std::vector<double> &dft_magnitude) {
    pixels_lock.lock();
    /* Move pixels up one */
    for (unsigned int i = 1; i < HEIGHT; i++)
        memcpy(pixels[i-1], pixels[i], WIDTH*sizeof(pixels[0][0]));
    /* Replace pixels with green */
    for (unsigned int i = 0; i < WIDTH; i++)
        pixels[HEIGHT-1][i] = magnitude_to_pixel(dft_magnitude[i+1]);
    pixels_lock.unlock();
}

void thread_audio() {
    AudioSource audio;
    std::array<double, DFT_SAMPLES> samples;
    std::array<double, DFT_SAMPLES> wsamples;
    std::array<double, DFT_SAMPLES> window = window_hanning<DFT_SAMPLES>();

    fftw_complex *dft = fftw_alloc_complex(DFT_SAMPLES/2+1);
    std::vector<double> dft_magnitude(DFT_SAMPLES/2+1);

    fftw_plan plan;

    plan = fftw_plan_dft_r2c_1d(DFT_SAMPLES, wsamples.data(), dft, FFTW_MEASURE);

    while (true) {
        /* Move old samples */
        memmove(samples.data(), samples.data()+READ_SAMPLES, sizeof(double)*(DFT_SAMPLES-READ_SAMPLES));

        /* Read new samples */
        audio.read(samples.data()+(DFT_SAMPLES-READ_SAMPLES), READ_SAMPLES);

        auto tic = std::chrono::system_clock::now();

        /* Mulitply by window */
        for (unsigned int i = 0; i < DFT_SAMPLES; i++)
            wsamples[i] = samples[i] * window[i];

        /* Execute DFT */
        fftw_execute(plan);

        /* Compute magnitude */
        for (unsigned int i = 0; i < DFT_SAMPLES/2+1; i++)
            dft_magnitude[i] = dft[i][0]*dft[i][0] + dft[i][1]*dft[i][1];

        auto toc = std::chrono::system_clock::now();

        dft_magnitude_to_pixels(dft_magnitude);

        //std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(toc-tic).count() << " ns\n";
    }

    fftw_destroy_plan(plan);
    fftw_cleanup();
}

int main(int argc, char *argv[]) {
    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("Spectogram", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
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

