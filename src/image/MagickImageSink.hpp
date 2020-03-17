#pragma once

#include <string>

#include "ImageSink.hpp"

namespace Image {

class MagickImageSink : public ImageSink {
  public:
    enum class Orientation { Horizontal,
                             Vertical };

    MagickImageSink(std::string path, unsigned int spectrumWidth, Orientation orientation);

    virtual void append(const std::vector<uint32_t> &pixels);
    virtual void write();

  private:
    const std::string _path;
    const unsigned int _spectrumWidth;
    const Orientation _orientation;
    std::vector<uint32_t> _imagePixels;
};

}
