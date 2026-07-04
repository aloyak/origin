#include "engine/render/postprocess/postProcessor.h"
#include "engine/render/shader.h"
#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

PostProcessor::PostProcessor(Vec2 size, bool needsDepth)
    : m_size(size)
    , m_needsDepth(needsDepth)
{
    createFramebuffer();
    createQuad();
}

PostProcessor::~PostProcessor() {
    destroyFramebuffer();
    destroyQuad();
}

void PostProcessor::createFramebuffer() {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_fboTexture);
    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)m_size.x, (int)m_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);

    if (m_needsDepth) {
        glGenRenderbuffers(1, &m_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)m_size.x, (int)m_size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Logger::error("PostProcessor framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::destroyFramebuffer() {
    if (m_fbo == 0) return;

    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1,    &m_fboTexture);
    if (m_needsDepth) {
        glDeleteRenderbuffers(1, &m_rbo);
    }
    m_fbo = m_fboTexture = m_rbo = 0;
}

void PostProcessor::createQuad() {
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVao);
    glGenBuffers(1, &m_quadVbo);
    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void PostProcessor::destroyQuad() {
    if (m_quadVao == 0) return;
    glDeleteVertexArrays(1, &m_quadVao);
    glDeleteBuffers(1,      &m_quadVbo);
    m_quadVao = m_quadVbo = 0;
}

void PostProcessor::resize(unsigned int width, unsigned int height) {
    if (width == (unsigned int)m_size.x && height == (unsigned int)m_size.y) return;

    m_size = Vec2((float)width, (float)height);

    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    if (m_needsDepth) {
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }
}

void PostProcessor::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, (int)m_size.x, (int)m_size.y);
}

void PostProcessor::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::process(unsigned int inputTexture, Vec2 outputSize, unsigned int depthTexture) {
    if (!m_shader) {
        Logger::warn("PostProcessor::process() called with no shader set — call setShader() first. Skipping.");
        return;
    }

    if (m_outputToScreen) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, (int)outputSize.x, (int)outputSize.y);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, (int)m_size.x, (int)m_size.y);
    }

    m_shader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    m_shader->setInt("screenTexture", 0);

    if (depthTexture != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        m_shader->setInt("depthTexture", 1);
    }

    glBindVertexArray(m_quadVao);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void PostProcessor::setShader(const std::string& vert, const std::string& frag) {
    m_shader = std::make_unique<Shader>(vert, frag);
}

void PostProcessor::setBool(const std::string& name, bool value)        { m_shader->use(); m_shader->setBool(name, value); }
void PostProcessor::setInt(const std::string& name, int value)          { m_shader->use(); m_shader->setInt(name, value); }
void PostProcessor::setFloat(const std::string& name, float value)      { m_shader->use(); m_shader->setFloat(name, value); }
void PostProcessor::setVec2(const std::string& name, const Vec2& value) { m_shader->use(); m_shader->setVec2(name, value); }
void PostProcessor::setVec3(const std::string& name, const Vec3& value) { m_shader->use(); m_shader->setVec3(name, value); }
void PostProcessor::setVec4(const std::string& name, const Vec4& value) { m_shader->use(); m_shader->setVec4(name, value); }
void PostProcessor::setMat4(const std::string& name, const Mat4& value) { m_shader->use(); m_shader->setMat4(name, value); }

void PostProcessor::setTexture(const std::string& name, const Texture& texture, int unit) { m_shader->setTexture(name, texture, unit); }