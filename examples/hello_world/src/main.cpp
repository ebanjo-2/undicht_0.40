#include "iostream"

#include "debug.h"
#include "undicht_graphics.h"
#include "images/image_file.h"

#include "time.h"

using namespace undicht;
using namespace graphics;

// root dir of the hello world example
const std::string PROJECT_DIR = std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/')) + "/../";
const int MAX_FRAMES_IN_FLIGHT = 2;

int main() {

	WindowAPI window_api;

	Window window("Hello World");

	GraphicsAPI graphics_api;
	GraphicsSurface canvas = graphics_api.createGraphicsSurface(window);
	GraphicsDevice gpu = graphics_api.getGraphicsDevice(canvas);
	SwapChain swap_chain = graphics_api.createSwapChain(gpu, canvas);
	swap_chain.setMaxFramesInFlight(MAX_FRAMES_IN_FLIGHT);

	UND_LOG << "using graphics api: vulkan\n";
	UND_LOG << "using gpu: " << gpu.info() << " score: " << graphics_api.rateDevice(gpu) << "\n";

	Shader shader = gpu.createShader();
	shader.loadBinaryFile(PROJECT_DIR + "res/vert.spv", UND_VERTEX_SHADER);
	shader.loadBinaryFile(PROJECT_DIR + "res/frag.spv", UND_FRAGMENT_SHADER);
	shader.linkStages();

    VertexBuffer vbo = gpu.createVertexBuffer();
    vbo.setVertexAttribute(0, UND_VEC3F); // position
    vbo.setVertexAttribute(1, UND_VEC2F); // uv
    vbo.setVertexData({
        -0.5f,-0.5f, 0.0f,  0.0f, 1.0f, // top left
        0.5f,-0.5f, 0.0f,  1.0f, 1.0f, // top right
        0.5f, 0.0f, 0.0f,  1.0f, 0.0f, // bottom right
        -0.5f, 0.0f, 0.0f,  0.0f, 0.0f
    });// bottom left

    vbo.setIndexData({0, 1, 2, 2, 3, 0});
    vbo.setInstanceAttribute(0, UND_VEC2F); // instance position
    vbo.setInstanceData({0.0f, 0.0f}, 0);
    vbo.setInstanceData({0.5f, 0.6f}, 2 * sizeof(float));

	Renderer renderer = gpu.createRenderer();
    renderer.setMaxFramesInFlight(MAX_FRAMES_IN_FLIGHT);
    renderer.setVertexBufferLayout(vbo);
	renderer.setShader(&shader);
    renderer.setShaderInput(1, 0);
	renderer.setRenderTarget(&swap_chain);
	renderer.linkPipeline();

    UniformBuffer uniforms = renderer.createUniformBuffer();
    uniforms.setMaxFramesInFlight(MAX_FRAMES_IN_FLIGHT); // has an internal buffer for every frame
    uniforms.setAttribute(0, UND_FLOAT32); // time
    uniforms.setAttribute(1, UND_VEC2F); // var
    uniforms.setAttribute(2, UND_VEC4F); // color
    uniforms.finalizeLayout();

    Texture texture = gpu.createTexture();
    tools::ImageFile(PROJECT_DIR + "res/Tux.jpg", texture);

    while(!window.shouldClose()) {

		// begin new frame
		uint32_t current_frame = swap_chain.beginFrame();
        renderer.setCurrentFrameID(current_frame);
        uniforms.setCurrentFrame(current_frame); // updating the buffer for the current frame

        // updating the uniform buffer
        std::array<float, 4> pos = {0.2f, 0.0f, 0.3f, 0.0f};
        float t = (time(0) % 60) / 30.0f - 1.0f;
        uniforms.setData(0, &t, sizeof(t));
        uniforms.setData(2, pos.data(), pos.size() * sizeof(float));

		// draw
        renderer.submit(&vbo);
        renderer.submit(&uniforms, 0);
        renderer.draw();

		// present
		swap_chain.endFrame();
        window.update();

        // checking for window resize
        if(window.hasResized()) {
            UND_LOG << "resizing window to : " << window.getWidth() << " * " << window.getHeight() << "\n";
            canvas.matchWindowExtent(window);
            swap_chain.matchSurfaceExtent(canvas);
            renderer.updateRenderTarget(&swap_chain);
        }

        // checking if the window is minimized
        if(window.isMinimized()) {
            window.update();
        }

	}

	gpu.waitForProcessesToFinish();

	return 0;
}
