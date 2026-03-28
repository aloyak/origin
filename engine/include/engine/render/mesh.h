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

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw(const Material& material,
              const std::vector<DirectionalLight>& directionalLights,
              const std::vector<PointLight>& pointLights) const;

private:
    unsigned int m_vao, m_vbo, m_ebo;
    int m_vertexCount, m_indexCount;

    int m_maxDirectionalLights = 4;
    int m_maxPointLights = 8;
};