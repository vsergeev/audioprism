#ifndef _PULSE_AUDIO_SOURCE_HPP
#define _PULSE_AUDIO_SOURCE_HPP

#include <pulse/simple.h>

#include "AudioSource.hpp"

class PulseAudioSource : public AudioSource {
  public:
    PulseAudioSource();
    ~PulseAudioSource();
    virtual void read(double *samples, size_t num);
    virtual unsigned int getSampleRate();
  private:
    pa_simple *s;
};

#endif

