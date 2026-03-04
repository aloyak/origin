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

void RenderComponent::render(Engine& engine, const Camera& camera, const Transform& cameraTransform) {
    engine.render(*m_model, *m_shader, camera, cameraTransform, entity->transform);
}