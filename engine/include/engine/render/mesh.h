#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <memory>

#include "engine/core/math.h"

class Shader; 
class Texture;
class Material;
class DirectionalLight;
class PointLight;

struct Vertex {
    Vec3 Position;
    Vec3 Normal;
    Vec2 TexCoords;
    Vec3 Tangent;
    Vec3 Bitangent;
};

struct MeshTexture {
    std::shared_ptr<Texture> texture;
    std::string type;
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<MeshTexture>  textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures,
         bool dynamic = false, Vec3 baseColor = Vec3(1.0f, 1.0f, 1.0f));
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void updateVertexBuffer();
    void recalculateNormals();

    bool isDynamic() const { return m_dynamic; }
    
    Vec3 getBaseColor() const { return m_baseColor; } // base color if no texture but material still has color data

    void draw(const Material& material,
              const std::vector<DirectionalLight>& directionalLights,
              const std::vector<PointLight>& pointLights) const;


    void drawInstanced(const Material& material,
                   const std::vector<DirectionalLight>& directionalLights,
                   const std::vector<PointLight>& pointLights,
                   unsigned int instanceVBO,
                   unsigned int instanceCount) const;

private:
    unsigned int m_vao, m_vbo, m_ebo;
    int m_vertexCount, m_indexCount;
    bool m_dynamic = false;
    Vec3 m_baseColor = Vec3(1.0f, 1.0f, 1.0f);

    int m_maxDirectionalLights = 4;
    int m_maxPointLights = 8;
};