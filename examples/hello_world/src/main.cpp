#include "iostream"

#include "debug.h"
#include "undicht_graphics.h"

using namespace undicht;
using namespace graphics;

// root dir of the hello world example
const std::string PROJECT_DIR = std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/')) + "/../";

int main() {

	WindowAPI window_api;

	Window window("Hello World");

	GraphicsAPI graphics_api;
	GraphicsSurface canvas = graphics_api.createGraphicsSurface(window);
	GraphicsDevice gpu = graphics_api.getGraphicsDevice(canvas);
	SwapChain swap_chain = graphics_api.createSwapChain(gpu, canvas);

	UND_LOG << "using graphics api: vulkan\n";	
	UND_LOG << "using gpu: " << gpu.info() << " score: " << graphics_api.rateDevice(gpu) << "\n";

	Shader shader = gpu.createShader();
	shader.loadBinaryFile(PROJECT_DIR + "res/vert.spv", UND_VERTEX_SHADER);
	shader.loadBinaryFile(PROJECT_DIR + "res/frag.spv", UND_FRAGMENT_SHADER);
	shader.linkStages();

	Renderer renderer = gpu.createRenderer();
	renderer.setShader(&shader);
	renderer.setRenderTarget(&swap_chain);
	renderer.linkPipeline();


	while(!window.shouldClose()) {

		window.update();
	}

	return 0;
}
