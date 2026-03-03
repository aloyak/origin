#pragma once

#include <vector>

class Mesh {
public:
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    void draw() const;

private:
    unsigned int m_vao, m_vbo, m_ebo;
    int m_vertexCount, m_indexCount;
};