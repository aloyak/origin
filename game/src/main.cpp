#include "engine/engine.h"
#include "engine/input/input.h"
#include "engine/scene/sceneManager.h"

#include "engine/utils/logger.h"

#include "engine/components/rendererComponent.h"
#include "engine/components/cameraComponent.h"
#include "engine/components/skyboxComponent.h"
#include "engine/components/directionalLightComponent.h"
#include "engine/components/pointLightComponent.h"
#include "engine/components/rigidbodyComponent.h"

#include <iostream>

int main() {
    Engine engine(1600, 900, "Physics Demo");

    // Pixel art settings
    engine.getRenderer().setupRenderTarget(400, 225); // Creates the offscreen framebuffer (used for pixelart or special post-processing effects, like the sandbox viewport!)
    engine.getRenderer().setPixelArt(true, 8);
    //engine.getRenderer().setVertexSnap(true, 100.0f); // Higher values give a less noticeable effect

    Input& input = engine.getInput();
    SceneManager& sm = engine.getSceneManager();

    input.setCursorMode(true); // true = locked
    engine.getWindow().setFullscreen(false);
    engine.getWindow().enableVSync(false);

    sm.load("assets/scenes/sponza.json"); // Oops

    engine.getPhysicsWorld().setGravity(Vec3(0.0f, -980.0f, 0.0f)); // Scene is too big for normal gravity

    // Player entity (camera)
    Entity* playerCam = sm.getActiveScene()->getEntityByName("Player Camera")->get();
    Entity* playerBody = sm.getActiveScene()->getEntityByName("Player Body")->get();

    Entity* lightObj = sm.getActiveScene()->createEntity("Light");
    lightObj->transform.position = Vec3(0.0f, 200.0f, 0.0f);
    auto* light = lightObj->addComponent<PointLightComponent>();
    light->setColor(Vec3(0.93f, 0.43f, 0.15f));
    light->setIntensity(1.5f);
    light->setRadius(1300.0f);

    float sensitivity = 0.05f;
    Vec3 allowedMove = {1, 0, 1};

    engine.run([&]() {
        const float dt = engine.getDeltaTime();
        const float speed = 300.0f * dt;

        // Mouse look: yaw on body, pitch on camera
        Vec2 delta = input.getMouseDelta();
        playerBody->transform.rotation.y += delta.x * sensitivity;
        playerCam->transform.rotation.x -= delta.y * sensitivity;
        playerCam->transform.rotation.x = std::clamp(playerCam->transform.rotation.x, -89.0f, 89.0f);
        playerCam->transform.rotation.y = playerBody->transform.rotation.y;

        // Move body in facing direction
        Vec3 direction(0.0f);

        Vec3 forward = playerBody->transform.forward() * allowedMove;
        Vec3 right   = playerBody->transform.right() * allowedMove;

        if (input.isKeyDown(KEY_W)) direction += forward;
        if (input.isKeyDown(KEY_S)) direction -= forward;
        if (input.isKeyDown(KEY_A)) direction += right;
        if (input.isKeyDown(KEY_D)) direction -= right;

        if (direction.length() > 0.0f) {
            direction = direction.normalize();
            playerBody->transform.position += direction * speed;
        }

        // Dynamic light
        lightObj->transform.position.x = cosf(engine.getTime()) * 400.0f;
        Vec3 dynamicColor(
            0.5f + 0.5f * sinf(engine.getTime() * 0.8f),
            0.5f + 0.5f * sinf(engine.getTime() * 0.8f + 2.094f),
            0.5f + 0.5f * sinf(engine.getTime() * 0.8f + 4.188f)
        );
        light->setColor(dynamicColor);

        // Camera follows body
        //float headBob = sinf(engine.getTime() * 5.0f) * 2.0f; 

        float headBob = 0.0f;
        if (direction.length() > 0.0f) {
            headBob = sinf(engine.getTime() * 15.0f) * 3.0f; 
        } else {
            headBob = 0.0f;
        }

        playerCam->transform.position = playerBody->transform.position + Vec3(0.0f, 100.0f + headBob, 0.0f);
    });

    return 0;
}