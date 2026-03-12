#pragma once

#include "engine/transform.h"
#include <nlohmann/json_fwd.hpp>

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

    virtual void serialize(nlohmann::json& j) const {}
    virtual void deserialize(const nlohmann::json& j) {}
};