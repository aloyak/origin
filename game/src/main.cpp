#include "engine/engine.h"
#include "engine/input/input.h"
#include "engine/scene/sceneManager.h"

#include "engine/utils/logger.h"

#include "engine/components/rendererComponent.h"
#include "engine/components/cameraComponent.h"
#include "engine/components/skyboxComponent.h"
#include "engine/components/directionalLightComponent.h"
#include "engine/components/pointLightComponent.h"

#include <iostream>

int main() {
    Engine engine(1600, 900, "Origin Game");

    // Pixel art settings
    engine.getRenderer().setupRenderTarget(400, 225); // Creates the offscreen framebuffer (used for pixelart or special post-processing effects, like the sandbox viewport!)
    engine.getRenderer().setPixelArt(true, 8);
    engine.getRenderer().setLightingEnabled(true);
    engine.getRenderer().setMinimumAmbientLight(0.1f);
    //engine.getRenderer().setVertexSnap(true, 100.0f); // Higher values give a less noticeable effect

    Input& input = engine.getInput();
    SceneManager& sm = engine.getSceneManager();

    input.setCursorMode(true); // true = locked
    engine.getWindow().setFullscreen(false);
    engine.getWindow().enableVSync(false);

    sm.load("assets/scenes/sponza.json");

    // Player entity (camera)
    Entity* player = engine.createEntity("Player");
    player->addComponent<CameraComponent>(60.0f, engine.getWindow().getAspectRatio(), 0.1f, 10000.0f); // fov, aspect ratio, near, far
    player->transform.position = Vec3(0.0f, 150.0f, 0.0f);

    Entity* directionalLight = engine.createEntity("Directional Light");
    directionalLight->addComponent<DirectionalLightComponent>(Vec3(-1, -1, -1), Vec3(1, 1, 1), 0.5f);

    engine.getRenderer().setMinimumAmbientLight(0.2f);

    float sensitivity = 0.035f;
    Vec3 allowedMove = {1, 0, 1};

    engine.run([&]() {
        float speed = 300.0f * engine.getDeltaTime();
        Vec3 direction = Vec3(0.0f);
        
        if (input.isKeyPressed(KEY_W)) direction += player->transform.forward() * allowedMove;
        if (input.isKeyPressed(KEY_S)) direction -= player->transform.forward() * allowedMove;
        if (input.isKeyPressed(KEY_A)) direction += player->transform.right() * allowedMove;
        if (input.isKeyPressed(KEY_D)) direction -= player->transform.right() * allowedMove;
        
        if (direction.length() > 0.0f) {
            direction = direction.normalize();
            player->transform.position += direction * speed;
        }

        Vec2 delta = input.getMouseDelta();
        player->transform.rotation.y += delta.x * sensitivity;
        player->transform.rotation.x -= delta.y * sensitivity;

        if (input.isKeyPressed(KEY_ESCAPE))
            engine.stop();

        if (input.isKeyPressed(KEY_E))
            engine.getRenderer().setLightingEnabled(!engine.getRenderer().isLightingEnabled());
    });
}