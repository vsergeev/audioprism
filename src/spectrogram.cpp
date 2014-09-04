#include <thread>

#include "ThreadSafeQueue.hpp"
#include "PulseAudioSource.hpp"
#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"
#include "InterfaceThread.hpp"

#define SAMPLE_RATE     48000
#define WIDTH           640
#define HEIGHT          480
#define READ_SIZE       128
#define DFT_SIZE        2048
#define WINDOW_FUNC     WindowFunction::Hanning
#define MAGNITUDE_MIN   0.0
#define MAGNITUDE_MAX   60.0

int main(int argc, char *argv[]) {
    ThreadSafeQueue<std::vector<double>> samplesQueue;
    ThreadSafeQueue<std::vector<uint32_t>> pixelsQueue;

    PulseAudioSource audioSource(SAMPLE_RATE);
    AudioThread audioThread(audioSource, samplesQueue, READ_SIZE);
    SpectrogramThread spectrogramThread(samplesQueue, pixelsQueue, SAMPLE_RATE, WIDTH, DFT_SIZE, WINDOW_FUNC, MAGNITUDE_MIN, MAGNITUDE_MAX);
    InterfaceThread interfaceThread(pixelsQueue, audioThread, spectrogramThread, WIDTH, HEIGHT);

    std::thread t1(&AudioThread::run, &audioThread);
    std::thread t2(&SpectrogramThread::run, &spectrogramThread);

    interfaceThread.run();

    //t1.join();
    //t2.join();

    return 0;
}

