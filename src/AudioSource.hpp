#ifndef _AUDIO_SOURCE_HPP
#define _AUDIO_SOURCE_HPP

#include <cstdlib>

class AudioSource {
  public:
    AudioSource() { }
    virtual ~AudioSource() { }
    virtual void read(double *samples, size_t num) = 0;
    virtual unsigned int getSampleRate() = 0;
};

#endif
