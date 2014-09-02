#include <thread>

#include "PulseAudioSource.hpp"
#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"
#include "InterfaceThread.hpp"

#define SAMPLE_RATE 48000
#define WIDTH       640
#define HEIGHT      480

int main(int argc, char *argv[]) {
    PulseAudioSource audioSource(SAMPLE_RATE);
    AudioThread audioThread(audioSource);
    SpectrogramThread spectrogramThread(audioThread, WIDTH, HEIGHT);
    InterfaceThread interfaceThread(audioThread, spectrogramThread, WIDTH, HEIGHT);

    std::thread t1(&AudioThread::run, &audioThread);
    std::thread t2(&SpectrogramThread::run, &spectrogramThread);

    interfaceThread.run();

    //t1.join();
    //t2.join();

    return 0;
}

