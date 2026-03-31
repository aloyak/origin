#pragma once

#include "engine/core/transform.h"
#include <nlohmann/json_fwd.hpp>
#include <memory>

class Entity;
class Engine;
class Camera;
class Renderer;

class Component {
public:
    bool isEnabled = true;
    
    Entity* entity = nullptr;

    virtual ~Component() = default;

    virtual void update(float dt) {}
    virtual void render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) {}

    virtual std::unique_ptr<Component> clone() const = 0;

    virtual void serialize(nlohmann::json& j) const {}
    virtual void deserialize(const nlohmann::json& j) {}
};