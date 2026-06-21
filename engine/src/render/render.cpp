#include "engine/render/render.h"
#include "engine/render/material.h"
#include "engine/render/shader.h"
#include "engine/render/model.h"
#include "engine/render/camera.h"
#include "engine/lighting/directionalLight.h"
#include "engine/lighting/pointLight.h"

#include "engine/core/window.h"
#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

Renderer::Renderer(Window& window)
    : m_window(window)
{}

Renderer::~Renderer() {
    if (m_lineVAO) {
        glDeleteVertexArrays(1, &m_lineVAO);
        glDeleteBuffers(1,      &m_lineVBO);
    }
    if (m_sceneFBO != 0) {
        glDeleteFramebuffers(1,  &m_sceneFBO);
        glDeleteTextures(1,      &m_sceneTexture);
        glDeleteTextures(1,      &m_sceneDepthTexture);
    }
}

void Renderer::setupRenderTarget(unsigned int width, unsigned int height) {
    m_virtualWidth  = width;
    m_virtualHeight = height;

    if (m_sceneFBO != 0) {
        glDeleteFramebuffers(1,  &m_sceneFBO);
        glDeleteTextures(1,      &m_sceneTexture);
        glDeleteTextures(1,      &m_sceneDepthTexture);
    }

    glGenFramebuffers(1, &m_sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);

    glGenTextures(1, &m_sceneTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexture, 0);

    glGenTextures(1, &m_sceneDepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_sceneDepthTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Logger::error("Renderer scene framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_postProcessors.clear();
    auto defaultPass = std::make_unique<PostProcessor>(Vec2((float)width, (float)height), false);
    defaultPass->setShader(
        Path::resolve("assets/shaders/builtin/post_vert.glsl").string(),
        Path::resolve("assets/shaders/builtin/post_frag.glsl").string()
    );
    m_postProcessors.push_back(std::move(defaultPass));
    updateOutputToScreenFlag();

    if (m_lineVAO) {
        glDeleteVertexArrays(1, &m_lineVAO);
        glDeleteBuffers(1,      &m_lineVBO);
        m_lineVAO = m_lineVBO = 0;
    }

    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 7 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));

    if (!m_lineShader) {
        m_lineShader = std::make_unique<Shader>(
            Path::resolve("assets/shaders/builtin/line_vert.glsl").string(),
            Path::resolve("assets/shaders/builtin/line_frag.glsl").string()
        );
    }
}

void Renderer::resizeRenderTarget(unsigned int width, unsigned int height) {
    if (m_postProcessors.empty()) {
        Logger::warn("resizeRenderTarget called before setupRenderTarget — ignoring.");
        return;
    }
    if (width == m_virtualWidth && height == m_virtualHeight) return;

    m_virtualWidth  = width;
    m_virtualHeight = height;

    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, m_sceneDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    for (auto& pass : m_postProcessors) {
        if (!pass->isOutputToScreen()) {
            pass->resize(width, height);
        }
    }
}

void Renderer::setPixelArt(bool enabled, int colorDepth) {
    if (enabled && m_postProcessors.empty()) {
        Logger::warn("setPixelArt() called before setupRenderTarget!");
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

unsigned int Renderer::getRenderTexture() const {
    return m_sceneTexture;
}

PostProcessor& Renderer::addPostProcessor(const std::string& vertPath, const std::string& fragPath) {
    auto pass = std::make_unique<PostProcessor>(Vec2((float)m_virtualWidth, (float)m_virtualHeight));
    pass->setShader(vertPath, fragPath);
    m_postProcessors.push_back(std::move(pass));
    updateOutputToScreenFlag();
    return *m_postProcessors.back();
}

void Renderer::removePostProcessor(PostProcessor* processor) {
    if (!processor) return;

    auto it = std::find_if(m_postProcessors.begin(), m_postProcessors.end(),
        [processor](const std::unique_ptr<PostProcessor>& p) {
            return p.get() == processor;
        });

    if (it != m_postProcessors.end()) {
        if (it == m_postProcessors.begin()) {
            Logger::warn("removePostProcessor: The default pass at index 0 cannot be removed.");
            return;
        }
        m_postProcessors.erase(it);
        updateOutputToScreenFlag();
    }
}

void Renderer::removePostProcessor(size_t index) {
    if (index == 0 || index >= m_postProcessors.size()) {
        Logger::warn("removePostProcessor: invalid index (the default pass at 0 cannot be removed).");
        return;
    }
    m_postProcessors.erase(m_postProcessors.begin() + index);
    updateOutputToScreenFlag();
}

PostProcessor& Renderer::getPostProcessor(size_t index) {
    return *m_postProcessors.at(index);
}

void Renderer::updateOutputToScreenFlag() {
    for (size_t i = 0; i < m_postProcessors.size(); ++i) {
        m_postProcessors[i]->setOutputToScreen(i == m_postProcessors.size() - 1);
    }
}

void Renderer::beginFrame() {
    if (m_sceneFBO != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
        glViewport(0, 0, (int)m_virtualWidth, (int)m_virtualHeight);
    } else {
        Vec2 size = m_window.getSize();
        glViewport(0, 0, (int)size.x, (int)size.y);
    }

    glDepthFunc(GL_GREATER);
    glClearDepth(0.0f);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f); // default base color bright
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::resolveFrame() {
    if (m_postProcessors.empty()) return;
    
    Vec2 windowSize = m_window.getSize();

    PostProcessor& defaultPass = *m_postProcessors.front();
    defaultPass.setBool("u_GammaEnabled", m_gammaCorrectionEnabled);
    defaultPass.setFloat("u_Gamma", m_gamma);
    defaultPass.setFloat("u_Exposure", m_exposure);

    float levels = 0.0f;
    if (m_pixelArtEnabled) {
        levels = 255.0f;
        if      (m_colorDepth <= 4)  levels = 4.0f;
        else if (m_colorDepth <= 8)  levels = 8.0f;
        else if (m_colorDepth <= 16) levels = 32.0f;
        else if (m_colorDepth >= 32) levels = 0.0f;
    }
    defaultPass.setFloat("colorLevels", levels);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    unsigned int inputTexture = m_sceneTexture;

    for (size_t i = 0; i < m_postProcessors.size(); ++i) {
        PostProcessor& pass = *m_postProcessors[i];

        if (pass.isOutputToScreen()) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        pass.process(inputTexture, windowSize, m_sceneDepthTexture);
        inputTexture = pass.getTexture();
    }
}

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

    Mat4 modelMat4;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            modelMat4[col][row] = modelMat[col][row];
    shader.setMat4("u_Model", modelMat4);

    Mat4 viewMat4;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            viewMat4[col][row] = (*(glm::mat4*)camera.getViewMatrix(cameraTransform))[col][row];
    shader.setMat4("u_View", viewMat4);

    Mat4 projMat4;
    camera.getProjectionMatrix(projMat4);
    shader.setMat4("u_Projection", projMat4);

    shader.setBool ("u_VertexSnap",    m_vertexSnap);
    shader.setFloat("u_SnapIntensity", m_snapIntensity);
    shader.setBool ("u_LightingEnabled", m_lightingEnabled);
    shader.setFloat("u_MinAmbientLight", m_minAmbientLight);

    shader.setVec3("u_ViewPos", cameraTransform.position);

    model.draw(material, directionalLights, pointLights);
}

void Renderer::renderInstanced(Model& model, Material& material,
                               const Camera& camera,
                               const Transform& cameraTransform,
                               const std::vector<Transform>& modelTransforms,
                               const std::vector<DirectionalLight>& directionalLights,
                               const std::vector<PointLight>& pointLights)
{
    if (modelTransforms.empty()) return;

    Shader& shader = material.getShader();
    shader.use();

    Mat4 viewMat4Instanced;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            viewMat4Instanced[col][row] = (*(glm::mat4*)camera.getViewMatrix(cameraTransform))[col][row];
    shader.setMat4("u_View", viewMat4Instanced);

    Mat4 projMat4Instanced;
    camera.getProjectionMatrix(projMat4Instanced);
    shader.setMat4("u_Projection", projMat4Instanced);

    shader.setBool ("u_VertexSnap",      m_vertexSnap);
    shader.setFloat("u_SnapIntensity",   m_snapIntensity);
    shader.setBool ("u_LightingEnabled", m_lightingEnabled);
    shader.setFloat("u_MinAmbientLight", m_minAmbientLight);
    shader.setVec3 ("u_ViewPos",         cameraTransform.position);

    std::vector<glm::mat4> modelMatrices;
    modelMatrices.reserve(modelTransforms.size());

    for (const auto& t : modelTransforms) {
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, glm::vec3(t.position.x, t.position.y, t.position.z));
        modelMat = glm::rotate(modelMat, glm::radians(t.rotation.z), glm::vec3(0, 0, 1));
        modelMat = glm::rotate(modelMat, glm::radians(t.rotation.y), glm::vec3(0, 1, 0));
        modelMat = glm::rotate(modelMat, glm::radians(t.rotation.x), glm::vec3(1, 0, 0));
        modelMat = glm::scale(modelMat, glm::vec3(t.scale.x, t.scale.y, t.scale.z));
        modelMatrices.push_back(modelMat);
    }

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data(), GL_STREAM_DRAW);

    model.drawInstanced(material, directionalLights, pointLights, instanceVBO, modelTransforms.size());

    glDeleteBuffers(1, &instanceVBO);
}

void Renderer::drawLine(const Vec3& start, const Vec3& end,
                        const Camera& camera, const Transform& cameraTransform,
                        const Vec3& color, float thickness, bool ignoreDepth)
{
    if (!m_lineShader) return;

    if (ignoreDepth) {
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
    }

    float vertices[] = {
        start.x, start.y, start.z,  end.x,   end.y,   end.z,   1.0f,
        start.x, start.y, start.z,  end.x,   end.y,   end.z,  -1.0f,
        end.x,   end.y,   end.z,    start.x, start.y, start.z, 1.0f,

        start.x, start.y, start.z,  end.x,   end.y,   end.z,  -1.0f,
        end.x,   end.y,   end.z,    start.x, start.y, start.z, 1.0f,
        end.x,   end.y,   end.z,    start.x, start.y, start.z, -1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    m_lineShader->use();
    Mat4 viewMat4Line;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            viewMat4Line[col][row] = (*(glm::mat4*)camera.getViewMatrix(cameraTransform))[col][row];
    m_lineShader->setMat4("u_View", viewMat4Line);

    Mat4 projMat4Line;
    camera.getProjectionMatrix(projMat4Line);
    m_lineShader->setMat4("u_Projection", projMat4Line);
    m_lineShader->setVec3("u_Color",      Vec3(color.x, color.y, color.z));
    m_lineShader->setFloat("u_Thickness", thickness);

    Vec2 size = m_window.getSize();
    m_lineShader->setVec2("u_ScreenSize", Vec2(size.x, size.y));

    glBindVertexArray(m_lineVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

Ray Renderer::pickRay(float mouseX, float mouseY,
                      const Camera& camera,
                      const Transform& cameraTransform) const
{
    float vpW, vpH;
    if (!m_postProcessors.empty() && m_virtualWidth > 0 && m_virtualHeight > 0) {
        Vec2 windowSize = m_window.getSize();
        
        mouseX = mouseX * (static_cast<float>(m_virtualWidth)  / windowSize.x);
        mouseY = mouseY * (static_cast<float>(m_virtualHeight) / windowSize.y);
        vpW = static_cast<float>(m_virtualWidth);
        vpH = static_cast<float>(m_virtualHeight);
    } else {
        Vec2 windowSize = m_window.getSize();
        vpW = windowSize.x;
        vpH = windowSize.y;
    }

    const float ndcX =  (2.0f * mouseX / vpW) - 1.0f;
    const float ndcY = -((2.0f * mouseY / vpH) - 1.0f);

    const glm::mat4 view       = glm::make_mat4(reinterpret_cast<const float*>(camera.getViewMatrix(cameraTransform)));
    const glm::mat4 projection = glm::make_mat4(reinterpret_cast<const float*>(camera.getProjectionMatrix()));
    const glm::mat4 invVP      = glm::inverse(projection * view);

    // reverse-z: far plane is z = -1, near plane is z = 1
    const glm::vec4 nearNDC(ndcX, ndcY,  1.0f, 1.0f);
    const glm::vec4 farNDC (ndcX, ndcY, -1.0f, 1.0f);

    glm::vec4 nearWorld = invVP * nearNDC;
    glm::vec4 farWorld  = invVP * farNDC;

    nearWorld /= nearWorld.w;
    farWorld  /= farWorld.w;

    const glm::vec3 dir = glm::normalize(glm::vec3(farWorld) - glm::vec3(nearWorld));

    return Ray {
        Vec3(nearWorld.x, nearWorld.y, nearWorld.z),
        Vec3(dir.x,       dir.y,       dir.z)
    };
}