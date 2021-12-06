#include "shared/image/ico/ico.h"
#include "shared/log/log.h"
#include "shared/memory/byte_writer.h"

namespace shared::image::ico {
namespace {

size_t getNumBytesForBitmap(const BITMAP& bm) {
    // Number of bytes in each row must be divisible by 4 as per BITMAP spec.
    const auto bytesPerRow = (bm.bmWidthBytes + 3) & ~0x03;
    return bm.bmHeight * bytesPerRow;
}

void readBitmapIntoBuffer(HBITMAP hbm, const BITMAP& bm, std::vector<char>& buffer) {
    buffer.resize(getNumBytesForBitmap(bm));

    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biPlanes = bm.bmPlanes;
    bi.biBitCount = bm.bmBitsPixel;
    bi.biCompression = bm.bmBitsPixel == 1 ? BI_BITFIELDS : BI_RGB;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biSizeImage = getNumBytesForBitmap(bm);

    const auto dc = GetDC(NULL);
    GetDIBits(
        dc,
        hbm,
        0,
        bm.bmHeight,
        (void*)buffer.data(),
        (BITMAPINFO*)&bi,
        DIB_RGB_COLORS
    );
    ReleaseDC(NULL, dc);   
}

}

IcoImage::IcoImage() {
    _dir.reserved = 0;
    _dir.type = 1;
}

void IcoImage::addIcon(HICON ico) {
    ICONINFO info;
    if (!GetIconInfo(ico, &info)) {
        LOG_WARNING("Failed to get icon info." << std::endl);
        return;
    }

    BITMAP color;
    if (!GetObject(info.hbmColor, sizeof(BITMAP), &color)) {
        LOG_WARNING("Failed to get color bitmap." << std::endl);
        return;
    }

    BITMAP mask;
    if (!GetObject(info.hbmMask, sizeof(BITMAP), &mask)) {
        LOG_WARNING("Failed to get mask bitmap." << std::endl);
        return;
    }

    _dir.numImages += 1;
    const auto& bitmapEntry = addBitmapEntry(info.hbmColor, color, info.hbmMask, mask);

    // Fill in the offset field later when/if
    // we ever need to serialize.
    ICONDIRENTRY newEntry = { 0 };
    newEntry.width = color.bmWidth;
    newEntry.height = color.bmHeight;
    
    // According to the BITMAPINFOHEADER documentation: if biCompression equals BI_RGB
    // and the bitmap uses 8bpp or less, then the bitmap has a color table in which case
    // numColorsInPalette needs to be filled out.
    if (color.bmBitsPixel < 8) {
        newEntry.numColorsInPalette =  1 << (color.bmBitsPixel * color.bmPlanes);
    } else {
        newEntry.numColorsInPalette = 0;
    }
    newEntry.val1 = color.bmPlanes;
    newEntry.val2 = color.bmBitsPixel;
    newEntry.imageDataBytes = bitmapEntry.byteSize();
    // Filled in only upon serialization.
    newEntry.imageDataOffset = 0;
    _entries.push_back(newEntry);

    DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
}

std::vector<char> IcoImage::serialize() const {
    const auto byteSize = sizeof(ICONDIR) + sizeof(ICONDIRENTRY) * _entries.size() + [this](){
        size_t imageSize = 0;
        for (const auto& img: _images) {
            imageSize += img.byteSize();
        }
        return imageSize;
    }();

    std::vector<char> buffer(byteSize);
    shared::memory::ByteWriter writer(buffer);
    writer.writeData(_dir);

    assert(_entries.size() == _images.size());
    auto totalImageOffset = sizeof(ICONDIR) + sizeof(ICONDIRENTRY) * _entries.size();
    for (auto i = 0; i < _entries.size(); ++i) {
        auto serializableEntry = _entries[i];
        serializableEntry.imageDataOffset = totalImageOffset;
        writer.writeData(serializableEntry);
        totalImageOffset += _images[i].byteSize();
    }

    for (const auto& image: _images) {
        writer.writeData(image.header);
        writer.writeData(image.colorData);
        writer.writeData(image.maskData);
    }

    return buffer;
}

const IcoImage::BitmapEntry& IcoImage::addBitmapEntry(HBITMAP hColor, const BITMAP& color, HBITMAP hMask, const BITMAP& mask) {
    IcoImage::BitmapEntry newEntry;
    newEntry.header = { 0 };
    newEntry.header.biSize = sizeof(BITMAPINFOHEADER);
    newEntry.header.biWidth = color.bmWidth;
    // As per the ICO spec - the BITMAP needs to be twice the height of the image. Second half being the mask.
    newEntry.header.biHeight = color.bmHeight * 2;
    newEntry.header.biSizeImage = getNumBytesForBitmap(color) + getNumBytesForBitmap(mask);
    newEntry.header.biPlanes = color.bmPlanes;
    newEntry.header.biBitCount = color.bmBitsPixel;

    readBitmapIntoBuffer(hColor, color, newEntry.colorData);
    readBitmapIntoBuffer(hMask, mask, newEntry.maskData);

    _images.push_back(newEntry);
    return _images.back();
}

size_t IcoImage::BitmapEntry::byteSize() const {
    return sizeof(BITMAPINFOHEADER) + colorData.size() + maskData.size();
}

}