#pragma once

#include <string>

#include <sndfile.h>

#include "AudioSource.hpp"

namespace Audio {

class WaveAudioSource : public AudioSource {
  public:
    WaveAudioSource(std::string path);
    ~WaveAudioSource();
    virtual void read(std::vector<float> &samples);
    virtual unsigned int getSampleRate();

  private:
    void _read_multi_channel(std::vector<float> &samples);
    void _read_single_channel(std::vector<float> &samples);

    SNDFILE *_sndfile;
    SF_INFO _sfinfo;
    std::vector<float> _buf;
};

}
