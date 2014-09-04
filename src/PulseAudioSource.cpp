#include <stdexcept>
#include <vector>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "PulseAudioSource.hpp"

PulseAudioSource::PulseAudioSource(unsigned int sampleRate) : sampleRate(sampleRate) {
    int error;
    pa_sample_spec ss;

    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = sampleRate;
    ss.channels = 1;

    s = pa_simple_new(NULL, "spectrogram", PA_STREAM_RECORD, NULL, "audio in", &ss, NULL, NULL, &error);
    if (s == NULL)
        throw std::runtime_error("Opening PulseAudio: pa_simple_new(): " + std::string(pa_strerror(error)));
}

PulseAudioSource::~PulseAudioSource() {
    if (s) {
        pa_simple_free(s);
        s = NULL;
    }
}

void PulseAudioSource::read(double *samples, size_t num) {
    std::vector<float> fsamples(num);
    int error;

    if (pa_simple_read(s, fsamples.data(), num*sizeof(float), &error) < 0)
        throw std::runtime_error("Reading PulseAudio: pa_simple_read(): " + std::string(pa_strerror(error)));

    for (unsigned int i = 0; i < num; i++)
        samples[i] = static_cast<double>(fsamples[i]);
}

unsigned int PulseAudioSource::getSampleRate() {
    return sampleRate;
}

