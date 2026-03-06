#pragma once

#include "engine/components/component.h"
#include "engine/model.h"
#include "engine/shader.h"

#include <string>
#include <memory>

class RenderComponent : public Component {
public:
    RenderComponent(const std::string& modelPath,
                    const std::string& vertPath = "assets/shaders/vert.glsl",
                    const std::string& fragPath = "assets/shaders/frag.glsl");

    Shader& getShader() { return *m_shader; }

    void setTexture(const std::string& path, const std::string& type = "diffuse");

    void render(Engine& engine, const Camera& camera, const Transform& cameraTransform) override;

private:
    std::unique_ptr<Model>  m_model;
    std::unique_ptr<Shader> m_shader;

    std::shared_ptr<Texture> m_diffuseOverride;
    std::shared_ptr<Texture> m_specularOverride;

    void bindOverrides() const;
};