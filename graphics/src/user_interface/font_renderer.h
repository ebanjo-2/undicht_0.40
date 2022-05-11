#ifndef FONT_RENDERER_H
#define FONT_RENDERER_H

#include "font.h"
#include "undicht_graphics.h"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;

        class FontRenderer : public Renderer {

        public:

            Shader m_shader;

            FontRenderer(const GraphicsDevice* device);

        };


    }

}

#endif // FONT_RENDERER