#include <sndfile.h>

#include "WaveAudioSource.hpp"

namespace Audio {

WaveAudioSource::WaveAudioSource(std::string path) : _sfinfo() {
    if ((_sndfile = sf_open(path.c_str(), SFM_READ, &_sfinfo)) == nullptr)
        throw OpenException("Error opening WAV file: " + std::string(sf_strerror(nullptr)));
}

WaveAudioSource::~WaveAudioSource() {
    if (_sndfile)
        sf_close(_sndfile);
}

void WaveAudioSource::_read_multi_channel(std::vector<float> &samples) {
    sf_count_t ret;

    _buf.resize(samples.size() * static_cast<unsigned int>(_sfinfo.channels));

    ret = sf_readf_float(_sndfile, _buf.data(), static_cast<sf_count_t>(samples.size()));

    /* Resize samples buffer if we read less than requested */
    if (ret < static_cast<sf_count_t>(samples.size())) {
        samples.resize(static_cast<size_t>(ret));
        _buf.resize(static_cast<size_t>(ret * _sfinfo.channels));
    }

    /* Mix multiple channels into one */
    for (unsigned int i = 0; i < samples.size(); i++) {
        float sum = 0;
        for (unsigned int j = 0; j < static_cast<unsigned int>(_sfinfo.channels); j++)
            sum += _buf[i * static_cast<unsigned int>(_sfinfo.channels) + j];

        samples[i] = sum / static_cast<float>(_sfinfo.channels);
    }
}

void WaveAudioSource::_read_single_channel(std::vector<float> &samples) {
    sf_count_t ret;

    ret = sf_read_float(_sndfile, samples.data(), static_cast<sf_count_t>(samples.size()));

    /* Resize samples buffer if we read less than requested */
    if (ret < static_cast<sf_count_t>(samples.size()))
        samples.resize(static_cast<size_t>(ret));
}

void WaveAudioSource::read(std::vector<float> &samples) {
    if (_sfinfo.channels == 1)
        _read_single_channel(samples);
    else
        _read_multi_channel(samples);
}

unsigned int WaveAudioSource::getSampleRate() {
    return static_cast<unsigned int>(_sfinfo.samplerate);
}

}
