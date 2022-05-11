#include "undicht_graphics.h"
#include "fonts/true_type.h"

const std::string PROJECT_DIR = std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/')) + "/../";
const int MAX_FRAMES_IN_FLIGHT = 2;

using namespace undicht;
using namespace graphics;

int main() {

    WindowAPI window_api;
    Window window("User Interface Test", 800, 600);
    GraphicsAPI graphics_api;
    GraphicsSurface canvas = graphics_api.createGraphicsSurface(window);
    GraphicsDevice gpu = graphics_api.getGraphicsDevice(canvas);
    SwapChain swap_chain = graphics_api.createSwapChain(gpu, canvas);
    swap_chain.setMaxFramesInFlight(MAX_FRAMES_IN_FLIGHT);

    Font font = gpu.create<Font>();
    tools::TrueType(PROJECT_DIR + "res/FreeMono.ttf", font);

    VertexBuffer text_data = gpu.create<VertexBuffer>();
    font.buildString("Hello World !!", text_data);

    FontRenderer font_renderer = gpu.create<FontRenderer>();
    font_renderer.setMaxFramesInFlight(MAX_FRAMES_IN_FLIGHT);
    font_renderer.setVertexBufferLayout(text_data);
    font_renderer.setRenderTarget(&swap_chain);
    font_renderer.linkPipeline();

    while(!window.shouldClose()) {

        uint32_t frame_id = swap_chain.beginFrame();
        font_renderer.setCurrentFrameID(frame_id);

        font_renderer.submit(&text_data);
        font_renderer.submit(&font.m_font_map, 0);
        font_renderer.draw();

        swap_chain.endFrame();

        window.update();
    }


    return 0;
}