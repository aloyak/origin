#include <iostream>

#include <engine.h>

int main() {
    Engine engine(640, 480, "Origin Engine");
    
    while (engine.isRunning()) {
        engine.beginFrame();

        // Game logic and rendering would go here

        engine.endFrame();
    }
}