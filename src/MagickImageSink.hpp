#ifndef _MAGICKIMAGESINK_HPP
#define _MAGICKIMAGESINK_HPP

#include <string>

#include "ImageSink.hpp"

class MagickImageSink : public ImageSink {
  public:
    MagickImageSink(std::string path, unsigned int width);

    virtual void append(const std::vector<uint32_t> &pixels);
    virtual void write();

  private:
    const std::string path;
    std::vector<uint32_t> imagePixels;
};

#endif

