#include <stdexcept>
#include <vector>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "PulseAudioSource.hpp"

namespace Audio {

PulseAudioSource::PulseAudioSource(unsigned int sampleRate) : _sampleRate(sampleRate) {
    int error;
    pa_sample_spec ss;
    pa_buffer_attr attr;

    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = sampleRate;
    ss.channels = 1;

    attr.maxlength = -1u;
    attr.tlength = -1u;
    attr.prebuf = -1u;
    attr.minreq = -1u;
    attr.fragsize = 1024;

    _handle = pa_simple_new(nullptr, "spectrogram", PA_STREAM_RECORD, nullptr, "audio in", &ss, nullptr, &attr, &error);
    if (_handle == nullptr)
        throw OpenException("Opening PulseAudio: pa_simple_new(): " + std::string(pa_strerror(error)));
}

PulseAudioSource::~PulseAudioSource() {
    if (_handle)
        pa_simple_free(_handle);
}

void PulseAudioSource::read(std::vector<double> &samples) {
    size_t count = samples.size();
    std::vector<float> fsamples(count);
    int error;

    if (pa_simple_read(_handle, fsamples.data(), count * sizeof(float), &error) < 0)
        throw ReadException("Reading PulseAudio: pa_simple_read(): " + std::string(pa_strerror(error)));

    for (unsigned int i = 0; i < count; i++)
        samples[i] = static_cast<double>(fsamples[i]);
}

unsigned int PulseAudioSource::getSampleRate() {
    return _sampleRate;
}

}
