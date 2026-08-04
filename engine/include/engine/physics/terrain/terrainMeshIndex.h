#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <functional>

#include "engine/core/math.h"
#include "engine/render/mesh.h"

// TerrainMeshIndex builds a one-time spatial index over the triangles of a
// (typically very large) source mesh, so that "give me every triangle near
// this point" can be answered in linear time to the number of triangles near
class TerrainMeshIndex {
public:
    TerrainMeshIndex() = default;

    void build(const std::vector<Mesh>& meshes, float cellSize);

    bool isBuilt() const { return m_built; }
    float getCellSize() const { return m_cellSize; }
    std::size_t getTriangleCount() const { return m_triangles.size(); }

    // Fills outVertices/outIndices with a self-contained triangle soup for
    // every triangle whose centroid lies within "radius" of "center"
    void queryTrianglesInRadius(const Vec3& center, float radius,
                                 std::vector<Vec3>& outVertices,
                                 std::vector<unsigned int>& outIndices) const;

private:
    struct CellKey {
        int32_t x, y, z;
        bool operator==(const CellKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct CellKeyHash {
        std::size_t operator()(const CellKey& k) const {
            std::size_t h1 = std::hash<int32_t>()(k.x);
            std::size_t h2 = std::hash<int32_t>()(k.y);
            std::size_t h3 = std::hash<int32_t>()(k.z);
            return h1 ^ (h2 * 0x9E3779B1u) ^ (h3 * 0x85EBCA77u);
        }
    };

    struct Triangle {
        Vec3 p0, p1, p2;
        Vec3 centroid;
    };

    CellKey cellKeyForPoint(const Vec3& p) const;

    std::vector<Triangle> m_triangles;
    std::unordered_map<CellKey, std::vector<uint32_t>, CellKeyHash> m_grid;
    float m_cellSize = 1.0f;
    bool m_built = false;
};