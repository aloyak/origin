#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <memory>

#include "engine/math.h"

class Shader; 
class Texture;

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

    void draw(Shader& shader) const; 

private:
    unsigned int m_vao, m_vbo, m_ebo;
    int m_vertexCount, m_indexCount;
};