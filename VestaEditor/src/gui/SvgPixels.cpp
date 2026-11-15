#include "SvgPixels.h"

#include <lunasvg.h>

#include <vector>
#include <cstring>

namespace SvgPixels {
    Pixels loadPixels(const std::string& svgPath, uint32_t targetWidth, uint32_t targetHeight) {
        Pixels output;

        auto document = lunasvg::Document::loadFromFile(svgPath);
        if (!document) return output;

        auto bitmap = document->renderToBitmap(targetWidth, targetHeight);

        output.width = bitmap.width();
        output.height = bitmap.height();

        size_t size = output.width * output.height * 4; // RGBA 4 bytes per pixel
        output.data.resize(size);
        std::memcpy(output.data.data(), bitmap.data(), size);

        return output;
    }
}