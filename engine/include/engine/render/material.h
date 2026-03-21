#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "engine/core/math.h"

class Shader;
class Texture;

class Material {
public:
    explicit Material(std::shared_ptr<Shader> shader);

    // Shader management
    void setShader(std::shared_ptr<Shader> shader);
    Shader& getShader() const;
    std::shared_ptr<Shader> getShaderHandle() const { return m_shader; }

    // Texture management
    void setTexture(const std::string& slot, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(const std::string& slot) const;
    bool hasTexture(const std::string& slot) const;

    // Blinn-Phong parameters
    void setAmbientStrength(float strength) { m_ambientStrength = strength; }
    float getAmbientStrength() const { return m_ambientStrength; }

    void setSpecularStrength(float strength) { m_specularStrength = strength; }
    float getSpecularStrength() const { return m_specularStrength; }

    void setShininess(float shininess) { m_shininess = shininess; }
    float getShininess() const { return m_shininess; }

    void setBaseColor(Vec3 color) { m_baseColor = color; }
    Vec3 getBaseColor() const { return m_baseColor; }

private:
    static std::string normalizeSlot(const std::string& slot);

    std::shared_ptr<Shader> m_shader;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

    // Blinn-Phong params
    float m_ambientStrength = 0.1f;
    float m_specularStrength = 1.0f;
    float m_shininess = 32.0f;
    Vec3 m_baseColor = Vec3(1.0f, 1.0f, 1.0f);
};
