#include <cstdlib>

class AudioSource {
  public:
    AudioSource() { }
    virtual ~AudioSource() { }
    virtual void read(float *samples, size_t num) = 0;
};

