#include <stdexcept>
#include <vector>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "PulseAudioSource.hpp"

PulseAudioSource::PulseAudioSource(unsigned int sampleRate) : sampleRate(sampleRate) {
    int error;
    pa_sample_spec ss;
    pa_buffer_attr attr;

    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = sampleRate;
    ss.channels = 1;

    attr.maxlength = -1;
    attr.tlength = -1;
    attr.prebuf = -1;
    attr.minreq = -1;
    attr.fragsize = 1024;

    s = pa_simple_new(nullptr, "spectrogram", PA_STREAM_RECORD, nullptr, "audio in", &ss, nullptr, &attr, &error);
    if (s == nullptr)
        throw std::runtime_error("Opening PulseAudio: pa_simple_new(): " + std::string(pa_strerror(error)));
}

PulseAudioSource::~PulseAudioSource() {
    if (s)
        pa_simple_free(s);
}

void PulseAudioSource::read(std::vector<double> &samples) {
    unsigned int count = samples.size();
    std::vector<float> fsamples(count);
    int error;

    if (pa_simple_read(s, fsamples.data(), count*sizeof(float), &error) < 0)
        throw std::runtime_error("Reading PulseAudio: pa_simple_read(): " + std::string(pa_strerror(error)));

    for (unsigned int i = 0; i < count; i++)
        samples[i] = static_cast<double>(fsamples[i]);
}

unsigned int PulseAudioSource::getSampleRate() {
    return sampleRate;
}

