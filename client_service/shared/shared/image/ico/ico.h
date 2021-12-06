#pragma once

#include <Windows.h>
#include <stdint.h>
#include <vector>

namespace shared::image::ico {

// ICO structure as documented in wikipedia:
// https://en.wikipedia.org/wiki/ICO_(file_format)

struct ICONDIR {
    int16_t reserved = 0;
    int16_t type = 1; // 1: icon, 2: cursor
    int16_t numImages = 0;
};

struct ICONDIRENTRY {
    int8_t width = 0;
    int8_t height = 0;
    int8_t numColorsInPalette = 0;
    int8_t reserved = 0;
    int16_t val1; // For ICO: specifies color planes (0 or 1)
    int16_t val2; // For ICO: bits per pixel
    int32_t imageDataBytes;
    int32_t imageDataOffset;
};

class IcoImage {
public:
    IcoImage();
    void addIcon(HICON ico);
    std::vector<char> serialize() const;
    
private:
    ICONDIR _dir;
    std::vector<ICONDIRENTRY> _entries;

    struct BitmapEntry {
        BITMAPINFOHEADER header;
        std::vector<char> colorData;
        std::vector<char> maskData;

        size_t byteSize() const;
    };
    std::vector<BitmapEntry> _images;
    const BitmapEntry& addBitmapEntry(HBITMAP hColor, const BITMAP& color, HBITMAP hMask, const BITMAP& mask);
};

}