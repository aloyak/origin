#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

#include "engine/physics/terrain/terrainMeshIndex.h"
#include "engine/render/model.h"
#include "engine/core/math.h"

class RigidbodyComponent;

// Streams a small local chunk of collision geometry out of a very large
// source mesh instead of building one collider for the whole thing up front
// RigidbodyComponent and keep it fed with "whatever triangles are near the
// tracked point right now"
//
// Two costs are kept off the main thread:
//   1. Building the spatial index over the whole source mesh once per
//      distinct source (cached by key, so re-entering the same source
//      later is free).
//   2. Re-gathering the local triangle subset whenever the tracked point
//      has moved far enough to need a new chunk.

class TerrainCollider {
public:
    struct Config {
        float chunkRadius = 400.0f;
        float rebuildDistance = 100.0f;
        float indexCellSize = 50.0f;
    };

    TerrainCollider() : TerrainCollider(Config{}) {}
    explicit TerrainCollider(Config config);
    ~TerrainCollider();

    TerrainCollider(const TerrainCollider&) = delete;
    TerrainCollider& operator=(const TerrainCollider&) = delete;

    void setSource(const std::string& sourceKey, std::shared_ptr<Model> model);

    void setConfig(const Config& config) { m_config = config; }
    const Config& getConfig() const { return m_config; }


    void update(const Vec3& localCenter, RigidbodyComponent& targetRigidbody);

    bool hasSource() const { return m_model != nullptr; }
    bool isIndexReady() const { return m_index != nullptr; }

private:
    struct ChunkResult {
        std::vector<Vec3> vertices;
        std::vector<unsigned int> indices;
        Vec3 center = Vec3(0.0f, 0.0f, 0.0f);
    };

    void startIndexBuildIfNeeded();
    void startChunkRebuild(const Vec3& localCenter);
    void joinWorkerIfDone();

    Config m_config;

    std::string m_sourceKey;
    std::shared_ptr<Model> m_model;
    std::shared_ptr<TerrainMeshIndex> m_index;
    std::unordered_map<std::string, std::shared_ptr<TerrainMeshIndex>> m_indexCache;

    std::thread m_worker;
    std::atomic<bool> m_workerBusy{false};

    std::mutex m_resultMutex;
    ChunkResult m_pendingResult;
    bool m_hasPendingResult = false;

    Vec3 m_lastBuiltCenter = Vec3(1e9f, 1e9f, 1e9f);
    bool m_hasBuiltOnce = false;
};