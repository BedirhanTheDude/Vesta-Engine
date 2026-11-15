#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class IconRegistry {
public:
    using IconId = uint32_t;
    static constexpr IconId INVALID_ICON = UINT32_MAX;

    IconId registerIcon(const std::filesystem::path& svgPath);

    void mapExtension(const std::string& extension, IconId icon);
    void setDirectoryIcon(IconId icon) { m_DirectoryIcon = icon; }
    void setFallbackIcon(IconId icon) { m_FallbackIcon = icon; }

    // the fallback icon for unmapped extensions, INVALID_ICON if there is none
    IconId iconFor(const std::filesystem::path& path, bool isDirectory) const;

    // GL texture of the icon rasterised at size x size, 0 if the icon is invalid or failed to load
    uint32_t getTexture(IconId icon, uint32_t size);

private:
    struct Icon {
        std::filesystem::path svgPath;
        uint32_t size = 0; // the size the cached texture was rasterised at, it's a square
        uint32_t texture = 0;
    };

    static std::string toLower(std::string s);

    std::vector<Icon> m_Icons;
    std::unordered_map<std::string, IconId> m_ByExtension;
    IconId m_DirectoryIcon = INVALID_ICON;
    IconId m_FallbackIcon = INVALID_ICON;
};
