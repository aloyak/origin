#pragma once

#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"

class Entity {
public:
    Transform transform;

    Mesh* mesh = nullptr;
    Shader* shader = nullptr;
    Texture* texture = nullptr;

    Entity() = default;

    void draw(Engine &engine, const Camera& camera) {
        if (mesh && shader && texture) {
            engine.render(*mesh, *shader, camera, transform, *texture);
        }
    }
};