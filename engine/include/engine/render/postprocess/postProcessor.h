#pragma once

#include <string>
#include <memory>

#include "engine/core/math.h"

class Shader;

class PostProcessor {
public:
    PostProcessor(Vec2 size, bool needsDepth = false);
    ~PostProcessor();

    PostProcessor(const PostProcessor&) = delete;
    PostProcessor& operator=(const PostProcessor&) = delete;

    void resize(unsigned int width, unsigned int height);

    void bind();
    void unbind();

    void process(unsigned int inputTexture, Vec2 outputSize, unsigned int depthTexture = 0);

    void setShader(const std::string& vert, const std::string& frag);

    void setOutputToScreen(bool enabled) { m_outputToScreen = enabled; }
    bool isOutputToScreen() const { return m_outputToScreen; }

    unsigned int getTexture() const { return m_fboTexture; }
    unsigned int getFBO() const { return m_fbo; }
    Vec2 getSize() const { return m_size; }

    Shader& getShader() { return *m_shader; }

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const Vec2& value);
    void setVec3(const std::string& name, const Vec3& value);
    void setVec4(const std::string& name, const Vec4& value);
    void setMat4(const std::string& name, const Mat4& value);

private:
    void createFramebuffer();
    void destroyFramebuffer();
    void createQuad();
    void destroyQuad();

    Vec2 m_size;
    bool m_needsDepth;
    bool m_outputToScreen = false;

    unsigned int m_fbo        = 0;
    unsigned int m_fboTexture = 0;
    unsigned int m_rbo        = 0; // depth/stencil, only if m_needsDepth

    unsigned int m_quadVao = 0;
    unsigned int m_quadVbo = 0;

    std::unique_ptr<Shader> m_shader;
};