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

    engine.getRenderer().setMinimumAmbientLight(0.4f);

    Entity* lightObj = sm.getActiveScene()->createEntity("Light");
    lightObj->transform.position = Vec3(0.0f, 200.0f, 0.0f);
    auto* light = lightObj->addComponent<PointLightComponent>();
    light->setColor(Vec3(0.93f, 0.43f, 0.15f));
    light->setIntensity(1.5f);
    light->setRadius(1300.0f);

    float sensitivity = 0.035f;
    Vec3 allowedMove = {1, 0, 1};

    engine.run([&]() {
        float speed = 300.0f * engine.getDeltaTime();
        Vec3 direction = Vec3(0.0f);
        
        if (input.isKeyDown(KEY_W)) direction += player->transform.forward() * allowedMove;
        if (input.isKeyDown(KEY_S)) direction -= player->transform.forward() * allowedMove;
        if (input.isKeyDown(KEY_A)) direction += player->transform.right() * allowedMove;
        if (input.isKeyDown(KEY_D)) direction -= player->transform.right() * allowedMove;
        
        if (direction.length() > 0.0f) {
            direction = direction.normalize();
            player->transform.position += direction * speed;
        }

        lightObj->transform.position.x = cosf(engine.getTime()) * 400.0f;
        Vec3 dynamicColor(
            0.5f + 0.5f * sinf(engine.getTime() * 0.8f),
            0.5f + 0.5f * sinf(engine.getTime() * 0.8f + 2.094f),
            0.5f + 0.5f * sinf(engine.getTime() * 0.8f + 4.188f)
        );
        light->setColor(dynamicColor);

        Vec2 delta = input.getMouseDelta();
        player->transform.rotation.y += delta.x * sensitivity;
        player->transform.rotation.x -= delta.y * sensitivity;

        if (input.isKeyPressed(KEY_E))
            engine.getRenderer().setLightingEnabled(!engine.getRenderer().isLightingEnabled());
    });
}