#include "sandbox/layer.h"

int main() {
    Engine engine(1600, 900, "Origin Sandbox");
    engine.initUI();
    engine.getWindow().enableVSync(true);
    engine.getRenderer().setupRenderTarget(1600, 900);

    Layer layer(engine);

    engine.run([&]() {
        engine.beginUI();
        layer.OnUIRender();
        engine.endUI();
    });
}