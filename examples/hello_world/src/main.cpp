#include "iostream"

#include "debug.h"
#include "undicht_graphics.h"

using namespace undicht;
using namespace graphics;

int main() {

    WindowAPI window_api;

    Window window("Hello World");

    GraphicsAPI graphics_api;
    GraphicsSurface canvas = graphics_api.createGraphicsSurface(window);
    GraphicsDevice gpu = graphics_api.getGraphicsDevice(canvas);
	SwapChain swap_chain = graphics_api.createSwapChain(gpu, canvas);

	UND_LOG << "using graphics api: vulkan\n";	
    UND_LOG << "using gpu: " << gpu.info() << " score: " << graphics_api.rateDevice(gpu) << "\n";

    while(!window.shouldClose()) {

        window.update();
    }

    return 0;
}
