#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Shader;
class Texture;

class Material {
public:
    explicit Material(std::shared_ptr<Shader> shader);

    void setShader(std::shared_ptr<Shader> shader);
    Shader& getShader() const;
    std::shared_ptr<Shader> getShaderHandle() const { return m_shader; }

    void setTexture(const std::string& slot, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(const std::string& slot) const;
    bool hasTexture(const std::string& slot) const;

private:
    static std::string normalizeSlot(const std::string& slot);

    std::shared_ptr<Shader> m_shader;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};
