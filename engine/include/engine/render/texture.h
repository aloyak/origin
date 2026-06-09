#pragma once
#include <string>

class Texture {
public:
    unsigned int ID;
    Texture(const std::string& path);
    Texture(const unsigned char* data, int width, int height, int channels);  // embedded / in-memory
    Texture(const unsigned char* compressedData, unsigned int dataLength);    // compressed embedded (e.g. PNG/JPG inside FBX)
    ~Texture();

    void bind(unsigned int unit = 0) const;
    void unbind() const;

private:
    void uploadToGPU(const unsigned char* pixels, int width, int height, int channels);
};