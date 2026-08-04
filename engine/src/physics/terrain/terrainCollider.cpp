#include "engine/physics/terrain/terrainCollider.h"
#include "engine/components/rigidbodyComponent.h"

TerrainCollider::TerrainCollider(Config config) : m_config(config) {}

TerrainCollider::~TerrainCollider() {
    if (m_worker.joinable()) {
        m_worker.join();
    }
}

void TerrainCollider::setSource(const std::string& sourceKey, std::shared_ptr<Model> model) {
    if (sourceKey == m_sourceKey && model == m_model) return;

    if (m_worker.joinable()) {
        m_worker.join();
    }

    m_sourceKey = sourceKey;
    m_model = std::move(model);

    const auto cached = m_indexCache.find(m_sourceKey);
    m_index = (cached != m_indexCache.end()) ? cached->second : nullptr;

    m_hasBuiltOnce = false;
    m_hasPendingResult = false;
    m_lastBuiltCenter = Vec3(1e9f, 1e9f, 1e9f);
}

void TerrainCollider::startIndexBuildIfNeeded() {
    if (m_index || !m_model) return;
    if (m_worker.joinable()) return;

    m_workerBusy = true;

    const std::string key = m_sourceKey;
    const std::shared_ptr<Model> model = m_model;
    const float cellSize = m_config.indexCellSize;

    m_worker = std::thread([this, key, model, cellSize]() {
        auto index = std::make_shared<TerrainMeshIndex>();
        index->build(model->getMeshes(), cellSize);

        std::lock_guard<std::mutex> lock(m_resultMutex);
        if (m_sourceKey == key) {
            m_indexCache[key] = index;
            m_index = index;
        }
        m_workerBusy = false;
    });
}

void TerrainCollider::startChunkRebuild(const Vec3& localCenter) {
    if (!m_index) return;
    if (m_worker.joinable()) return;

    m_workerBusy = true;
    const std::shared_ptr<TerrainMeshIndex> index = m_index;
    const float radius = m_config.chunkRadius;

    m_worker = std::thread([this, index, localCenter, radius]() {
        ChunkResult result;
        result.center = localCenter;
        index->queryTrianglesInRadius(localCenter, radius, result.vertices, result.indices);

        std::lock_guard<std::mutex> lock(m_resultMutex);
        m_pendingResult = std::move(result);
        m_hasPendingResult = true;
        m_workerBusy = false;
    });
}

void TerrainCollider::joinWorkerIfDone() {
    if (m_worker.joinable() && !m_workerBusy) {
        m_worker.join();
    }
}

void TerrainCollider::update(const Vec3& localCenter, RigidbodyComponent& targetRigidbody) {
    if (!m_model) return;

    joinWorkerIfDone();

    if (!m_index) {
        startIndexBuildIfNeeded();
        return;
    }

    // Apply whatever chunk finished building, if any, before deciding
    // whether we need another one
    {
        std::lock_guard<std::mutex> lock(m_resultMutex);
        if (m_hasPendingResult) {
            if (targetRigidbody.applyMeshColliderRegion(m_pendingResult.vertices, m_pendingResult.indices)) {
                m_lastBuiltCenter = m_pendingResult.center;
                m_hasBuiltOnce = true;
            }
            m_hasPendingResult = false;
        }
    }

    const Vec3 delta = localCenter - m_lastBuiltCenter;
    const float distSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    const bool needsRebuild = !m_hasBuiltOnce || distSq > (m_config.rebuildDistance * m_config.rebuildDistance);

    if (needsRebuild) {
        startChunkRebuild(localCenter);
    }
}