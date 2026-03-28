#pragma once

#include "engine/core/transform.h"

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

class Renderer {
public:
    explicit Renderer(Window& window);
    ~Renderer();

    void setupRenderTarget(unsigned int width, unsigned int height);
    void resizeRenderTarget(unsigned int width, unsigned int height);

    void setPixelArt(bool enabled, int colorDepth = 32);
    bool isPixelArtEnabled() const { return m_pixelArtEnabled; }

    void setLightingEnabled(bool enabled) { m_lightingEnabled = enabled; }
    bool isLightingEnabled() const { return m_lightingEnabled; }

    void setMinimumAmbientLight(float value);
    float getMinimumAmbientLight() const { return m_minAmbientLight; }
    void setAmbientLightChangedCallback(std::function<void(float)> callback) {
        m_onMinimumAmbientLightChanged = std::move(callback);
    }

    void setVertexSnap(bool enabled, float intensity = 40.0f) {
        m_vertexSnap   = enabled;
        m_snapIntensity = intensity;
    }
    bool isVertexSnapEnabled() const { return m_vertexSnap; }

    void setGammaCorrection(float gamma) { m_gamma = std::max(0.01f, gamma); }
    float getGammaCorrection() const { return m_gamma; }
    void setGammaCorrectionEnabled(bool enabled) { m_gammaCorrectionEnabled = enabled; }
    bool isGammaCorrectionEnabled() const { return m_gammaCorrectionEnabled; }
    void setExposure(float exposure) { m_exposure = std::max(0.0f, exposure); }
    float getExposure() const { return m_exposure; }

    void beginFrame();
    void resolveFrame();

    void render(Model& model, Material& material,
                const Camera& camera,
                const Transform& cameraTransform,
                const Transform& modelTransform,
                const std::vector<DirectionalLight>& directionalLights,
                const std::vector<PointLight>& pointLights);

    unsigned int getRenderTexture() const { return m_fboTexture; }

private:
    Window& m_window;   // non-owning reference

    unsigned int m_fbo        = 0;
    unsigned int m_fboTexture = 0;
    unsigned int m_rbo        = 0;
    unsigned int m_quadVAO    = 0;
    unsigned int m_quadVBO    = 0;

    unsigned int m_virtualWidth  = 0;
    unsigned int m_virtualHeight = 0;

    bool  m_pixelArtEnabled = false;
    int   m_colorDepth      = 32;
    std::unique_ptr<Shader> m_screenShader;

    bool  m_vertexSnap    = false;
    float m_snapIntensity = 40.0f;

    bool  m_lightingEnabled = true;
    float m_minAmbientLight = 0.05f;
    std::function<void(float)> m_onMinimumAmbientLightChanged;

    // Gamma correction
    float m_gamma = 2.2f;
    bool m_gammaCorrectionEnabled = true;
    float m_exposure = 1.0f;
};