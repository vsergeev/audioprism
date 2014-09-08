#include <stdexcept>
#include <iostream>

#include <sndfile.h>

#include "WaveAudioSource.hpp"

WaveAudioSource::WaveAudioSource(std::string path) : sfinfo() {
    if ((sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo)) == nullptr)
        throw std::runtime_error("Error opening WAV file: " + std::string(sf_strerror(NULL)));

    if (sfinfo.channels != 1)
        throw std::runtime_error("Error: only one channel supported.");
}

WaveAudioSource::~WaveAudioSource() {
    if (sndfile)
        sf_close(sndfile);
}

void WaveAudioSource::read(std::vector<double> &samples) {
    sf_count_t ret;

    ret = sf_read_double(sndfile, samples.data(), samples.size());

    if (ret < samples.size())
        samples.resize(ret);
}

unsigned int WaveAudioSource::getSampleRate() {
    return sfinfo.samplerate;
}

