#pragma once

#include <pulse/simple.h>

#include "AudioSource.hpp"

namespace Audio {

class PulseAudioSource : public AudioSource {
  public:
    PulseAudioSource(unsigned int sampleRate);
    ~PulseAudioSource();
    virtual void read(std::vector<float> &samples);
    virtual unsigned int getSampleRate();

  private:
    pa_simple *_handle;
    unsigned int _sampleRate;
};

}
