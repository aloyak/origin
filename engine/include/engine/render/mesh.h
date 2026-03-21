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

struct Vertex {
    Vec3 Position;
    Vec3 Normal;
    Vec2 TexCoords;
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

    void draw(const Material& material, const std::vector<DirectionalLight>& lights) const;

private:
    unsigned int m_vao, m_vbo, m_ebo;
    int m_vertexCount, m_indexCount;

    int m_maxDirectionalLights = 4;
};