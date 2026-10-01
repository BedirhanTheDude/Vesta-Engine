#pragma once

#include <string>
#include <filesystem>

class Texture {
public:
    Texture(const std::filesystem::path& filePath);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(unsigned int unit = 0) const;

    bool isLoaded() const { return id != 0; }

    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }
private:
    unsigned int id = 0;
    std::string name;
};
