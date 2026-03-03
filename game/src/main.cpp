#include "engine/engine.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include <cmath>

int main() {
    Engine engine(800, 600, "ORIGIN DEMO");

    std::vector<float> cubeVertices = {
        // Back
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        // Front
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
        // Left
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        // Right
        0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
        // Bottom
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
        // Top
        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
    };

    Mesh cube(cubeVertices);
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

        engine.render(cube, shader, camera, cubeTransform);

        engine.endFrame();
    }

    return 0;
}