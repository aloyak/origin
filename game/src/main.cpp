#include "engine/engine.h"
#include "engine/input/input.h"
#include "engine/components/entity.h"
#include "engine/components/renderer.h"
#include "engine/components/camera.h"

#include <iostream>

int main() {
    Engine engine(800, 600, "ORIGIN DEMO");
    Input& input = engine.getInput();
    input.setCursorMode(true); 

    // Camera entity
    Entity* cam = engine.createEntity();
    cam->addComponent<CameraComponent>(45.0f, 800.0f / 600.0f, 0.1f, 10000.0f);
    cam->transform.position = Vec3(0.0f, 150.0f, 0.0f);

    // Renderable entity
    Entity* monkey = engine.createEntity();
    monkey->addComponent<RenderComponent>(
        "assets/models/sponza/sponza.obj",
        "assets/shaders/vert.glsl",
        "assets/shaders/frag.glsl"
    );
    monkey->transform.position = Vec3(0.0f, 0.0f, -5.0f);
    monkey->transform.scale    = Vec3(1.0f, 1.0f, 1.0f);

    float sensitivity = 0.05f;
    
    while (engine.isRunning()) {
        engine.beginFrame();
        
        float speed = 300.0f * engine.getDeltaTime();
        if (input.isKeyPressed(ORIGIN_KEY_W)) cam->transform.position += cam->transform.forward() * speed;
        if (input.isKeyPressed(ORIGIN_KEY_S)) cam->transform.position -= cam->transform.forward() * speed;
        if (input.isKeyPressed(ORIGIN_KEY_A)) cam->transform.position += cam->transform.right() * speed;
        if (input.isKeyPressed(ORIGIN_KEY_D)) cam->transform.position -= cam->transform.right() * speed;

        Vec2 delta = input.getMouseDelta();
        cam->transform.rotation.y += delta.x * sensitivity;
        cam->transform.rotation.x -= delta.y * sensitivity;

        if (input.isKeyPressed(ORIGIN_KEY_ESCAPE))
            break;

        engine.updateScene();

        std::cout << "FPS: " << 1.0f / engine.getDeltaTime() << "\r" << std::flush;

        engine.endFrame();
    }

    return 0;
}