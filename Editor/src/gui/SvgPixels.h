#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace SvgPixels {
    struct Pixels {
        std::vector<uint8_t> data;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    Pixels loadPixels(const std::string& svgPath, uint32_t targetWidth, uint32_t targetHeight);
}