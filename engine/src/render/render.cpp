#include "engine/render/render.h"
#include "engine/render/material.h"
#include "engine/render/shader.h"
#include "engine/render/model.h"
#include "engine/render/camera.h"
#include "engine/lighting/directionalLight.h"
#include "engine/lighting/pointLight.h"

#include "engine/core/window.h"
#include "engine/utils/path.h"

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <cmath>

Renderer::Renderer(Window& window)
    : m_window(window)
{}

Renderer::~Renderer() {
    if (m_fbo) {
        glDeleteFramebuffers(1,  &m_fbo);
        glDeleteTextures(1,      &m_fboTexture);
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteVertexArrays(1,  &m_quadVAO);
        glDeleteBuffers(1,       &m_quadVBO);
    }
}

// Render target
void Renderer::setupRenderTarget(unsigned int width, unsigned int height) {
    m_virtualWidth  = width;
    m_virtualHeight = height;

    if (m_fbo != 0) {
        glDeleteFramebuffers(1,  &m_fbo);
        glDeleteTextures(1,      &m_fboTexture);
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteVertexArrays(1,  &m_quadVAO);
        glDeleteBuffers(1,       &m_quadVBO);
        m_fbo = m_fboTexture = m_rbo = m_quadVAO = m_quadVBO = 0;
    }

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_fboTexture);
    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);

    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    if (!m_screenShader) {
        m_screenShader = std::make_unique<Shader>(
            Path::resolve("assets/shaders/post_vert.glsl").string(),
            Path::resolve("assets/shaders/post_frag.glsl").string()
        );
    }
}

void Renderer::resizeRenderTarget(unsigned int width, unsigned int height) {
    if (m_fbo == 0) {
        spdlog::warn("resizeRenderTarget called before setupRenderTarget — ignoring.");
        return;
    }
    if (width == m_virtualWidth && height == m_virtualHeight) return;

    m_virtualWidth  = width;
    m_virtualHeight = height;

    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}

void Renderer::setPixelArt(bool enabled, int colorDepth) {
    if (enabled && m_fbo == 0) {
        spdlog::warn("setPixelArt() called before setupRenderTarget!");
    }
    m_pixelArtEnabled = enabled;
    m_colorDepth      = colorDepth;
}

void Renderer::setMinimumAmbientLight(float value) {
    if (std::abs(m_minAmbientLight - value) <= 0.0001f) {
        return;
    }

    m_minAmbientLight = value;
    if (m_onMinimumAmbientLightChanged) {
        m_onMinimumAmbientLightChanged(value);
    }
}

// Frame pipeline: begin, reslolve, end (called from engine::run)
void Renderer::beginFrame() {
    if (m_fbo != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_virtualWidth, m_virtualHeight);
    } else {
        Vec2 size = m_window.getSize();
        glViewport(0, 0, (int)size.x, (int)size.y);
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::resolveFrame() {
    if (m_fbo == 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Vec2 size = m_window.getSize();
    glViewport(0, 0, (int)size.x, (int)size.y);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screenShader->use();
    m_screenShader->setBool("u_GammaEnabled", m_gammaCorrectionEnabled);
    m_screenShader->setFloat("u_Gamma", m_gamma);
    m_screenShader->setFloat("u_Exposure", m_exposure);

    float levels = 0.0f;
    if (m_pixelArtEnabled) {
        levels = 255.0f;
        if      (m_colorDepth <= 4)  levels = 4.0f;
        else if (m_colorDepth <= 8)  levels = 8.0f;
        else if (m_colorDepth <= 16) levels = 32.0f;
        else if (m_colorDepth >= 32) levels = 0.0f;
    }
    m_screenShader->setFloat("colorLevels", levels);

    glBindVertexArray(m_quadVAO);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    m_screenShader->setInt("screenTexture", 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

// Rendering
// This would need to be a lot more complex when adding lighting
void Renderer::render(Model& model, Material& material,
                      const Camera& camera,
                      const Transform& cameraTransform,
                      const Transform& modelTransform,
                      const std::vector<DirectionalLight>& directionalLights,
                      const std::vector<PointLight>& pointLights)
{
    Shader& shader = material.getShader();
    shader.use();

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(modelTransform.position.x,
                                                   modelTransform.position.y,
                                                   modelTransform.position.z));
    modelMat = glm::rotate(modelMat, glm::radians(modelTransform.rotation.z), glm::vec3(0, 0, 1));
    modelMat = glm::rotate(modelMat, glm::radians(modelTransform.rotation.y), glm::vec3(0, 1, 0));
    modelMat = glm::rotate(modelMat, glm::radians(modelTransform.rotation.x), glm::vec3(1, 0, 0));
    modelMat = glm::scale(modelMat, glm::vec3(modelTransform.scale.x,
                                               modelTransform.scale.y,
                                               modelTransform.scale.z));

    shader.setMat4("u_Model",      &modelMat);
    shader.setMat4("u_View",       camera.getViewMatrix(cameraTransform));
    shader.setMat4("u_Projection", camera.getProjectionMatrix());

    shader.setBool ("u_VertexSnap",    m_vertexSnap);
    shader.setFloat("u_SnapIntensity", m_snapIntensity);
    shader.setBool ("u_LightingEnabled", m_lightingEnabled);
    shader.setFloat("u_MinAmbientLight", m_minAmbientLight);
    
    // Bind camera position for lighting calculations
    shader.setVec3("u_ViewPos", cameraTransform.position);

    model.draw(material, directionalLights, pointLights);
}