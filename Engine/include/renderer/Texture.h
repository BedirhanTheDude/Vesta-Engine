#pragma once

#include <string>

class Texture {
public:
    Texture(const std::string& name);
    ~Texture();

    void bind(unsigned int unit = 0) const;

    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }
private:
    unsigned int id = 0;
    std::string name;
};