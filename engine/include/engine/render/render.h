#pragma once

#include "engine/core/transform.h"

#include <memory>
#include <vector>

class Window;
class Shader;
class Camera;
class Model;
class Material;
class DirectionalLight;

class Renderer {
public:
    explicit Renderer(Window& window);
    ~Renderer();

    void setupRenderTarget(unsigned int width, unsigned int height);
    void resizeRenderTarget(unsigned int width, unsigned int height);

    void setPixelArt(bool enabled, int colorDepth = 32);

    void setLightingEnabled(bool enabled) { m_lightingEnabled = enabled; }
    bool isLightingEnabled() const { return m_lightingEnabled; }

    void setMinimumAmbientLight(float value) { m_minAmbientLight = value; }
    float getMinimumAmbientLight() const { return m_minAmbientLight; }

    void setVertexSnap(bool enabled, float intensity = 40.0f) {
        m_vertexSnap   = enabled;
        m_snapIntensity = intensity;
    }

    void beginFrame();

    void resolveFrame();

    // Draw a single model. Called per-entity from updateScene().
    void render(Model& model, Material& material,
                const Camera& camera,
                const Transform& cameraTransform,
                const Transform& modelTransform,
                const std::vector<DirectionalLight>& lights);

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
};