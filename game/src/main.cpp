#include "engine/engine.h"
#include "engine/input/input.h"
#include "engine/components/entity.h"
#include "engine/components/renderer.h"
#include "engine/components/camera.h"

#include <iostream>

int main() {
    Engine engine(1440, 900, "ORIGIN DEMO");
    Input& input = engine.getInput();

    input.setCursorMode(true); 

    //engine.setFullscreen(true);
    //engine.setWindowStartCentered(true);

    // Camera entity
    Entity* player = engine.createEntity();
    player->addComponent<CameraComponent>(60.0f, 1440.0f / 900.0f, 0.1f, 10000.0f);
    player->transform.position = Vec3(0.0f, 150.0f, 0.0f);

    // Renderable entity
    Entity* sponza = engine.createEntity();
    sponza->addComponent<RenderComponent>( // Shaders default to assets/shaders/vert.glsl or frag.glsl
        "assets/models/sponza/sponza.obj"
    );

    Entity* suzanne = engine.createEntity();
    suzanne->addComponent<RenderComponent>(
        "assets/models/suzanne.obj"
    );
    suzanne->transform.position = Vec3(0.0f, 150.0f, 0.0f);
    suzanne->transform.scale = Vec3(25.0f, 25.0f, 25.0f);

    float sensitivity = 0.035f;
    Vec3 allowedMove = {1, 0, 1};
    
    while (engine.isRunning()) {
        engine.beginFrame();
        
        float speed = 300.0f * engine.getDeltaTime();
        if (input.isKeyPressed(KEY_W)) player->transform.position += player->transform.forward() * allowedMove * speed;
        if (input.isKeyPressed(KEY_S)) player->transform.position -= player->transform.forward() * allowedMove * speed;
        if (input.isKeyPressed(KEY_A)) player->transform.position += player->transform.right() * allowedMove * speed;
        if (input.isKeyPressed(KEY_D)) player->transform.position -= player->transform.right() * allowedMove * speed;

        Vec2 delta = input.getMouseDelta();
        player->transform.rotation.y += delta.x * sensitivity;
        player->transform.rotation.x -= delta.y * sensitivity;

        suzanne->transform.rotation.y += .3f;

        static int cooldown = 0;
        if (input.isKeyPressed(KEY_E) && cooldown == 0) {
            suzanne->getComponent<RenderComponent>()->isEnabled = !suzanne->getComponent<RenderComponent>()->isEnabled;
            cooldown = 50;
        }
        if (cooldown > 0) cooldown--;

        if (input.isKeyPressed(KEY_ESCAPE))
            break;

        engine.updateScene();

        std::cout << "FPS: " << 1.0f / engine.getDeltaTime() << "\r" << std::flush;

        engine.endFrame();
    }

    return 0;
}