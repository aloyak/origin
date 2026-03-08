#include "engine/engine.h"
#include "engine/input/input.h"
#include "engine/components/entity.h"
#include "engine/components/renderer.h"
#include "engine/components/camera.h"

#include <iostream>

// Simplified "game" for easy testing!
int main() {
    Engine engine(1440, 900, "ORIGIN DEMO");
    Input& input = engine.getInput();

    input.setCursorMode(true); // true = locked
    engine.setFullscreen(false);
    engine.enableVSync(false);

    // Camera entity
    Entity* player = engine.createEntity();
    player->addComponent<CameraComponent>(60.0f, engine.getAspectRatio(), 0.1f, 10000.0f); // fov, aspect ratio, near, far
    player->transform.position = Vec3(0.0f, 150.0f, 0.0f);

    // Renderable entity
    Entity* sponza = engine.createEntity();
    // Shaders default to assets/shaders/vert.glsl or frag.glsl if not specified
    sponza->addComponent<RenderComponent>("assets/models/sponza/sponza.obj");

    float sensitivity = 0.035f;
    Vec3 allowedMove = {1, 0, 1};

    engine.run([&]() {
        float speed = 300.0f * engine.getDeltaTime();
        if (input.isKeyPressed(KEY_W)) player->transform.position += player->transform.forward() * allowedMove * speed;
        if (input.isKeyPressed(KEY_S)) player->transform.position -= player->transform.forward() * allowedMove * speed;
        if (input.isKeyPressed(KEY_A)) player->transform.position += player->transform.right() * allowedMove * speed;
        if (input.isKeyPressed(KEY_D)) player->transform.position -= player->transform.right() * allowedMove * speed;

        Vec2 delta = input.getMouseDelta();
        player->transform.rotation.y += delta.x * sensitivity;
        player->transform.rotation.x -= delta.y * sensitivity;

        if (input.isKeyPressed(KEY_ESCAPE))
            engine.stop();

        std::cout << "FPS: " << 1.0f / engine.getDeltaTime() << "\r" << std::flush;
    });
}