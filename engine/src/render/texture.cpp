#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "engine/render/texture.h"

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include "engine/utils/path.h"
#include "engine/utils/logger.h"

void Texture::uploadToGPU(const unsigned char* pixels, int width, int height, int channels) {
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}

static void setDefaultParams() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::Texture(const std::string& path) {
    std::string resolvedPath = Path::resolve(path).string();
    
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    setDefaultParams();

    int width, height, nrChannels;
    unsigned char* data = stbi_load(resolvedPath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        uploadToGPU(data, width, height, nrChannels);
    } else {
        Logger::error("Failed to load texture at: " + resolvedPath);
    }

    stbi_image_free(data);
}

Texture::Texture(const unsigned char* data, int width, int height, int channels) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    setDefaultParams();

    if (data) {
        uploadToGPU(data, width, height, channels);
    } else {
        Logger::error("Failed to create embedded texture: null pixel data");
    }
}

Texture::Texture(const unsigned char* compressedData, unsigned int dataLength) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    setDefaultParams();

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

Texture::~Texture() {
    glDeleteTextures(1, &ID);
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}