#include "iostream"

#include "debug.h"
#include "undicht_graphics.h"

using namespace undicht;
using namespace graphics;

int main() {

    WindowAPI window_api;

    Window window("Hello World");

    GraphicsAPI graphics_api;
    GraphicsDevice gpu = graphics_api.getGraphicsDevice();

    UND_LOG << "using gpu: " << gpu.info() << "\n";

    while(!window.shouldClose()) {

        window.update();
    }

    return 0;
}