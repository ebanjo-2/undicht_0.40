#ifndef GRAPHICS_DEVICE_H
#define GRAPHICS_DEVICE_H

#include "string"
#include "vector"
#include "set"

#include "vulkan_declaration.h"

#include "graphics_pipeline/vulkan/shader.h"
#include "graphics_pipeline/vulkan/renderer.h"
#include "graphics_pipeline/vulkan/vertex_buffer.h"
#include "graphics_pipeline/vulkan/uniform_buffer.h"


namespace undicht {

    namespace graphics {

		struct QueueFamilyIDs {
			uint32_t graphics_queue = 0;
			uint32_t present_queue = 0;
            uint32_t transfer_queue = 0;
		};

        class GraphicsAPI;
		class GraphicsSurface;
		class Renderer;
		class SwapChain;
        class VertexBuffer;
        class VramBuffer;
        class UniformBuffer;

        class GraphicsDevice {

          private:

            friend GraphicsAPI;
			friend GraphicsSurface;
			friend Renderer;
			friend SwapChain;
            friend VertexBuffer;
            friend VramBuffer;
            friend UniformBuffer;


            vk::PhysicalDevice* m_physical_device = 0;
            vk::Device * m_device = 0;

            // queues
            vk::Queue* m_graphics_queue = 0;
            uint32_t m_graphics_queue_id = 0;
            vk::Queue* m_present_queue = 0;
            uint32_t m_present_queue_id = 0;
            vk::Queue* m_transfer_queue = 0;
            uint32_t m_transfer_queue_id = 0;

			std::set<uint32_t> m_unique_queue_family_ids;

            // only the graphics api can create GraphicsDevice objects
            GraphicsDevice(vk::PhysicalDevice device, vk::SurfaceKHR* surface, QueueFamilyIDs queue_families, const std::vector<const char*>& extensions);

            void retrieveQueueHandles();
            std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(float* priority);

        public:

            ~GraphicsDevice();

        public:
            // interface

            std::string info() const;

			void waitForProcessesToFinish();

        public:
            // creating objects on the gpu

			Shader createShader() const;
			Renderer createRenderer() const;
            VertexBuffer createVertexBuffer() const;
            UniformBuffer createUniformBuffer() const;

          private:


        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_DEVICE_H
