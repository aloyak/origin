#pragma once

#include <string>
#include <vector>
#include "engine/core/math.h"
#include "engine/render/texture.h"

class Shader {
public:
    unsigned int ID;

    Shader(std::string vertexPath, std::string fragmentPath);
    ~Shader();
    void use();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    
    void setVec2(const std::string& name, const Vec2& value) const;
    void setVec3(const std::string& name, const Vec3& value) const;
    void setVec4(const std::string& name, const Vec4& value) const;

    void setMat4(const std::string& name, const Mat4& value) const;

    void setTexture(const std::string& name, const class Texture& texture, int unit) const;
    
private:
    void compileFromSource(const std::string& vertexPath, const std::string& fragmentPath);
    void loadFromSpirV(const std::string& vertexPath, const std::string& fragmentPath);

    static std::string toSpirVPath(const std::string& source);
    static bool spirVSupported();
    static std::vector<char> readBinaryFile(const std::string& path);

    void checkShader(unsigned int shader);
    void checkProgram();
};