#include <utility/Hash.h>

uint32_t Hash::hashBytes(const void* data, size_t size, uint32_t currentHash) {
	uint32_t hash = currentHash;
	const uint8_t* bytes = static_cast<const uint8_t*>(data);
	for (size_t i = 0; i < size; ++i) {
		hash ^= static_cast<uint32_t>(bytes[i]);
		hash *= 16777619u;
	}
	return hash;
}
