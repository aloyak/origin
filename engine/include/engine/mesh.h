#pragma once

#include <vector>

class Mesh {
public:
    Mesh(const std::vector<float>& vertices);
    ~Mesh();

    void draw() const;

private:
    unsigned int m_vao, m_vbo;
    int m_vertexCount;
};