#include "recorder/image/image.h"

#include <png.h>
#include "shared/errors/error.h"

namespace service::recorder::image {

void Image::initializeImage(size_t width, size_t height) {
    _buffer.reset(new uint8_t[width * height * bytesPerPixel()]);
    _width = width;
    _height = height;
}

void Image::saveToFile(const std::filesystem::path& path) const {
    png_image image;
    memset(&image, 0, sizeof(png_image));
    
    image.version = PNG_IMAGE_VERSION;
    image.opaque = NULL;
    image.width = static_cast<int>(width());
    image.height = static_cast<int>(height());
    image.format = PNG_FORMAT_BGRA;

    if (!png_image_write_to_file(
        &image,
        path.string().c_str(),
        0,
        _buffer.get(),
        0,
        NULL)) {
        THROW_ERROR("Failed to write PNG image to file.");
    }
}

}