#include "iostream"
#include "undicht_graphics.h"
#include "3D/camera/perspective_camera_3d.h"
#include "model_loading/collada/collada_file.h"
#include "debug.h"

using namespace undicht;
using namespace graphics;
using namespace tools;

const std::string PROJECT_DIR = std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/')) + "/../";


int main() {

    UND_LOG << "Started the Application\n";

    // setting up a window that can be drawn to
    WindowAPI window_api;
    Window window("Sponza Test Scene", 1200, 900);
    GraphicsAPI graphics_api;
    GraphicsSurface canvas = graphics_api.createGraphicsSurface(window);
    GraphicsDevice gpu = graphics_api.getGraphicsDevice(canvas);
    SwapChain swap_chain = graphics_api.createSwapChain(gpu, canvas);

    // loading the sponza model
    std::vector<MeshData> meshes;
    std::vector<ImageData> images;

    ColladaFile model_file(PROJECT_DIR + "res/sponza_collada/sponza.dae");
    model_file.loadAllMeshes(meshes);
    model_file.loadAllTextures(images);

    UND_LOG << "loaded " << meshes.size() << " meshes + " << images.size() << " textures\n";

    // loading the model to the gpu
    std::vector<VertexBuffer*> vbos(meshes.size(), nullptr);
    std::vector<Texture*> textures(images.size(), nullptr);

    for(int i = 0; i < meshes.size(); i++) {
        MeshData& mesh = meshes.at(i);
        vbos.at(i) = new VertexBuffer(gpu.create<VertexBuffer>());
        vbos.at(i)->setVertexAttribute(0, UND_VEC3F); // position
        vbos.at(i)->setVertexAttribute(1, UND_VEC2F); // uv
        vbos.at(i)->setVertexAttribute(2, UND_VEC3F); // normal
        vbos.at(i)->setVertexData(mesh.vertices);
    }

    for(int i = 0; i < images.size(); i++) {
        ImageData& image = images.at(i);
        textures.at(i) = new Texture(gpu.create<Texture>());
        if(image._nr_channels && image._width && image._height) {
            textures.at(i)->setSize(image._width, image._height);
            textures.at(i)->setFormat(FixedType(Type::COLOR_BGRA, 1, image._nr_channels));
            textures.at(i)->finalizeLayout();
            textures.at(i)->setData(image._pixels.data(), image._pixels.size());
        } else { // texture is missing
            int color_data = 0xFF00A000;
            textures.at(i)->setSize(1, 1);
            textures.at(i)->setFormat(FixedType(Type::COLOR_BGRA, 1, 4));
            textures.at(i)->finalizeLayout();
            textures.at(i)->setData((char*)&color_data, sizeof(color_data));
        }
    }

    UND_LOG << "finished transferring the model to the gpu\n";

    // setting up a 3D renderer
    Shader shader = gpu.create<Shader>();
    shader.loadBinaryFile(PROJECT_DIR + "res/shader/vert.spv", UND_VERTEX_SHADER);
    shader.loadBinaryFile(PROJECT_DIR + "res/shader/frag.spv", UND_FRAGMENT_SHADER);
    shader.linkStages();

    Renderer renderer = gpu.create<Renderer>();
    renderer.setVertexBufferLayout(*vbos.at(0));
    renderer.setShader(&shader);
    renderer.setShaderInput(1, 1);
    renderer.setFramebufferLayout(swap_chain.getVisibleFramebuffer());
    renderer.setDepthTest(true, true);
    renderer.linkPipeline();

    UniformBuffer uniforms = gpu.create<UniformBuffer>();
    uniforms.setAttribute(0, UND_MAT4F); // proj
    uniforms.setAttribute(1, UND_MAT4F); // view
    uniforms.finalizeLayout();

    PerspectiveCamera3D cam;
    cam.setAxesRotation({180.0f, 0.0f, 90.0f});
    cam.setPosition(glm::vec3(0.0f, 10.0f, 0.0f));

    // main loop
    while(!window.shouldClose()) {

        gpu.beginFrame();
        swap_chain.acquireNextImage({&renderer});

        // moving the camera
        cam.setPosition(cam.getPosition() + glm::vec3(0.1f, 0.0f, 0.0f));

        // updating the ubo
        uniforms.setData(0, glm::value_ptr(cam.getCameraProjectionMatrix()), 16 * sizeof(float));
        uniforms.setData(1, glm::value_ptr(cam.getViewMatrix()), 16 * sizeof(float));

        // drawing
        renderer.beginRenderPass(&swap_chain.getVisibleFramebuffer());
        for(int i = 0; i < meshes.size(); i++) {
            renderer.submit(textures.at(meshes.at(i).color_texture), 1);
            renderer.submit(&uniforms, 0);
            renderer.draw(vbos.at(i));
        }
        renderer.endRenderPass();

        swap_chain.presentImage();
        gpu.endFrame();

        window.update();
    }

    gpu.waitForProcessesToFinish();

    for(VertexBuffer* vbo : vbos)
        delete vbo;

    for(Texture* texture : textures)
        delete texture;

    return 0;
}