#include "iostream"

#include "undicht_graphics.h"

using namespace undicht;
using namespace graphics;

int main() {

    WindowAPI window_api;

    Window window("Hello World");

    GraphicsAPI graphics_api;
    std::cout << "Hello World\n";

    while(!window.shouldClose()) {

        window.update();
    }

    return 0;
}