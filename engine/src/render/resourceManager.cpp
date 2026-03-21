#include "engine/render/resourceManager.h"

#include "engine/debug/path.h"
#include "engine/render/model.h"
#include "engine/render/shader.h"
#include "engine/render/texture.h"

ResourceManager& ResourceManager::instance() {
    static ResourceManager manager;
    return manager;
}

std::shared_ptr<Model> ResourceManager::getModel(const std::string& modelPath) {
    const std::string key = normalizePath(modelPath);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_models.find(key);
        if (it != m_models.end()) {
            if (auto existing = it->second.lock()) {
                return existing;
            }
        }
    }

    auto created = std::make_shared<Model>(key.c_str());

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_models.find(key);
    if (it != m_models.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    m_models[key] = created;
    return created;
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string key = buildShaderKey(vertexPath, fragmentPath);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_shaders.find(key);
        if (it != m_shaders.end()) {
            if (auto existing = it->second.lock()) {
                return existing;
            }
        }
    }

    const std::string resolvedVertex = normalizePath(vertexPath);
    const std::string resolvedFragment = normalizePath(fragmentPath);

    auto created = std::make_shared<Shader>(resolvedVertex, resolvedFragment);

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_shaders.find(key);
    if (it != m_shaders.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    m_shaders[key] = created;
    return created;
}

std::shared_ptr<Texture> ResourceManager::getTexture(const std::string& texturePath) {
    const std::string key = normalizePath(texturePath);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_textures.find(key);
        if (it != m_textures.end()) {
            if (auto existing = it->second.lock()) {
                return existing;
            }
        }
    }

    auto created = std::make_shared<Texture>(key);

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    m_textures[key] = created;
    return created;
}

void ResourceManager::purgeUnused() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto it = m_models.begin(); it != m_models.end();) {
        if (it->second.expired()) {
            it = m_models.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_shaders.begin(); it != m_shaders.end();) {
        if (it->second.expired()) {
            it = m_shaders.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_textures.begin(); it != m_textures.end();) {
        if (it->second.expired()) {
            it = m_textures.erase(it);
        } else {
            ++it;
        }
    }
}

void ResourceManager::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_models.clear();
    m_shaders.clear();
    m_textures.clear();
}

std::string ResourceManager::normalizePath(const std::string& path) {
    return Path::resolve(path).string();
}

std::string ResourceManager::buildShaderKey(const std::string& vertexPath, const std::string& fragmentPath) {
    return normalizePath(vertexPath) + "|" + normalizePath(fragmentPath);
}
