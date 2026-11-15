#pragma once

#include <cstdint>
#include <filesystem>

namespace Thumbnail {
	uint32_t loadThumbnailFromImage(const std::filesystem::path& imagePath);
	uint32_t loadThumbnailFromSvg(const std::filesystem::path& svgPath, uint32_t targetWidth, uint32_t targetHeight);
	void releaseThumbnails();
}