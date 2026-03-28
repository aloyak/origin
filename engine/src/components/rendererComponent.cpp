#include "engine/components/rendererComponent.h"
#include "engine/components/entity.h"
#include "engine/engine.h"
#include "engine/render/material.h"
#include "engine/core/resourceManager.h"
#include "engine/lighting/lightingManager.h"
#include "engine/utils/path.h"

#include <nlohmann/json.hpp>

RenderComponent::RenderComponent(const std::string& modelPath,
                                 const std::string& vertPath,
                                 const std::string& fragPath)
{
    m_modelPath = Path::toAssetsRelative(modelPath);
    m_vertPath = Path::toAssetsRelative(vertPath);
    m_fragPath = Path::toAssetsRelative(fragPath);

    ensureMaterial();

    if (!m_modelPath.empty()) {
        m_model = ResourceManager::instance().getModel(m_modelPath);
    }
}

void RenderComponent::serialize(nlohmann::json& j) const {
    j["type"] = "RenderComponent";
    j["model"] = m_modelPath;
    j["vert"] = m_vertPath;
    j["frag"] = m_fragPath;
    j["diffuse"] = m_diffusePath;
    j["specular"] = m_specularPath;
    j["normal"] = m_normalPath;

    if (m_material) {
        j["ambientStrength"] = m_material->getAmbientStrength();
        j["specularStrength"] = m_material->getSpecularStrength();
        j["shininess"] = m_material->getShininess();

        Vec3 baseColor = m_material->getBaseColor();
        j["baseColor"] = { baseColor.x, baseColor.y, baseColor.z };

        Vec2 uvScale = m_material->getUVScale();
        j["uvScale"] = { uvScale.x, uvScale.y };
    }
}

void RenderComponent::deserialize(const nlohmann::json& j) {
    m_modelPath = Path::toAssetsRelative(j.value("model", ""));
    m_vertPath = Path::toAssetsRelative(j.value("vert", "assets/shaders/vert.glsl"));
    m_fragPath = Path::toAssetsRelative(j.value("frag", "assets/shaders/frag.glsl"));
    m_diffusePath = Path::toAssetsRelative(j.value("diffuse", ""));
    m_specularPath = Path::toAssetsRelative(j.value("specular", ""));
    m_normalPath = Path::toAssetsRelative(j.value("normal", ""));

    ensureMaterial();

    if (!m_modelPath.empty()) {
        m_model = ResourceManager::instance().getModel(m_modelPath);
    } else {
        m_model.reset();
    }

    if (!m_diffusePath.empty()) {
        setTexture(m_diffusePath, "diffuse");
    }
    if (!m_specularPath.empty()) {
        setTexture(m_specularPath, "specular");
    }
    if (!m_normalPath.empty()) {
        setTexture(m_normalPath, "normal");
    }

    setAmbientStrength(j.value("ambientStrength", getAmbientStrength()));
    setSpecularStrength(j.value("specularStrength", getSpecularStrength()));
    setShininess(j.value("shininess", getShininess()));

    if (j.contains("baseColor") && j["baseColor"].is_array() && j["baseColor"].size() >= 3) {
        setBaseColor(Vec3(
            j["baseColor"][0].get<float>(),
            j["baseColor"][1].get<float>(),
            j["baseColor"][2].get<float>()
        ));
    }

    if (j.contains("uvScale") && j["uvScale"].is_array() && j["uvScale"].size() >= 2) {
        setUVScale(Vec2(
            j["uvScale"][0].get<float>(),
            j["uvScale"][1].get<float>()
        ));
    }
}

void RenderComponent::setModelPath(const std::string& modelPath) {
    m_modelPath = Path::toAssetsRelative(modelPath);

    if (m_modelPath.empty()) {
        m_model.reset();
        return;
    }

    ensureMaterial();
    m_model = ResourceManager::instance().getModel(m_modelPath);
}

void RenderComponent::setVertPath(const std::string& vertPath) {
    m_vertPath = Path::toAssetsRelative(vertPath);
    ensureMaterial();
}

void RenderComponent::setFragPath(const std::string& fragPath) {
    m_fragPath = Path::toAssetsRelative(fragPath);
    ensureMaterial();
}

void RenderComponent::setTexture(const std::string& path, const std::string& type) {
    ensureMaterial();

    const std::string normalizedPath = Path::toAssetsRelative(path);
    if (normalizedPath.empty()) {
        return;
    }

    if (type == "diffuse" || type == "texture_diffuse") {
        m_diffusePath = normalizedPath;
    } else if (type == "specular" || type == "texture_specular") {
        m_specularPath = normalizedPath;
    } else if (type == "normal" || type == "texture_normal") {
        m_normalPath = normalizedPath;
    }

    m_material->setTexture(type, ResourceManager::instance().getTexture(normalizedPath));
}

void RenderComponent::setDiffuseTexturePath(const std::string& path) {
    m_diffusePath = Path::toAssetsRelative(path);
    if (m_diffusePath.empty()) {
        return;
    }

    setTexture(m_diffusePath, "diffuse");
}

void RenderComponent::setSpecularTexturePath(const std::string& path) {
    m_specularPath = Path::toAssetsRelative(path);
    if (m_specularPath.empty()) {
        return;
    }

    setTexture(m_specularPath, "specular");
}

void RenderComponent::setNormalTexturePath(const std::string& path) {
    m_normalPath = Path::toAssetsRelative(path);
    if (m_normalPath.empty()) {
        return;
    }

    setTexture(m_normalPath, "normal");
}

void RenderComponent::setAmbientStrength(float strength) {
    ensureMaterial();
    m_material->setAmbientStrength(strength);
}

float RenderComponent::getAmbientStrength() const {
    return m_material ? m_material->getAmbientStrength() : 0.1f;
}

void RenderComponent::setSpecularStrength(float strength) {
    ensureMaterial();
    m_material->setSpecularStrength(strength);
}

float RenderComponent::getSpecularStrength() const {
    return m_material ? m_material->getSpecularStrength() : 1.0f;
}

void RenderComponent::setShininess(float shininess) {
    ensureMaterial();
    m_material->setShininess(shininess);
}

float RenderComponent::getShininess() const {
    return m_material ? m_material->getShininess() : 32.0f;
}

void RenderComponent::setBaseColor(const Vec3& color) {
    ensureMaterial();
    m_material->setBaseColor(color);
}

Vec3 RenderComponent::getBaseColor() const {
    return m_material ? m_material->getBaseColor() : Vec3(1.0f, 1.0f, 1.0f);
}

void RenderComponent::setUVScale(const Vec2& uvScale) {
    ensureMaterial();
    m_material->setUVScale(uvScale);
}

Vec2 RenderComponent::getUVScale() const {
    return m_material ? m_material->getUVScale() : Vec2(1.0f, 1.0f);
}

void RenderComponent::ensureMaterial() {
    auto shader = ResourceManager::instance().getShader(m_vertPath, m_fragPath);

    if (!m_material) {
        m_material = std::make_unique<Material>(shader);
        return;
    }

    m_material->setShader(shader);
}

void RenderComponent::render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) {
    if (!isEnabled) return;
    if (!m_model || !m_material || !m_material->getShaderHandle()) return;
    
    const auto& directionalLights = LightingManager::instance().getDirectionalLights();
    const auto& pointLights = LightingManager::instance().getPointLights();
    renderer.render(*m_model, *m_material, camera, cameraTransform, entity->transform, directionalLights, pointLights);
}

