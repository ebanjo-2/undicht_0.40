#ifndef GRAPHICS_DEVICE_H
#define GRAPHICS_DEVICE_H

#include "string"

#include "vulkan_declaration.h"

namespace undicht {

    namespace graphics {

        class GraphicsAPI;

        class GraphicsDevice {

          private:

            friend GraphicsAPI;


            vk::PhysicalDevice* m_physical_device = 0;
            vk::Device * m_device = 0;

            struct {
                uint32_t graphics_queue;
                uint32_t present_queue;
            } m_queues;


            // only the graphics api can create GraphicsDevice objects
            GraphicsDevice(vk::PhysicalDevice device);

          public:

            ~GraphicsDevice();

            std::string info() const;

          private:

            bool findQueueFamilies();

        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_DEVICE_H
