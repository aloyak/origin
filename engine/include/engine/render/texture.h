#pragma once

#include <vector>
#include <string>
#include "engine/core/math.h"

class Texture {
public:
    // no CPU buffer retained
    Texture(const std::string& path);
    Texture(const unsigned char* data, int width, int height, int channels);  // embedded / in-memory
    Texture(const unsigned char* compressedData, unsigned int dataLength);    // compressed embedded (e.g. PNG/JPG inside FBX)
    
    static Texture* createPaintable(Vec2 size, Vec4 color = Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    static Texture* createPaintable(const std::string& sourcePath);

    ~Texture();

    // owns a GPU texture object
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(unsigned int unit = 0) const;
    void unbind() const;

    unsigned int getID() const { return m_id; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }
    bool isPaintable() const { return !m_pixels.empty(); }


    void paint(float u, float v, float brushRadius, Vec4 color);

    void uploadToGPU(); // push the entire CPU buffer to GPU (for paintable textures)
    void uploadRegion(int x, int y, int width, int height); 

    bool save(const std::string& path) const;
    void fill(Vec4 color);

    void setPixel(int px, int py, Vec4 color);
private:
    Texture() = default;

    void uploadToGPU(const unsigned char* pixels, int width, int height, int channels);

    unsigned int m_id = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;

    std::vector<uint8_t> m_pixels;
};