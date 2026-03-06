#include "engine/components/renderer.h"
#include "engine/components/entity.h"
#include "engine/engine.h"

RenderComponent::RenderComponent(const std::string& modelPath,
                                 const std::string& vertPath,
                                 const std::string& fragPath)
    : m_model(std::make_unique<Model>(modelPath.c_str()))
    , m_shader(std::make_unique<Shader>(vertPath.c_str(), fragPath.c_str()))
{
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

