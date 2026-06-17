#pragma once

#include "engine/components/component.h"
#include "engine/render/material.h"
#include "engine/render/model.h"

#include <string>
#include <memory>

class RenderComponent : public Component {
public:
    RenderComponent(const std::string& modelPath = "",
                    const std::string& vertPath = "assets/shaders/builtin/vert.glsl",
                    const std::string& fragPath = "assets/shaders/builtin/frag.glsl");
    
    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

    void setTexture(const std::string& path, const std::string& type = "diffuse");

    void setDiffuseTexturePath(const std::string& path);
    void setSpecularTexturePath(const std::string& path);
    void setNormalTexturePath(const std::string& path);
    void setMetallicTexturePath(const std::string& path);
    std::string getDiffuseTexturePath() const { return m_diffusePath; }
    std::string getSpecularTexturePath() const { return m_specularPath; }
    std::string getNormalTexturePath() const { return m_normalPath; }
    std::string getMetallicTexturePath() const { return m_metallicPath; }

    void setPaintableTexture(std::shared_ptr<Texture> texture);
    void clearPaintableTexture();
    bool hasPaintableTexture() const { return m_paintableTexture != nullptr; }

    void setAmbientStrength(float strength);
    float getAmbientStrength() const;
    void setSpecularStrength(float strength);
    float getSpecularStrength() const;
    void setShininess(float shininess);
    float getShininess() const;
    void setBaseColor(const Vec3& color);
    Vec3 getBaseColor() const;
    void setUVScale(const Vec2& uvScale);
    Vec2 getUVScale() const;

    std::unique_ptr<Component> clone() const override;

    void render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) override;

    void setModelPath(const std::string& modelPath);
    void setVertPath(const std::string& vertPath);
    void setFragPath(const std::string& fragPath);

    std::shared_ptr<Model> getModel() const { return m_model; }
    std::string getModelPath() const { return m_modelPath; }
    std::string getVertPath() const { return m_vertPath; }
    std::string getFragPath() const { return m_fragPath; }
private:
    void ensureMaterial();

    std::shared_ptr<Model>  m_model;
    std::unique_ptr<Material> m_material;

    
    std::shared_ptr<Texture> m_paintableTexture;

    std::string m_modelPath;
    std::string m_vertPath;
    std::string m_fragPath;
    std::string m_diffusePath;
    std::string m_specularPath;
    std::string m_normalPath;
    std::string m_metallicPath;
};