#include "engine/physics/terrain/terrainMeshIndex.h"

#include <cmath>

TerrainMeshIndex::CellKey TerrainMeshIndex::cellKeyForPoint(const Vec3& p) const {
    return CellKey{
        static_cast<int32_t>(std::floor(p.x / m_cellSize)),
        static_cast<int32_t>(std::floor(p.y / m_cellSize)),
        static_cast<int32_t>(std::floor(p.z / m_cellSize))
    };
}

void TerrainMeshIndex::build(const std::vector<Mesh>& meshes, float cellSize) {
    m_cellSize = cellSize > 0.0f ? cellSize : 1.0f;
    m_triangles.clear();
    m_grid.clear();

    std::size_t triangleCount = 0;
    for (const Mesh& mesh : meshes) {
        triangleCount += mesh.indices.size() / 3;
    }
    m_triangles.reserve(triangleCount);

    for (const Mesh& mesh : meshes) {
        const auto& vertices = mesh.vertices;
        const auto& indices = mesh.indices;
        if (vertices.empty() || indices.size() < 3) continue;

        for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
            const unsigned int i0 = indices[i];
            const unsigned int i1 = indices[i + 1];
            const unsigned int i2 = indices[i + 2];
            if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) continue;

            Triangle tri;
            tri.p0 = vertices[i0].Position;
            tri.p1 = vertices[i1].Position;
            tri.p2 = vertices[i2].Position;
            tri.centroid = (tri.p0 + tri.p1 + tri.p2) * (1.0f / 3.0f);
            m_triangles.push_back(tri);
        }
    }

    for (uint32_t i = 0; i < static_cast<uint32_t>(m_triangles.size()); ++i) {
        m_grid[cellKeyForPoint(m_triangles[i].centroid)].push_back(i);
    }

    m_built = true;
}

void TerrainMeshIndex::queryTrianglesInRadius(const Vec3& center, float radius,
                                               std::vector<Vec3>& outVertices,
                                               std::vector<unsigned int>& outIndices) const {
    outVertices.clear();
    outIndices.clear();
    if (!m_built || radius <= 0.0f) return;

    const float radiusSq = radius * radius;

    const CellKey minCell = cellKeyForPoint(center - Vec3(radius, radius, radius));
    const CellKey maxCell = cellKeyForPoint(center + Vec3(radius, radius, radius));

    outVertices.reserve(4096);
    outIndices.reserve(4096);

    for (int32_t cx = minCell.x; cx <= maxCell.x; ++cx) {
        for (int32_t cy = minCell.y; cy <= maxCell.y; ++cy) {
            for (int32_t cz = minCell.z; cz <= maxCell.z; ++cz) {
                const auto it = m_grid.find(CellKey{cx, cy, cz});
                if (it == m_grid.end()) continue;

                for (uint32_t triIndex : it->second) {
                    const Triangle& tri = m_triangles[triIndex];
                    const Vec3 toCentroid = tri.centroid - center;
                    const float distSq = toCentroid.x * toCentroid.x +
                                          toCentroid.y * toCentroid.y +
                                          toCentroid.z * toCentroid.z;
                    if (distSq > radiusSq) continue;

                    const unsigned int base = static_cast<unsigned int>(outVertices.size());
                    outVertices.push_back(tri.p0);
                    outVertices.push_back(tri.p1);
                    outVertices.push_back(tri.p2);
                    outIndices.push_back(base);
                    outIndices.push_back(base + 1);
                    outIndices.push_back(base + 2);
                }
            }
        }
    }
}