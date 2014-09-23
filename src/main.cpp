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

    AudioThread audioThread(audioResource, samplesQueue, InitialSettings.audioReadSize);
    SpectrogramThread spectrogramThread(samplesQueue, pixelsQueue, dftResource, spectrogramResource, InitialSettings.audioSampleRate, (InitialSettings.orientation == Orientation::Vertical) ? InitialSettings.width : InitialSettings.height);
    InterfaceThread interfaceThread(pixelsQueue, audioResource, dftResource, spectrogramResource, audioThread, spectrogramThread, InitialSettings.width, InitialSettings.height, InitialSettings.orientation);

    std::thread t1(&AudioThread::run, &audioThread);
    std::thread t2(&SpectrogramThread::run, &spectrogramThread);
    interfaceThread.run();

    t1.join();
    t2.join();
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

