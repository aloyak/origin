#include "engine/render/material.h"

#include <stdexcept>

#include "engine/render/shader.h"
#include "engine/render/texture.h"

Material::Material(std::shared_ptr<Shader> shader)
    : m_shader(std::move(shader))
{}

void Material::setShader(std::shared_ptr<Shader> shader) {
    m_shader = std::move(shader);
}

Shader& Material::getShader() const {
    if (!m_shader) {
        throw std::runtime_error("Material has no shader assigned");
    }
    return *m_shader;
}

void Material::setTexture(const std::string& slot, std::shared_ptr<Texture> texture) {
    const auto key = normalizeSlot(slot);
    if (!texture) {
        m_textures.erase(key);
        return;
    }

    m_textures[key] = std::move(texture);
}

std::shared_ptr<Texture> Material::getTexture(const std::string& slot) const {
    const auto key = normalizeSlot(slot);
    auto it = m_textures.find(key);
    if (it == m_textures.end()) {
        return nullptr;
    }

    return it->second;
}

bool Material::hasTexture(const std::string& slot) const {
    const auto key = normalizeSlot(slot);
    auto it = m_textures.find(key);
    return it != m_textures.end() && static_cast<bool>(it->second);
}

std::string Material::normalizeSlot(const std::string& slot) {
    if (slot == "diffuse" || slot == "texture_diffuse") {
        return "texture_diffuse";
    }
    if (slot == "specular" || slot == "texture_specular") {
        return "texture_specular";
    }
    if (slot == "normal" || slot == "texture_normal") {
        return "texture_normal";
    }
    if (slot == "metallic" || slot == "texture_metallic") {
        return "texture_metallic";
    }

    return slot;
}
