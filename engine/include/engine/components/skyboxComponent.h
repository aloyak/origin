#pragma once

#include "engine/components/component.h"
#include "engine/render/shader.h"

#include <vector>
#include <string>
#include <memory>

#include <nlohmann/json_fwd.hpp>

class SkyboxComponent : public Component {
public:
    SkyboxComponent(const std::vector<std::string>& faces = {});

    std::unique_ptr<Component> clone() const override;

    void render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) override;

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

    std::vector<std::string> getFaces() const { return m_facePaths; }
    bool setFaces(const std::vector<std::string>& faces);
    bool setFacePath(size_t index, const std::string& path);

    void setRotation(float rotation) { m_rotation = rotation; }
    float getRotation() const { return m_rotation;}
private:
    unsigned int m_cubemapID = 0;
    unsigned int m_skyboxVAO = 0;
    unsigned int m_skyboxVBO = 0;
    std::unique_ptr<Shader> m_shader;
    std::vector<std::string> m_facePaths;

    float m_rotation = 0.0f; 

    void loadCubemap(const std::vector<std::string>& faces);
    void setupMesh();
};