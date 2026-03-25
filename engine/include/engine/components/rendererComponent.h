#pragma once

#include "engine/components/component.h"
#include "engine/render/material.h"
#include "engine/render/model.h"

#include <string>
#include <memory>

class RenderComponent : public Component {
public:
    RenderComponent(const std::string& modelPath = "",
                    const std::string& vertPath = "assets/shaders/vert.glsl",
                    const std::string& fragPath = "assets/shaders/frag.glsl");
    
    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

    void setTexture(const std::string& path, const std::string& type = "diffuse");

    void render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) override;

    void setModelPath(const std::string& modelPath);
    void setVertPath(const std::string& vertPath);
    void setFragPath(const std::string& fragPath);

    std::string getModelPath() const { return m_modelPath; }
    std::string getVertPath() const { return m_vertPath; }
    std::string getFragPath() const { return m_fragPath; }
private:
    std::shared_ptr<Model>  m_model;
    std::unique_ptr<Material> m_material;

    std::string m_modelPath;
    std::string m_vertPath;
    std::string m_fragPath;
};