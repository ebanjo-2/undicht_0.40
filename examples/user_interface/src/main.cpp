#include "undicht_graphics.h"
#include "fonts/true_type.h"

using namespace undicht;
using namespace graphics;

int main() {

    WindowAPI window_api;
    Window window("User Interface Test", 1200, 1000);
    GraphicsAPI graphics_api;
    GraphicsSurface canvas = graphics_api.createGraphicsSurface(window);
    GraphicsDevice gpu = graphics_api.getGraphicsDevice(canvas);
    SwapChain swap_chain = graphics_api.createSwapChain(gpu, canvas);

    Font font = gpu.create<Font>();
    tools::TrueType(UND_DEFAULT_FONT, font);

    VertexBuffer text_data = gpu.create<VertexBuffer>();
    font.buildString("Hello World!!", text_data);

    FontRenderer font_renderer = gpu.create<FontRenderer>();
    font_renderer.setVertexBufferLayout(text_data);
    font_renderer.submit(&swap_chain.getVisibleFramebuffer());
    font_renderer.linkPipeline();

    while(!window.shouldClose()) {

        gpu.beginFrame();
        swap_chain.acquireNextImage({&font_renderer});

        font_renderer.submit(&text_data);
        font_renderer.submit(&font.m_font_map, 0);
        font_renderer.draw();

        swap_chain.presentImage();
        gpu.endFrame();

        window.update();
    }


    return 0;
}