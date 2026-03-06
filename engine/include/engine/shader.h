#pragma once

#include <string>
#include "engine/math.h"

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    
    void setVec2(const std::string& name, const Vec2& value) const;
    void setVec3(const std::string& name, const Vec3& value) const;
    void setVec4(const std::string& name, const Vec4& value) const;

    void setMat4(const std::string& name, const void* mat) const;
    
private:
    void checkShader(unsigned int shader);
};