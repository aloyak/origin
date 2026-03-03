#include "engine/engine.h"

#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/texture.h"

#include "engine/camera.h"
#include "engine/input/input.h"
#include "engine/input/keycodes.h"
#include <cmath>

#include <iostream>

int main() {
    Engine engine(800, 600, "ORIGIN DEMO");
    Input &input = engine.getInput();

    std::vector<float> vertices = {
        // Positions          // Colors (RGB)     // Texture Coords
        // Back face (Red)
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 
        0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 

        // Front face (Green)
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 
        0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, 
        0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 

        // Left face (Blue)
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, 
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 

        // Right face (Yellow)
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f, 
        0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f, 

        // Bottom face (Magenta)
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 

        // Top face (Cyan)
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 
        0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,   1.0f, 1.0f, 
        0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,   1.0f, 0.0f, 
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,   0.0f, 0.0f
    };

    std::vector<unsigned int> indices = {
        0,  1,  2,  2,  3,  0,  // Back
        4,  5,  6,  6,  7,  4,  // Front
        8,  9,  10, 10, 11, 8,  // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Bottom
        20, 21, 22, 22, 23, 20  // Top
    };

    // This should be an 'Entity' ------
    Mesh cube(vertices, indices);
    Shader shader("assets/shaders/vert.glsl", "assets/shaders/frag.glsl");
    Texture texture("assets/textures/crate.jpg");
    // ---------------------------------

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

        engine.render(cube, shader, camera, cubeTransform, texture);

        engine.endFrame();
    }

    return 0;
}