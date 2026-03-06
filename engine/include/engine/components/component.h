#pragma once

#include "engine/transform.h"

class Entity;
class Engine;
class Camera;

class Component {
public:
    bool isEnabled = true;

    Entity* entity = nullptr;

    virtual ~Component() = default;

    virtual void update(float dt) {}
    virtual void render(Engine& engine, const Camera& camera, const Transform& cameraTransform) {}
};