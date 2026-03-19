#pragma once

#include "engine/transform.h"

#include <memory>

class Window;
class Shader;
class Camera;
class Model;

class Renderer {
public:
    explicit Renderer(Window& window);
    ~Renderer();

    // Call once to create the offscreen FBO and fullscreen quad.
    // Required before setPixelArt() or resizeRenderTarget().
    void setupRenderTarget(unsigned int width, unsigned int height);

    // Resize the virtual resolution without recreating the FBO.
    void resizeRenderTarget(unsigned int width, unsigned int height);

    // Toggle pixel-art colour-depth banding. Requires setupRenderTarget().
    void setPixelArt(bool enabled, int colorDepth = 32);

    // Toggle vertex snapping (passed through to shaders via render()).
    void setVertexSnap(bool enabled, float intensity = 40.0f) {
        m_vertexSnap   = enabled;
        m_snapIntensity = intensity;
    }

    // Bind the FBO (or default framebuffer) and clear.
    void beginFrame();

    // Blit the FBO to the default framebuffer with the post-process pass.
    void resolveFrame();

    // Draw a single model. Called per-entity from updateScene().
    void render(Model& model, Shader& shader,
                const Camera& camera,
                const Transform& cameraTransform,
                const Transform& modelTransform);

    // Returns the FBO colour texture (used by ImGui viewport in the sandbox).
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
};