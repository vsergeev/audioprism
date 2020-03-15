#pragma once

#include <stdexcept>
#include <vector>

namespace Audio {

class AudioSource {
  public:
    virtual ~AudioSource() {}
    virtual void read(std::vector<double> &samples) = 0;
    virtual unsigned int getSampleRate() = 0;
};

class OpenException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class ReadException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

}
