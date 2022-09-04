#include "iostream"
#include "undicht_graphics.h"
#include "3D/camera/perspective_camera_3d.h"

using namespace undicht;
using namespace graphics;
using namespace tools;

const std::string PROJECT_DIR = std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/')) + "/../";


int main() {

    // setting up a window that can be drawn to
    WindowAPI window_api;
    Window window("Sponza Test Scene", 1200, 1000);
    GraphicsAPI graphics_api;
    GraphicsSurface canvas = graphics_api.createGraphicsSurface(window);
    GraphicsDevice gpu = graphics_api.getGraphicsDevice(canvas);
    SwapChain swap_chain = graphics_api.createSwapChain(gpu, canvas);

    // loading the sponza model
    VertexBuffer vbo = gpu.create<VertexBuffer>();
    vbo.setVertexAttribute(0, UND_VEC3F); // position
    vbo.setVertexAttribute(1, UND_VEC2F); // uv
    vbo.setVertexData({
            -0.5f,-0.5f, 0.0f,  0.0f, 1.0f, // top left
            0.5f,-0.5f, 0.0f,  1.0f, 1.0f, // top right
            0.5f, 0.0f, 0.0f,  1.0f, 0.0f, // bottom right
            -0.5f, 0.0f, 0.0f,  0.0f, 0.0f, // bottom left
            -1.5f,-0.5f, -1.0f,  0.0f, 1.0f, // top left
            1.5f,-0.5f, -1.0f,  1.0f, 1.0f, // top right
            1.5f, 0.0f, -1.0f,  1.0f, 0.0f, // bottom right
            -1.5f, 0.0f, -1.0f,  0.0f, 0.0f, // bottom left
    });
    vbo.setIndexData({0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4});

    // setting up a 3D renderer
    Shader shader = gpu.create<Shader>();
    shader.loadBinaryFile(PROJECT_DIR + "res/shader/vert.spv", UND_VERTEX_SHADER);
    shader.loadBinaryFile(PROJECT_DIR + "res/shader/frag.spv", UND_FRAGMENT_SHADER);
    shader.linkStages();

    Renderer renderer = gpu.create<Renderer>();
    renderer.setVertexBufferLayout(vbo);
    renderer.setShader(&shader);
    renderer.setShaderInput(1, 1);
    renderer.submit(&swap_chain.getVisibleFramebuffer());
    renderer.setDepthTest(true, true);
    renderer.linkPipeline();

    UniformBuffer uniforms = gpu.create<UniformBuffer>();
    uniforms.setAttribute(0, UND_MAT4F); // proj
    uniforms.setAttribute(1, UND_MAT4F); // view
    uniforms.finalizeLayout();

    PerspectiveCamera3D cam;

    // main loop
    while(!window.shouldClose()) {

        gpu.beginFrame();
        swap_chain.acquireNextImage({&renderer});

        // moving the camera
        cam.setPosition(cam.getPosition() + glm::vec3(0.0f, 0.0f, 0.01f));

        // updating the ubo
        uniforms.setData(0, glm::value_ptr(cam.getCameraProjectionMatrix()), 16 * sizeof(float));
        uniforms.setData(1, glm::value_ptr(cam.getViewMatrix()), 16 * sizeof(float));

        // drawing
        renderer.submit(&uniforms, 0);
        renderer.submit(&vbo);
        renderer.draw();

        swap_chain.presentImage();
        gpu.endFrame();

        window.update();
    }

    gpu.waitForProcessesToFinish();

    return 0;
}