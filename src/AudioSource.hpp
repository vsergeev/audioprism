#ifndef _AUDIO_SOURCE_HPP
#define _AUDIO_SOURCE_HPP

#include <vector>

class AudioSource {
  public:
    virtual ~AudioSource() { }
    virtual void read(std::vector<double> &samples) = 0;
    virtual unsigned int getSampleRate() = 0;
};

#endif

