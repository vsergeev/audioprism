#ifndef _PULSEAUDIOSOURCE_HPP
#define _PULSEAUDIOSOURCE_HPP

#include <pulse/simple.h>

#include "AudioSource.hpp"

namespace Audio {

class PulseAudioSource : public AudioSource {
  public:
    PulseAudioSource(unsigned int sampleRate);
    ~PulseAudioSource();
    virtual void read(std::vector<double> &samples);
    virtual unsigned int getSampleRate();

  private:
    pa_simple *s;
    unsigned int sampleRate;
};
}

#endif
