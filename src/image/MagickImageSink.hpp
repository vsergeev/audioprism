#pragma once

#include <string>

#include "ImageSink.hpp"

namespace Image {

class MagickImageSink : public ImageSink {
  public:
    enum class Orientation { Horizontal,
                             Vertical };

    MagickImageSink(std::string path, unsigned int width, Orientation orientation);

    virtual void append(const std::vector<uint32_t> &pixels);
    virtual void write();

  private:
    const std::string path;
    const unsigned int width;
    const Orientation orientation;
    std::vector<uint32_t> imagePixels;
};

}
