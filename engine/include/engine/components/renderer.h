#pragma once

#include "engine/components/component.h"
#include "engine/model.h"
#include "engine/shader.h"

#include <string>
#include <memory>

class RenderComponent : public Component {
public:
    RenderComponent(const std::string& modelPath,
                    const std::string& vertPath,
                    const std::string& fragPath);

    Shader& getShader() { return *m_shader; }

    void render(Engine& engine, const Camera& camera, const Transform& cameraTransform) override;

private:
    std::unique_ptr<Model>  m_model;
    std::unique_ptr<Shader> m_shader;
};