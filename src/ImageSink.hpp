#ifndef _IMAGESINK_HPP
#define _IMAGESINK_HPP

#include <vector>
#include <cstdint>

class ImageSink {
  public:
    ImageSink() { }
    virtual ~ImageSink() { }

    virtual void append(const std::vector<uint32_t> &pixels) = 0;
    virtual void write() = 0;
};

#endif

