#include <stdexcept>
#include <vector>

#include "pulseaudiosource.hpp"

#include <pulse/simple.h>
#include <pulse/error.h>

PulseAudioSource::PulseAudioSource() {
    int error;
    pa_sample_spec ss;
    
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = 48000;
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

void PulseAudioSource::read(float *samples, size_t num) {
    std::vector<int16_t> isamples(num);
    int error;

    if (pa_simple_read(s, isamples.data(), num*sizeof(int16_t), &error) < 0)
        throw std::runtime_error("Reading PulseAudio: pa_simple_read(): " + std::string(pa_strerror(error)));

    for (unsigned int i = 0; i < num; i++)
            samples[i] = static_cast<float>(isamples[i])/INT16_MAX;
}

