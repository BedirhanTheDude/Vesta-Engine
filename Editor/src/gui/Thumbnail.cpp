#include "Thumbnail.h"

#include <gui/SvgPixels.h>
#include <utility/Hash.h>

#include <stb_image.h>
#include <glad/glad.h>

#include <string>
#include <cstdint>
#include <unordered_map>

static std::unordered_map<uint32_t, unsigned int> _thumbnailCache;

static uint32_t _loadThumbnail(const uint8_t* data, uint32_t width, uint32_t height) {
    uint32_t texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texId;
}

static bool _tryGetFromCache(uint32_t key, uint32_t* outTexId) {
    auto it = _thumbnailCache.find(key);
    if (it != _thumbnailCache.end()) {
        *outTexId = it->second;
        return true;
    }

    return false;
}

uint32_t Thumbnail::loadThumbnailFromImage(const std::filesystem::path& imagePath) {
    std::string pathStr = imagePath.string();
    const char* cStr = pathStr.c_str();

    uint32_t key = Hash::hashString(cStr);

    uint32_t texId = 0;
    if (_tryGetFromCache(key, &texId)) return texId;

    int w, h, ch;
    stbi_set_flip_vertically_on_load(false);
    uint8_t* data = stbi_load(cStr, &w, &h, &ch, 4);
    if (!data) return 0;

    texId = _loadThumbnail(data, w, h);

    stbi_image_free(data);
    _thumbnailCache[key] = texId;
    return texId;
}

uint32_t Thumbnail::loadThumbnailFromSvg(const std::filesystem::path& svgPath, uint32_t targetWidth, uint32_t targetHeight) {
    std::string pathStr = svgPath.string();

    uint32_t key = Hash::hashString(pathStr.c_str());
    
    key = Hash::hashBytes(&targetWidth, sizeof(targetWidth), key);
    key = Hash::hashBytes(&targetHeight, sizeof(targetHeight), key);

    uint32_t texId = 0;
    if (_tryGetFromCache(key, &texId)) return texId;

    SvgPixels::Pixels svgPixels = SvgPixels::loadPixels(pathStr, targetWidth, targetHeight);
    
    texId = _loadThumbnail(svgPixels.data.data(), svgPixels.width, svgPixels.height);

    svgPixels.data.clear();
    _thumbnailCache[key] = texId;
    return texId;
}

void Thumbnail::releaseThumbnails() {
    for (auto& [k, id] : _thumbnailCache)
        if (id) glDeleteTextures(1, &id);
    _thumbnailCache.clear();
}