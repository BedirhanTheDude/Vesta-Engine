#include "stb_image.h"

#include <renderer/Texture.h>

#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <climits>

Texture::Texture(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open texture: " << filePath << std::endl;
        return;
    }

    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty() || bytes.size() > static_cast<size_t>(INT_MAX)) {
        std::cerr << "Failed to read texture: " << filePath << std::endl;
        return;
    }

    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char* data = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()),
        &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

Texture::~Texture() {
    if (id) glDeleteTextures(1, &id);
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}