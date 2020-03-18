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
    SNDFILE *_sndfile;
    SF_INFO _sfinfo;
};

}
