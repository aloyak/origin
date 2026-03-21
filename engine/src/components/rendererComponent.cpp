#include "engine/components/rendererComponent.h"
#include "engine/components/entity.h"
#include "engine/engine.h"
#include "engine/render/resourceManager.h"

#include <nlohmann/json.hpp>

RenderComponent::RenderComponent(const std::string& modelPath,
                                 const std::string& vertPath,
                                 const std::string& fragPath)
{
    m_modelPath = modelPath;
    m_vertPath = vertPath;
    m_fragPath = fragPath;

    m_model = ResourceManager::instance().getModel(m_modelPath);
    m_shader = ResourceManager::instance().getShader(m_vertPath, m_fragPath);
}

void RenderComponent::serialize(nlohmann::json& j) const {
    j["type"] = "RenderComponent";
    j["model"] = m_modelPath;
    j["vert"] = m_vertPath;
    j["frag"] = m_fragPath;
}

void RenderComponent::deserialize(const nlohmann::json& j) {
    m_modelPath = j.value("model", "");
    m_vertPath = j.value("vert", "assets/shaders/vert.glsl");
    m_fragPath = j.value("frag", "assets/shaders/frag.glsl");

    if (!m_modelPath.empty()) {
        m_model = ResourceManager::instance().getModel(m_modelPath);
        m_shader = ResourceManager::instance().getShader(m_vertPath, m_fragPath);
    }
}

void RenderComponent::setTexture(const std::string& path, const std::string& type) {
    if (type == "diffuse")
        m_diffuseOverride = ResourceManager::instance().getTexture(path);
    else if (type == "specular")
        m_specularOverride = ResourceManager::instance().getTexture(path);
}

void RenderComponent::bindOverrides() const {
    if (m_diffuseOverride) {
        m_diffuseOverride->bind(0);
        m_shader->setInt("material.texture_diffuse1", 0);
    }
    if (m_specularOverride) {
        m_specularOverride->bind(1);
        m_shader->setInt("material.texture_specular1", 1);
    }
}

void RenderComponent::render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) {
    if (!isEnabled) return;
    if (!m_model || !m_shader) return;
    bindOverrides();
    renderer.render(*m_model, *m_shader, camera, cameraTransform, entity->transform);
}

