#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

#include "engine/render/shader.h"
#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#ifndef __EMSCRIPTEN__
    #if defined(GL_SHADER_BINARY_FORMAT_SPIR_V_ARB) && !defined(GL_SHADER_BINARY_FORMAT_SPIR_V)
        #define GL_SHADER_BINARY_FORMAT_SPIR_V GL_SHADER_BINARY_FORMAT_SPIR_V_ARB
        #define glSpecializeShader glSpecializeShaderARB
    #endif
#endif

auto replaceFirstLine = [](std::string& src, const std::string& newLine)
{
    size_t pos = src.find('\n');
    if (pos != std::string::npos)
        src = newLine + "\n" + src.substr(pos + 1);
    else
        src = newLine;
};

std::string Shader::toSpirVPath(const std::string& source) {
    size_t dot = source.rfind('.');
    return (dot == std::string::npos ? source : source.substr(0, dot)) + ".spv";
}

bool Shader::spirVSupported() {
#if defined(__EMSCRIPTEN__) || !defined(GL_SHADER_BINARY_FORMAT_SPIR_V)
    return false;
#else
    if (!glSpecializeShader) return false; // declared, but driver didn't resolve it
    GLint numFormats = 0;
    glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numFormats);
    return numFormats > 0;
#endif
}

std::vector<char> Shader::readBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Logger::error("Failed to open precompiled shader file: " + path);
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(static_cast<size_t>(size));
    if (size > 0 && !file.read(buffer.data(), size)) {
        Logger::error("Failed to read precompiled shader file: " + path);
        return {};
    }
    return buffer;
}

Shader::Shader(std::string vertexPath, std::string fragmentPath) {
    vertexPath = Path::resolve(vertexPath).string();
    fragmentPath = Path::resolve(fragmentPath).string();

    bool hasSource = std::filesystem::exists(vertexPath) &&
                     std::filesystem::exists(fragmentPath);
    if (hasSource) {
        compileFromSource(vertexPath, fragmentPath);
        return;
    }

    std::string vertSpv = toSpirVPath(vertexPath);
    std::string fragSpv = toSpirVPath(fragmentPath);
    bool hasPrecompiled = std::filesystem::exists(vertSpv) &&
                          std::filesystem::exists(fragSpv);

    if (!hasPrecompiled) {
        Logger::error("Shader not found (from source or precompiled): " + vertexPath);
        return;
    }

    if (!spirVSupported()) {
        Logger::error("Precompiled shader found but GL_ARB_gl_spirv is unavailable: " + vertSpv);
        return;
    }

    loadFromSpirV(vertSpv, fragSpv);
}

void Shader::compileFromSource(const std::string& vertexPath, const std::string& fragmentPath) {
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
        Logger::error("Failed to read shader files: " + std::string(e.what()));
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkShader(vertex);

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkShader(fragment);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkProgram();

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::loadFromSpirV(const std::string& vertexPath, const std::string& fragmentPath) {
#if defined(__EMSCRIPTEN__) || !defined(GL_SHADER_BINARY_FORMAT_SPIR_V)
    Logger::error("Precompiled SPIR-V shaders are not supported by this build (regenerate glad with GL_ARB_gl_spirv / GL 4.6 core): " + vertexPath);
    return;
#else
    auto vertBytes = readBinaryFile(vertexPath);
    auto fragBytes = readBinaryFile(fragmentPath);

    if (vertBytes.empty() || fragBytes.empty()) {
        Logger::error("Precompiled shader could not be loaded: " + vertexPath);
        return;
    }

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertex, GL_SHADER_BINARY_FORMAT_SPIR_V, vertBytes.data(), (int)vertBytes.size());
    glSpecializeShader(vertex, "main", 0, nullptr, nullptr);
    checkShader(vertex);

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragment, GL_SHADER_BINARY_FORMAT_SPIR_V, fragBytes.data(), (int)fragBytes.size());
    glSpecializeShader(fragment, "main", 0, nullptr, nullptr);
    checkShader(fragment);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkProgram();

    glDeleteShader(vertex);
    glDeleteShader(fragment);
#endif
}

Shader::~Shader() {
    glDeleteProgram(ID);
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

void Shader::setMat4(const std::string& name, const Mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::setTexture(const std::string& name, const Texture& texture, int unit) const {
    texture.bind(unit);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), unit);
}

void Shader::checkShader(unsigned int shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        Logger::error("Failed to compile shader: " + std::string(infoLog));
    }
}

void Shader::checkProgram() {
    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        Logger::error("Failed to link shader program: " + std::string(infoLog));
    }
}