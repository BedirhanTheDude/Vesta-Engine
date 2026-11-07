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

    bool isLoaded() const { return m_Id != 0; }

    const std::string& getName() const { return m_Name; }
    void setName(const std::string& n) { m_Name = n; }
private:
    unsigned int m_Id = 0;
    std::string m_Name;
};
