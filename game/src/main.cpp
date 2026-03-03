#include "engine/engine.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include "engine/input/input.h"
#include "engine/input/keycodes.h"
#include <cmath>

#include <iostream>

int main() {
    Engine engine(800, 600, "ORIGIN DEMO");
    Input &input = engine.getInput();

    std::vector<float> vertices = {
        -0.5f, -0.5f,  0.5f, // 0: Bottom-front-left
        0.5f, -0.5f,  0.5f, // 1: Bottom-front-right
        0.5f,  0.5f,  0.5f, // 2: Top-front-right
        -0.5f,  0.5f,  0.5f, // 3: Top-front-left
        -0.5f, -0.5f, -0.5f, // 4: Bottom-back-left
        0.5f, -0.5f, -0.5f, // 5: Bottom-back-right
        0.5f,  0.5f, -0.5f, // 6: Top-back-right
        -0.5f,  0.5f, -0.5f  // 7: Top-back-left
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0, // Front
        1, 5, 6, 6, 2, 1, // Right
        7, 6, 5, 5, 4, 7, // Back
        4, 0, 3, 3, 7, 4, // Left
        4, 5, 1, 1, 0, 4, // Bottom
        3, 2, 6, 6, 7, 3  // Top
    };

    Mesh cube(vertices, indices);
    Shader shader("assets/shaders/vert.glsl", "assets/shaders/frag.glsl");
    Camera camera(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

    Transform cubeTransform;
    cubeTransform.scale = Vec3(0.5f, 0.5f, 0.5f);
    cubeTransform.position = Vec3(0.0f, 0.0f, 0.0f);

    while (engine.isRunning()) {
        engine.beginFrame();

        cubeTransform.rotation.y += 0.5f;
        cubeTransform.rotation.x += 0.2f;

        cubeTransform.position.x = std::sin(engine.getTime());
        cubeTransform.position.z = std::cos(engine.getTime());

        if (input.isKeyPressed(ORIGIN_KEY_ESCAPE)) {
            break;
        }

        engine.render(cube, shader, camera, cubeTransform);

        engine.endFrame();
    }

    return 0;
}