#pragma once

#include "engine/components/component.h"
#include "engine/shader.h"

#include <vector>
#include <string>
#include <memory>

#include <nlohmann/json_fwd.hpp>

class SkyboxComponent : public Component {
public:
    SkyboxComponent(const std::vector<std::string>& faces);

    void render(Engine& engine, const Camera& camera, const Transform& cameraTransform) override;

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

private:
    unsigned int m_cubemapID;
    unsigned int m_skyboxVAO, m_skyboxVBO;
    std::unique_ptr<Shader> m_shader;
    std::vector<std::string> m_facePaths;

    void loadCubemap(const std::vector<std::string>& faces);
    void setupMesh();
};