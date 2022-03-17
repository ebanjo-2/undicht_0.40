#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

#include "string"

#include "vulkan_declaration.h"

namespace undicht {

    namespace graphics {

        class GraphicsAPI {

          private:

            vk::Instance* m_instance = 0;

          public:

            GraphicsAPI();
            ~GraphicsAPI();

            std::string info() const;
        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_API_H