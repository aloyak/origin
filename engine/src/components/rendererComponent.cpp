#include "engine/components/rendererComponent.h"
#include "engine/components/entity.h"
#include "engine/engine.h"
#include "engine/render/material.h"
#include "engine/render/resourceManager.h"
#include "engine/lighting/lightingManager.h"

#include <nlohmann/json.hpp>

RenderComponent::RenderComponent(const std::string& modelPath,
                                 const std::string& vertPath,
                                 const std::string& fragPath)
{
    m_modelPath = modelPath;
    m_vertPath = vertPath;
    m_fragPath = fragPath;

    m_model = ResourceManager::instance().getModel(m_modelPath);
    auto shader = ResourceManager::instance().getShader(m_vertPath, m_fragPath);
    m_material = std::make_unique<Material>(shader);
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
        auto shader = ResourceManager::instance().getShader(m_vertPath, m_fragPath);
        m_material = std::make_unique<Material>(shader);
    }
}

void RenderComponent::setTexture(const std::string& path, const std::string& type) {
    if (!m_material) {
        return;
    }

    m_material->setTexture(type, ResourceManager::instance().getTexture(path));
}

void RenderComponent::render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) {
    if (!isEnabled) return;
    if (!m_model || !m_material || !m_material->getShaderHandle()) return;
    
    const auto& lights = LightingManager::instance().getDirectionalLights();
    renderer.render(*m_model, *m_material, camera, cameraTransform, entity->transform, lights);
}

