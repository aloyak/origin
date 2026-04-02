#include "engine/physics/raycast.h"

#include "engine/components/rigidbodyComponent.h"
#include "engine/physics/world.h"

#include "engine/render/camera.h"
#include "engine/core/transform.h"
#include "engine/scene/scene.h"

#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

Entity* Raycast::raycastEntities(const Vec3& origin,
                                 const Vec3& direction,
                                 const std::vector<Entity*>& entities,
                                 float maxDistance) const {
    PhysicsWorld* world = PhysicsWorld::getActive();
    if (!world) {
        return nullptr;
    }

    btCollisionObject* hitObj = nullptr;
    if (!world->rayTest(origin, direction, maxDistance, hitObj)) {
        return nullptr;
    }

    // Match the hit Bullet object back to one of the provided entities.
    const btRigidBody* hitRigidBody = btRigidBody::upcast(hitObj);
    for (Entity* entity : entities) {
        if (!entity) continue;
        auto* rb = entity->getComponent<RigidbodyComponent>();
        if (rb && rb->getRigidBody() == hitRigidBody) {
            return entity;
        }
    }

    return nullptr;
}

Entity* Raycast::raycastScene(const Vec3& origin,
                              const Vec3& direction,
                              Scene& scene,
                              float maxDistance) const {
    std::vector<Entity*> entities;
    entities.reserve(scene.getEntities().size());

    for (const auto& entity : scene.getEntities()) {
        entities.push_back(entity.get());
    }

    return raycastEntities(origin, direction, entities, maxDistance);
}

Entity* Raycast::raycastScreenPoint(const Vec2& screenPoint,
                                    const Vec2& viewportSize,
                                    const Camera& camera,
                                    const Transform& cameraTransform,
                                    Scene& scene,
                                    float maxDistance) const {
    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) {
        return nullptr;
    }

    const auto* viewPtr = static_cast<const glm::mat4*>(camera.getViewMatrix(cameraTransform));
    const auto* projectionPtr = static_cast<const glm::mat4*>(camera.getProjectionMatrix());
    if (!viewPtr || !projectionPtr) {
        return nullptr;
    }

    const glm::mat4& view = *viewPtr;
    const glm::mat4& projection = *projectionPtr;

    const glm::mat4 viewInv = glm::inverse(view);
    const glm::mat4 projInv = glm::inverse(projection);

    // Convert screen coordinates to normalized device coordinates [-1, 1]
    const float normalizedX = (2.0f * screenPoint.x) / viewportSize.x - 1.0f;
    const float normalizedY = 1.0f - (2.0f * screenPoint.y) / viewportSize.y;  // Flip Y for OpenGL

    glm::vec4 rayClip(normalizedX, normalizedY, -1.0f, 1.0f);
    glm::vec4 rayView = projInv * rayClip;
    rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f);  // Direction, not position

    glm::vec4 rayWorld = viewInv * rayView;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z));

    Vec3 rayOrigin(cameraTransform.position.x, cameraTransform.position.y, cameraTransform.position.z);
    Vec3 rayDir(rayDirection.x, rayDirection.y, rayDirection.z);

    return raycastScene(rayOrigin, rayDir, scene, maxDistance);
}