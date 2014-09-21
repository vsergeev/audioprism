#include <thread>
#include <iostream>

#include "ThreadSafeQueue.hpp"
#include "PulseAudioSource.hpp"
#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"
#include "InterfaceThread.hpp"

#include "WaveAudioSource.hpp"
#include "MagickImageSink.hpp"

#define SAMPLE_RATE     24000
#define WIDTH           640
#define HEIGHT          480
#define READ_SIZE       1024
#define DFT_SIZE        2048
#define WINDOW_FUNC     WindowFunction::Hanning
#define COLORS          Spectrogram::ColorScheme::Heat
#define MAGNITUDE_MIN   0.0
#define MAGNITUDE_MAX   60.0
#define MAGNITUDE_LOG   true

void spectrogram_realtime() {
    ThreadSafeQueue<std::vector<double>> samplesQueue;
    ThreadSafeQueue<std::vector<uint32_t>> pixelsQueue;

    PulseAudioSource audioSource(SAMPLE_RATE);
    AudioThread audioThread(audioSource, samplesQueue, READ_SIZE);
    SpectrogramThread spectrogramThread(samplesQueue, pixelsQueue, SAMPLE_RATE, WIDTH, DFT_SIZE, WINDOW_FUNC, MAGNITUDE_MIN, MAGNITUDE_MAX, MAGNITUDE_LOG, COLORS);
    InterfaceThread interfaceThread(pixelsQueue, audioThread, spectrogramThread, WIDTH, HEIGHT);

    std::thread t1(&AudioThread::run, &audioThread);
    std::thread t2(&SpectrogramThread::run, &spectrogramThread);

    interfaceThread.run();
    t1.join();
    t2.join();
}

void spectrogram_audiofile(std::string audioPath, std::string imagePath) {
    WaveAudioSource audio(audioPath);
    RealDft dft(DFT_SIZE, WINDOW_FUNC);
    Spectrogram spectrogram(MAGNITUDE_MIN, MAGNITUDE_MAX, MAGNITUDE_LOG, COLORS);
    MagickImageSink image(imagePath, WIDTH);

    std::vector<double> samples(dft.getSize());
    std::vector<std::complex<double>> dftSamples(dft.getSize());
    std::vector<uint32_t> pixels(WIDTH);
    std::vector<double> newSamples(READ_SIZE);

    while (true) {
        audio.read(newSamples);

        if (newSamples.size() == 0)
            break;

        /* Move down old samples */
        memmove(samples.data(), samples.data()+newSamples.size(), sizeof(double)*(samples.size()-newSamples.size()));
        /* Copy new samples */
        memcpy(samples.data()+(samples.size()-newSamples.size()), newSamples.data(), sizeof(double)*newSamples.size());

        dft.compute(dftSamples, samples);

        spectrogram.render(pixels, dftSamples);

        image.append(pixels);
    }

    image.write();
}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        spectrogram_audiofile(std::string(argv[1]), std::string(argv[2]));
    } else {
        spectrogram_realtime();
    }

    return 0;
}

