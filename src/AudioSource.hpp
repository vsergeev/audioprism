#ifndef _AUDIO_SOURCE_HPP
#define _AUDIO_SOURCE_HPP

#include <stdexcept>
#include <vector>

class AudioSource {
  public:
    virtual ~AudioSource() { }
    virtual void read(std::vector<double> &samples) = 0;
    virtual unsigned int getSampleRate() = 0;
};

class AudioOpenException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class AudioReadException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

#endif

