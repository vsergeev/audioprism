#include <stdexcept>
#include <iostream>

#include <sndfile.h>

#include "WaveAudioSource.hpp"

namespace Audio {

WaveAudioSource::WaveAudioSource(std::string path) : _sfinfo() {
    if ((_sndfile = sf_open(path.c_str(), SFM_READ, &_sfinfo)) == nullptr)
        throw OpenException("Error opening WAV file: " + std::string(sf_strerror(nullptr)));

    if (_sfinfo.channels != 1)
        throw OpenException("Error: only one channel supported.");
}

WaveAudioSource::~WaveAudioSource() {
    if (_sndfile)
        sf_close(_sndfile);
}

void WaveAudioSource::read(std::vector<double> &samples) {
    sf_count_t ret;

    ret = sf_read_double(_sndfile, samples.data(), static_cast<sf_count_t>(samples.size()));

    /* Resize samples buffer if we read less than requested */
    if (ret < static_cast<sf_count_t>(samples.size()))
        samples.resize(static_cast<size_t>(ret));
}

unsigned int WaveAudioSource::getSampleRate() {
    return static_cast<unsigned int>(_sfinfo.samplerate);
}

}
