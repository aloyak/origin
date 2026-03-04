#include "engine/engine.h"
#include "engine/shader.h"
#include "engine/model.h"
#include "engine/camera.h"
#include "engine/input/input.h"
#include "engine/input/keycodes.h"
#include <cmath>

int main() {
    Engine engine(800, 600, "ORIGIN DEMO");
    Input &input = engine.getInput();

    Shader shader("assets/shaders/vert.glsl", "assets/shaders/frag.glsl");
    
    Model myModel("assets/models/sponza/sponza.obj");

    Camera camera(45.0f, 800.0f / 600.0f, 0.1f, 10000.0f);

    Transform modelTransform;
    modelTransform.scale = Vec3(0.5f, 0.5f, 0.5f);
    modelTransform.position = Vec3(0.0f, -75.0f, 0.0f);

    while (engine.isRunning()) {
        engine.beginFrame();

        modelTransform.rotation.y += 0.2f;

        if (input.isKeyPressed(ORIGIN_KEY_ESCAPE)) {
            break;
        }

        engine.render(myModel, shader, camera, modelTransform);

        engine.endFrame();
    }

    return 0;
}