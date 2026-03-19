#pragma once
#include <string>

class Texture {
public:
    unsigned int ID;
    Texture(const std::string& path);
    ~Texture();

    void bind(unsigned int unit = 0) const;
    void unbind() const;
};