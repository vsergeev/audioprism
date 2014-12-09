#ifndef _WAVEAUDIOSOURCE_HPP
#define _WAVEAUDIOSOURCE_HPP

#include <string>

#include <sndfile.h>

#include "AudioSource.hpp"

namespace Audio {

class WaveAudioSource : public AudioSource {
  public:
    WaveAudioSource(std::string path);
    ~WaveAudioSource();
    virtual void read(std::vector<double> &samples);
    virtual unsigned int getSampleRate();

  private:
    SNDFILE *sndfile;
    SF_INFO sfinfo;
};
}

#endif
