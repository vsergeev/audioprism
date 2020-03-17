#include <Magick++.h>

#include "MagickImageSink.hpp"

namespace Image {

MagickImageSink::MagickImageSink(std::string path, unsigned int spectrumWidth, Orientation orientation) : _path(path), _spectrumWidth(spectrumWidth), _orientation(orientation) {
    Magick::InitializeMagick(nullptr);
}

void MagickImageSink::append(const std::vector<uint32_t> &pixels) {
    _imagePixels.insert(_imagePixels.end(), pixels.begin(), pixels.end());
}

void MagickImageSink::write() {
    Magick::Image image(_spectrumWidth, static_cast<unsigned int>(_imagePixels.size() / _spectrumWidth), "BGRA", Magick::CharPixel, _imagePixels.data());
    image.quality(100);
    image.opacity(0);

    if (_orientation == Orientation::Horizontal)
        image.rotate(-90);

    image.write(_path);
}

}
