#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

#include "string"

#include "vulkan_declaration.h"

#include "graphics_device.h"

namespace undicht {

    namespace graphics {

        class GraphicsAPI {

          private:

            vk::Instance* m_instance = 0;

          public:

            GraphicsAPI();
            ~GraphicsAPI();

            uint32_t getGraphicsDeviceCount() const;
            GraphicsDevice getGraphicsDevice(bool choose_best = true, uint32_t id = 0) const;

            std::string info() const;

          private:

            uint32_t rateDevice(vk::PhysicalDevice* device) const;

        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_API_H