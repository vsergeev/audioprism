#include <stdexcept>
#include <iostream>

#include <sndfile.h>

#include "WaveAudioSource.hpp"

namespace Audio {

WaveAudioSource::WaveAudioSource(std::string path) : sfinfo() {
    if ((sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo)) == nullptr)
        throw OpenException("Error opening WAV file: " + std::string(sf_strerror(nullptr)));

    if (sfinfo.channels != 1)
        throw OpenException("Error: only one channel supported.");
}

WaveAudioSource::~WaveAudioSource() {
    if (sndfile)
        sf_close(sndfile);
}

void WaveAudioSource::read(std::vector<double> &samples) {
    sf_count_t ret;

    ret = sf_read_double(sndfile, samples.data(), samples.size());

    /* Resize samples buffer if we read less than requested */
    if (ret < static_cast<sf_count_t>(samples.size()))
        samples.resize(ret);
}

unsigned int WaveAudioSource::getSampleRate() {
    return sfinfo.samplerate;
}

}

