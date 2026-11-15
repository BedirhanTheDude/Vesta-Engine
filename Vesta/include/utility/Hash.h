#pragma once

#include <cstdint>

namespace Hash {
	constexpr uint32_t hashString(const char* str) {
		uint32_t hash = 2166136261u;
		while (*str) {
			hash ^= static_cast<uint32_t>(*str++);
			hash *= 16777619u;
		}
		return hash;
	}
	uint32_t hashBytes(const void* data, size_t size, uint32_t currentHash = 2166136261u);
}