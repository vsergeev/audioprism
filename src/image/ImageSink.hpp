#pragma once

#include <vector>
#include <cstdint>

namespace Image {

class ImageSink {
  public:
    virtual ~ImageSink() {}
    virtual void append(const std::vector<uint32_t> &pixels) = 0;
    virtual void write() = 0;
};

}
