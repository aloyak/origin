#include "engine/components/renderer.h"
#include "engine/components/entity.h"
#include "engine/engine.h"

#include <nlohmann/json.hpp>

RenderComponent::RenderComponent(const std::string& modelPath,
                                 const std::string& vertPath,
                                 const std::string& fragPath)
    : m_model(std::make_unique<Model>(modelPath.c_str()))
    , m_shader(std::make_unique<Shader>(vertPath.c_str(), fragPath.c_str()))
{
    m_modelPath = modelPath;
    m_vertPath = vertPath;
    m_fragPath = fragPath;
}

void RenderComponent::serialize(nlohmann::json& j) const {
    j["type"] = "RenderComponent";
    j["model"] = m_modelPath;
    j["vert"] = m_vertPath;
    j["frag"] = m_fragPath;
}

void RenderComponent::deserialize(const nlohmann::json& j) {
    std::string path = j.value("model", "");
    if (!path.empty()) {
        m_model = std::make_unique<Model>(path.c_str());
    }

    m_modelPath = j.value("model", "");
    m_vertPath = j.value("vert", "assets/shaders/vert.glsl");
    m_fragPath = j.value("frag", "assets/shaders/frag.glsl");

    if (!m_modelPath.empty()) {
        m_model = std::make_unique<Model>(m_modelPath.c_str());
        m_shader = std::make_unique<Shader>(m_vertPath.c_str(), m_fragPath.c_str());
    }
}

void RenderComponent::setTexture(const std::string& path, const std::string& type) {
    if (type == "diffuse")
        m_diffuseOverride = std::make_shared<Texture>(path);
    else if (type == "specular")
        m_specularOverride = std::make_shared<Texture>(path);
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

void RenderComponent::render(Engine& engine, const Camera& camera, const Transform& cameraTransform) {
    if (!isEnabled) return;
    bindOverrides();
    engine.render(*m_model, *m_shader, camera, cameraTransform, entity->transform);
}

