#pragma once

#include "engine/core/transform.h"
#include "engine/render/postprocess/postProcessor.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

class Window;
class Shader;
class Camera;
class Model;
class Material;
class DirectionalLight;
class PointLight;

// plain world-space ray returned by pickRay()
// Origin^(TM) is the camera position, direction is unit length
struct Ray {
    Vec3 origin;
    Vec3 direction;  // normalized
};

struct RenderSettings {
    bool  pixelArtEnabled = false;
    int   colorDepth      = 32;
 
    bool  vertexSnap    = false;
    float snapIntensity = 40.0f;
 
    bool  lightingEnabled  = true;
    float minAmbientLight  = 0.05f;
 
    float gamma                  = 2.2f;
    bool  gammaCorrectionEnabled = true;
    float exposure                = 1.0f;
};

class Renderer {
public:
    explicit Renderer(Window& window);
    ~Renderer();

    void setupRenderTarget(unsigned int width, unsigned int height);
    void resizeRenderTarget(unsigned int width, unsigned int height);

    void setPixelArt(bool enabled, int colorDepth = 32);
    bool isPixelArtEnabled() const { return m_settings.pixelArtEnabled; }

    void setLightingEnabled(bool enabled) { m_settings.lightingEnabled = enabled; }
    bool isLightingEnabled() const { return m_settings.lightingEnabled; }

    void setMinimumAmbientLight(float value);
    float getMinimumAmbientLight() const { return m_settings.minAmbientLight; }
    void setAmbientLightChangedCallback(std::function<void(float)> callback) {
        m_onMinimumAmbientLightChanged = std::move(callback);
    }

    void setVertexSnap(bool enabled, float intensity = 40.0f) {
        m_settings.vertexSnap = enabled;
        m_settings.snapIntensity = intensity;
    }
    bool isVertexSnapEnabled() const { return m_settings.vertexSnap; }

    void setGammaCorrection(float gamma) { m_settings.gamma = std::max(0.01f, gamma); }
    float getGammaCorrection() const { return m_settings.gamma; }
    void setGammaCorrectionEnabled(bool enabled) { m_settings.gammaCorrectionEnabled = enabled; }
    bool isGammaCorrectionEnabled() const { return m_settings.gammaCorrectionEnabled; }
    void setExposure(float exposure) { m_settings.exposure = std::max(0.0f, exposure); }
    float getExposure() const { return m_settings.exposure; }

    // Direct read-only access to the full settings bag, e.g. for an editor panel.
    const RenderSettings& getSettings() const { return m_settings; }

    void beginFrame();
    void resolveFrame();

    void render(Model& model, Material& material,
                const Camera& camera,
                const Transform& cameraTransform,
                const Transform& modelTransform,
                const std::vector<DirectionalLight>& directionalLights,
                const std::vector<PointLight>& pointLights);

    void renderInstanced(Model& model, Material& material,
                         const Camera& camera,
                         const Transform& cameraTransform,
                         const std::vector<Transform>& modelTransforms,
                         const std::vector<DirectionalLight>& directionalLights,
                         const std::vector<PointLight>& pointLights);

    unsigned int getRenderTexture() const;
    unsigned int getSceneDepthTexture() const { return m_sceneDepthTexture; }

    void drawLine(const Vec3& start, const Vec3& end,
                  const Camera& camera, const Transform& cameraTransform,
                  const Vec3& color, float thickness = 2.0f, bool ignoreDepth = true);

    Ray pickRay(float mouseX, float mouseY,
                const Camera& camera,
                const Transform& cameraTransform) const;

    // Custom post-processing chain
    PostProcessor& addPostProcessor(const std::string& vertPath, const std::string& fragPath);
    void removePostProcessor(size_t index);
    void removePostProcessor(PostProcessor* processor);
    void setPostProcessorLast(PostProcessor* processor);
    PostProcessor& getPostProcessor(size_t index);
    size_t getPostProcessorCount() const { return m_postProcessors.size(); }

private:
    void updateOutputToScreenFlag();

    Window& m_window;

    unsigned int m_virtualWidth  = 0;
    unsigned int m_virtualHeight = 0;

    unsigned int m_sceneFBO          = 0;
    unsigned int m_sceneTexture      = 0;
    unsigned int m_sceneDepthTexture = 0;

    std::vector<std::unique_ptr<PostProcessor>> m_postProcessors;

    unsigned int m_lineVAO    = 0;
    unsigned int m_lineVBO    = 0;
    std::unique_ptr<Shader> m_lineShader;

    RenderSettings m_settings;
    std::function<void(float)> m_onMinimumAmbientLightChanged;
};