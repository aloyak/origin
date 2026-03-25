#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class Model;
class Shader;
class Texture;

class ResourceManager {
public:
    static ResourceManager& instance();

    std::shared_ptr<Model> getModel(const std::string& modelPath);
    std::shared_ptr<Shader> getShader(const std::string& vertexPath, const std::string& fragmentPath);
    std::shared_ptr<Texture> getTexture(const std::string& texturePath);

    void purgeUnused();
    void clear();

private:
    ResourceManager() = default;

    static std::string normalizePath(const std::string& path);
    static std::string buildShaderKey(const std::string& vertexPath, const std::string& fragmentPath);

    std::unordered_map<std::string, std::weak_ptr<Model>> m_models;
    std::unordered_map<std::string, std::weak_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_textures;

    std::mutex m_mutex;
};
