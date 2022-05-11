#include "font_renderer.h"

const std::string FILE_DIR = std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/'));

namespace undicht {

    namespace graphics {

        FontRenderer::FontRenderer(const GraphicsDevice* device) : Renderer(device), m_shader(device) {

            m_shader.loadBinaryFile(FILE_DIR + "/shader/font/vert.spv", UND_VERTEX_SHADER);
            m_shader.loadBinaryFile(FILE_DIR + "/shader/font/frag.spv", UND_FRAGMENT_SHADER);
            m_shader.linkStages();

            setShader(&m_shader);
            setShaderInput(0, 1);
        }

    }

}