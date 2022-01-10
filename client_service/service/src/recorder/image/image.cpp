#include "recorder/image/image.h"

#include <cstring>
#include <png.h>
#include "shared/errors/error.h"
#include "shared/filesystem/utility.h"

#include <iostream>

namespace service::recorder::image {

Image::Image(Image&& other) {
    _width = other._width;
    _height = other._height;
    _buffer = std::move(other._buffer);
    
#ifdef _WIN32
    _stagingTexture = other._stagingTexture;
    other._stagingTexture = nullptr;
#endif
}

Image::~Image() {
#ifdef _WIN32
    if (_stagingTexture) {
        _stagingTexture->Release();
        _stagingTexture = nullptr;
    }
#endif
}

void Image::initializeImage(size_t width, size_t height, DXGI_FORMAT format) {
    _width = width;
    _height = height;
    _format = format;
    
    _buffer.reset(new uint8_t[numBytes()]);
    memset(_buffer.get(), 0, numBytes());
}

void Image::copyFrom(const Image& img) {
    assert(width() == img.width());
    assert(height() == img.height());
    assert(bytesPerPixel() == img.bytesPerPixel());
    std::memcpy(_buffer.get(), img.buffer(), numBytes());
}

size_t Image::bytesPerPixel() const {
    if (_format == DXGI_FORMAT_B8G8R8A8_UNORM) {
        return 4;
    } else {
        return 8;
    }
}

void Image::fillAlpha(uint8_t v) {
    for (auto h = 0; h < height(); ++h) {
        for (auto w = 0; w < width(); ++w) {
            const auto idx = h * numBytesPerRow() + w * bytesPerPixel() + 3;
            _buffer[idx] = v;
        }
    }
}

#ifdef _WIN32
void Image::loadFromD3d11Texture(ID3D11DeviceContext* context, ID3D11Texture2D* texture) {
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HRESULT hr = context->Map(texture, 0, D3D11_MAP_READ, 0, &mappedData);
    if (hr != S_OK) {
        THROW_ERROR("Failed to map texture: " << hr);
    }

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    const uint8_t* src = reinterpret_cast<const uint8_t*>(mappedData.pData);
    uint8_t* dst = buffer();
    for (size_t r = 0; r < _height; ++r) {
        std::memcpy(dst, src, numBytesPerRow());
        src += mappedData.RowPitch;
        dst += numBytesPerRow();
    }
    context->Unmap(texture, 0);
}

void Image::loadFromD3d11TextureWithStaging(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture) {
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    bool requiresStagingCreation = false;
    if (_stagingTexture) {
        D3D11_TEXTURE2D_DESC stagingDesc;
        _stagingTexture->GetDesc(&stagingDesc);

        requiresStagingCreation = 
            (desc.Width != stagingDesc.Width) ||
            (desc.Height != stagingDesc.Height) ||
            (desc.Format != stagingDesc.Format);
    } else {
        requiresStagingCreation = true;
    }

    if (requiresStagingCreation) {
        assert(_format == desc.Format);
        D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
        sharedDesc.Width = desc.Width;
        sharedDesc.Height = desc.Height;
        sharedDesc.MipLevels = 1;
        sharedDesc.ArraySize = 1;
        sharedDesc.Format = desc.Format;
        sharedDesc.SampleDesc.Count = 1;
        sharedDesc.Usage = D3D11_USAGE_STAGING;
        sharedDesc.BindFlags = 0;
        sharedDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        sharedDesc.MiscFlags = 0;

        if (_stagingTexture) {
            _stagingTexture->Release();
            _stagingTexture = nullptr;
        }

        if (device->CreateTexture2D(&sharedDesc, nullptr, &_stagingTexture) != S_OK) {
            THROW_ERROR("Failed to create staging texture.");
        }
    }

    context->CopyResource(_stagingTexture, texture);
    loadFromD3d11Texture(context, _stagingTexture);
}

#endif

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
        shared::filesystem::pathUtf8(path).c_str(),
        0,
        _buffer.get(),
        0,
        NULL)) {
        throw std::runtime_error("Failed to write PNG image to file.");
    }
}

void Image::loadFromFile(const std::filesystem::path& path) {
    png_image image;
    memset(&image, 0, sizeof(png_image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, shared::filesystem::pathUtf8(path).c_str())) {
        throw std::runtime_error("Failed to read PNG image header.");
    }

    image.format = PNG_FORMAT_BGRA;
    initializeImage(image.width, image.height);
    if (!png_image_finish_read(&image, nullptr, buffer(), numBytesPerRow(), nullptr)) {
        throw std::runtime_error("Failed to read PNG data.");
    }
}

}