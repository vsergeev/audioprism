#include "audiosource.hpp"

#include <pulse/simple.h>

class PulseAudioSource : public AudioSource {
  public:
    PulseAudioSource();
    ~PulseAudioSource();
    virtual void read(float *samples, size_t num);
  private:
    pa_simple *s;
};

