#pragma once

#include "engine/core/math.h"
#include "engine/components/entity.h"

class Camera;
class Scene;
class Transform;

class Raycast {
public:
    Entity* raycastScene(const Vec3& origin,
                         const Vec3& direction,
                         Scene& scene,
                         float maxDistance = 10000.0f) const;

    Entity* raycastEntities(const Vec3& origin,
                            const Vec3& direction,
                            const std::vector<Entity*>& entities,
                            float maxDistance = 10000.0f) const;

    Entity* raycastScreenPoint(const Vec2& screenPoint,
                               const Vec2& viewportSize,
                               const Camera& camera,
                               const Transform& cameraTransform,
                               Scene& scene,
                               float maxDistance = 10000.0f) const;
};