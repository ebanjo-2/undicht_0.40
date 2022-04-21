#ifndef GRAPHICS_DEVICE_H
#define GRAPHICS_DEVICE_H

#include "string"
#include "vector"
#include "set"

#include "vulkan_declaration.h"

#include "graphics_pipeline/vulkan/shader.h"
#include "graphics_pipeline/vulkan/renderer.h"


namespace undicht {

    namespace graphics {

		struct QueueFamilyIDs {
			uint32_t graphics_queue = 0;
			uint32_t present_queue = 0;
		};

        class GraphicsAPI;
		class GraphicsSurface;

        class GraphicsDevice {

          private:

            friend GraphicsAPI;
			friend GraphicsSurface;


            vk::PhysicalDevice* m_physical_device = 0;
            vk::Device * m_device = 0;

         	QueueFamilyIDs m_queue_family_ids;
			std::set<uint32_t> m_unique_queue_family_ids;

			struct {
				vk::Queue* graphics_queue;
				vk::Queue* present_queue;
			} m_queues;

            // only the graphics api can create GraphicsDevice objects
            GraphicsDevice(vk::PhysicalDevice* device, vk::SurfaceKHR* surface, QueueFamilyIDs* queue_families, const std::vector<const char*>& extensions);

          public:

            ~GraphicsDevice();

            std::string info() const;

			Shader createShader() const;
			Renderer createRenderer() const;

          private:

			void retrieveQueueHandles();
        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_DEVICE_H
