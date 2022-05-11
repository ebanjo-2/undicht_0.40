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
#include "graphics_pipeline/vulkan/texture.h"

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
        class Texture;
        class Shader;

        class GraphicsDevice {

          private:

            friend GraphicsAPI;
			friend GraphicsSurface;
			friend Renderer;
			friend SwapChain;
            friend VertexBuffer;
            friend VramBuffer;
            friend UniformBuffer;
            friend Texture;
            friend Shader;

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

            // command pools
            vk::CommandPool* m_graphics_command_pool = 0; // commands for the graphics queue
            vk::CommandPool* m_transfer_command_pool = 0; // commands for the transfer queue

            // only the graphics api can create GraphicsDevice objects
            GraphicsDevice(vk::PhysicalDevice device, vk::SurfaceKHR* surface, QueueFamilyIDs queue_families, const std::vector<const char*>& extensions);

        public:

            ~GraphicsDevice();

        private:
            // initializing the GraphicsDevice

            void retrieveQueueHandles();

            std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(float* priority);

            void initCmdPools();

        private:
            // interface for other vulkan class

            // finds the right memory type for your needs
            uint32_t findMemory(const vk::MemoryType& type) const;
            vk::CommandBuffer beginSingleTimeCommand(vk::CommandPool cmd_pool) const;
            void endSingleTimeCommand(vk::CommandBuffer cmd_buffer, vk::CommandPool cmd_pool, vk::Queue queue) const;

            uint32_t getAnisotropyLimit() const;

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
            Texture createTexture() const;

            template<typename T>
            T create() const {
                return T(this);
            }

        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_DEVICE_H
