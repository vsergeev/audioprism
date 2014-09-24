#include <thread>
#include <iostream>

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "Orientation.hpp"
#include "PulseAudioSource.hpp"
#include "RealDft.hpp"
#include "Spectrogram.hpp"
#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"
#include "InterfaceThread.hpp"

#include "WaveAudioSource.hpp"
#include "MagickImageSink.hpp"

#include "Configuration.hpp"

Settings InitialSettings;
Limits UserLimits;

void spectrogram_realtime() {
    PulseAudioSource audio(InitialSettings.audioSampleRate);
    RealDft dft(InitialSettings.dftSize, InitialSettings.dftWf);
    Spectrogram spectrogram(InitialSettings.magnitudeMin, InitialSettings.magnitudeMax, InitialSettings.magnitudeLog, InitialSettings.colors);

    ThreadSafeResource<AudioSource> audioResource(audio);
    ThreadSafeResource<RealDft> dftResource(dft);
    ThreadSafeResource<Spectrogram> spectrogramResource(spectrogram);
    ThreadSafeQueue<std::vector<double>> samplesQueue;
    ThreadSafeQueue<std::vector<uint32_t>> pixelsQueue;
    std::atomic<size_t> audioReadSize;
    std::atomic<bool> running;

    audioReadSize = InitialSettings.audioReadSize;
    running = true;

    InterfaceThread interfaceThread(pixelsQueue, audioResource, dftResource, spectrogramResource, audioReadSize, running, InitialSettings.width, InitialSettings.height, InitialSettings.orientation);

    std::thread audioThread(AudioThread, std::ref(audioResource), std::ref(samplesQueue), std::ref(audioReadSize), std::ref(running));
    std::thread spectrogramThread(SpectrogramThread, std::ref(samplesQueue), std::ref(pixelsQueue), std::ref(dftResource), std::ref(spectrogramResource), (InitialSettings.orientation == Orientation::Vertical) ? InitialSettings.width : InitialSettings.height, std::ref(running));
    interfaceThread.run();

    audioThread.join();
    spectrogramThread.join();
}

void spectrogram_audiofile(std::string audioPath, std::string imagePath) {
    WaveAudioSource audio(audioPath);
    RealDft dft(InitialSettings.dftSize, InitialSettings.dftWf);
    Spectrogram spectrogram(InitialSettings.magnitudeMin, InitialSettings.magnitudeMax, InitialSettings.magnitudeLog, InitialSettings.colors);
    MagickImageSink image(imagePath, InitialSettings.width, InitialSettings.orientation);

    std::vector<double> newSamples(InitialSettings.audioReadSize);
    std::vector<double> samples(InitialSettings.dftSize);
    std::vector<std::complex<double>> dftSamples(InitialSettings.dftSize);
    std::vector<uint32_t> pixels(InitialSettings.width);

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

