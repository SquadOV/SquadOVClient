#pragma once

#include <wingdi.h>
#include <string>
#include <regex>

namespace shared::color {

struct ColorRgba {
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float a = 0.f;

    static ColorRgba parseCssRgba(const std::string& rgba) {
        const std::regex rgbaRegex("rgba\\((.*), (.*), (.*), (.*)\\)");

        ColorRgba ret;

        std::smatch matches;
        if (!std::regex_search(rgba, matches, rgbaRegex) || matches.size() < 5) {
            return ret;
        }

        ret.r = std::stof(matches[1].str()) / 255.f;
        ret.g = std::stof(matches[2].str()) / 255.f;
        ret.b = std::stof(matches[3].str()) / 255.f;
        ret.a = std::stof(matches[4].str());

        return ret;
    }

#ifdef _WIN32
    COLORREF toGdiColor() const {
        return RGB(
            static_cast<int>(r * 255.f),
            static_cast<int>(g * 255.f),
            static_cast<int>(b * 255.f)
        );
    }
#endif
};

}