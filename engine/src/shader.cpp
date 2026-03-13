#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>
#include <fstream>
#include <string>
#include <sstream>

#include "engine/shader.h"
#include "engine/debug/path.h"

auto replaceFirstLine = [](std::string& src, const std::string& newLine)
{
    size_t pos = src.find('\n');
    if (pos != std::string::npos)
        src = newLine + "\n" + src.substr(pos + 1);
    else
        src = newLine;
};

Shader::Shader(std::string vertexPath, std::string fragmentPath) {
    vertexPath = Path::resolve(vertexPath).string();
    fragmentPath = Path::resolve(fragmentPath).string();

    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

#ifdef __EMSCRIPTEN__
        replaceFirstLine(vertexCode, "#version 300 es");
        replaceFirstLine(fragmentCode, "#version 300 es\nprecision mediump float;");
#endif

    } catch (std::ifstream::failure& e) {
        spdlog::error("Failed to read shader files: {}", e.what());
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    this->checkShader(vertex);

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    this->checkShader(fragment);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        spdlog::error("Failed to link shader program: {}", infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const Vec2& value) const {
    glm::vec2 glmValue(value.x, value.y);
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(glmValue));
}

void Shader::setVec3(const std::string& name, const Vec3& value) const {
    glm::vec3 glmValue(value.x, value.y, value.z);
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(glmValue));
}

void Shader::setVec4(const std::string& name, const Vec4& value) const {
    glm::vec4 glmValue(value.x, value.y, value.z, value.w);
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(glmValue));
}

void Shader::setMat4(const std::string& name, const void* mat) const {
    const glm::mat4* matPtr = static_cast<const glm::mat4*>(mat);
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &(*matPtr)[0][0]);
}

void Shader::checkShader(unsigned int shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        spdlog::error("Failed to compile shader: {}", infoLog);
    }
}