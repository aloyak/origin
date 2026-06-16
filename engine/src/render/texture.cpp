#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "engine/render/texture.h"

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#include <algorithm>
#include <cmath>
#include <cstring>

static void setDefaultParams() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void genAndBind(unsigned int& id) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    setDefaultParams();
}


void Texture::uploadToGPU(const unsigned char* pixels, int width, int height, int channels) {
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    m_width    = width;
    m_height   = height;
    m_channels = channels;
}

Texture::Texture(const std::string& path) {
    std::string resolvedPath = Path::resolve(path).string();

    genAndBind(m_id);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(resolvedPath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        uploadToGPU(data, width, height, nrChannels);
        stbi_image_free(data);
    } else {
        Logger::error("Failed to load texture at: " + resolvedPath);
    }
}

Texture::Texture(const unsigned char* data, int width, int height, int channels) {
    genAndBind(m_id);

    if (data) {
        uploadToGPU(data, width, height, channels);
    } else {
        Logger::error("Failed to create embedded texture: null pixel data");
    }
}

Texture::Texture(const unsigned char* compressedData, unsigned int dataLength) {
    genAndBind(m_id);

    int width, height, nrChannels;
    unsigned char* pixels = stbi_load_from_memory(
        compressedData, static_cast<int>(dataLength),
        &width, &height, &nrChannels, 0
    );

    if (pixels) {
        uploadToGPU(pixels, width, height, nrChannels);
        stbi_image_free(pixels);
    } else {
        Logger::error("Failed to decode compressed embedded texture: " +
                      std::string(stbi_failure_reason()));
    }
}

Texture* Texture::createPaintable(Vec2 size, Vec4 color)
{
    Texture* t = new Texture();

    genAndBind(t->m_id);

    t->m_channels = 4;
    t->m_width    = size.x;
    t->m_height   = size.y;

    t->m_pixels.resize(size.x * size.y * 4);
    for (int i = 0; i < size.x * size.y; ++i) {
        t->m_pixels[i * 4 + 0] = color.x * 255;
        t->m_pixels[i * 4 + 1] = color.y * 255;
        t->m_pixels[i * 4 + 2] = color.z * 255;
        t->m_pixels[i * 4 + 3] = color.w * 255;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, t->m_pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    return t;
}

Texture::~Texture() {
    glDeleteTextures(1, &m_id);
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}


// Paint
void Texture::paint(float u, float v, float brushRadius, Vec4 color)
{
    if (m_pixels.empty()) {
        Logger::warn("Texture::paint called on a non-paintable texture, ignoring");
        return;
    }

    // Convert UV + radius to pixel space
    const float cx = u * static_cast<float>(m_width);
    const float cy = (1.0f - v) * static_cast<float>(m_height); // flip V, UV origin is bottom-left
    const float rPx = brushRadius * static_cast<float>(m_width);

    const int x0 = static_cast<int>(std::floor(cx - rPx));
    const int y0 = static_cast<int>(std::floor(cy - rPx));
    const int x1 = static_cast<int>(std::ceil (cx + rPx));
    const int y1 = static_cast<int>(std::ceil (cy + rPx));

    // Clamp to texture bounds
    const int px0 = std::max(x0, 0);
    const int py0 = std::max(y0, 0);
    const int px1 = std::min(x1, m_width  - 1);
    const int py1 = std::min(y1, m_height - 1);

    const float rPx2 = rPx * rPx;

    for (int py = py0; py <= py1; ++py) {
        for (int px = px0; px <= px1; ++px) {
            const float dx = static_cast<float>(px) - cx;
            const float dy = static_cast<float>(py) - cy;
            const float dist2 = dx * dx + dy * dy;

            if (dist2 > rPx2) continue;

            const float dist   = std::sqrt(dist2);
            const float t      = 1.0f - (dist / rPx);
            const float alpha  = t * t * color.w * 255;  // quadratic falloff

            const int idx = (py * m_width + px) * 4;
            auto& pr = m_pixels[idx + 0];
            auto& pg = m_pixels[idx + 1];
            auto& pb = m_pixels[idx + 2];

            auto lerp8 = [](uint8_t src, uint8_t dst, float a) -> uint8_t {
                return static_cast<uint8_t>(static_cast<int>(src) + static_cast<int>(static_cast<int>(dst) - static_cast<int>(src)) * a);
            };
            pr = lerp8(pr, color.x * 255, alpha);
            pg = lerp8(pg, color.y * 255, alpha);
            pb = lerp8(pb, color.z * 255, alpha);
        }
    }

    uploadRegion(px0, py0, px1 - px0 + 1, py1 - py0 + 1);
}


void Texture::uploadToGPU() {
    if (m_pixels.empty()) return;

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::uploadRegion(int x, int y, int w, int h) {
    if (m_pixels.empty()) return;

    x = std::max(x, 0);
    y = std::max(y, 0);
    w = std::min(w, m_width  - x);
    h = std::min(h, m_height - y);
    if (w <= 0 || h <= 0) return;

    glBindTexture(GL_TEXTURE_2D, m_id);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,   y);

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    x, y, w, h,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    m_pixels.data());

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,   0);

    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::fill(Vec4 color) {
    if (m_pixels.empty()) {
        Logger::warn("Texture::fill called on a non-paintable texture, ignoring");
        return;
    }

    for (int i = 0; i < m_width * m_height; ++i) {
        m_pixels[i * 4 + 0] = color.x * 255;
        m_pixels[i * 4 + 1] = color.y * 255;
        m_pixels[i * 4 + 2] = color.z * 255;
        m_pixels[i * 4 + 3] = color.w * 255;
    }

    uploadToGPU();
}

bool Texture::save(const std::string& path) const {
    if (m_pixels.empty()) {
        Logger::error("Texture::save called on a non-paintable texture, no CPU buffer");
        return false;
    }

    std::string resolvedPath = Path::resolve(path).string();
    int result = stbi_write_png(
        resolvedPath.c_str(),
        m_width, m_height,
        4,                       // always RGBA
        m_pixels.data(),
        m_width * 4              // stride in bytes
    );

    if (!result) {
        Logger::error("Texture::save failed to write PNG at: " + resolvedPath);
        return false;
    }

    return true;
}