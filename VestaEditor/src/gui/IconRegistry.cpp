#include "IconRegistry.h"

#include <gui/Thumbnail.h>

#include <algorithm>
#include <cctype>

IconRegistry::IconId IconRegistry::registerIcon(const std::filesystem::path& svgPath) {
    m_Icons.push_back({ svgPath });
    return static_cast<IconId>(m_Icons.size() - 1);
}

void IconRegistry::mapExtension(const std::string& extension, IconId icon) {
    m_ByExtension[toLower(extension)] = icon;
}

IconRegistry::IconId IconRegistry::iconFor(const std::filesystem::path& path, bool isDirectory) const {
    if (isDirectory) return m_DirectoryIcon;

    auto it = m_ByExtension.find(toLower(path.extension().string()));
    return it != m_ByExtension.end() ? it->second : m_FallbackIcon;
}

uint32_t IconRegistry::getTexture(IconId icon, uint32_t size) {
    if (icon >= m_Icons.size()) return 0;

    Icon& entry = m_Icons[icon];
    if (entry.size != size) {
        entry.texture = Thumbnail::loadThumbnailFromSvg(entry.svgPath, size, size);
        entry.size = size;
    }

    return entry.texture;
}

std::string IconRegistry::toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
