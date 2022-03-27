#ifndef GRAPHICS_DEVICE_H
#define GRAPHICS_DEVICE_H

#include "string"

#include "vulkan_declaration.h"

namespace undicht {

    namespace graphics {

        class GraphicsAPI;

        class GraphicsDevice {

          private:

            vk::PhysicalDevice* m_physical_device = 0;
            vk::Device * m_device = 0;

            friend GraphicsAPI; 

            // only the graphics api can create GraphicsDevice objects
            GraphicsDevice(vk::PhysicalDevice device);

          public:

            ~GraphicsDevice();

            std::string info() const;
        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_DEVICE_H