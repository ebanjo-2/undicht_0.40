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

          public:

            vk::PhysicalDevice* m_physical_device = 0;
            vk::Device * m_device = 0;

            // max frames in flight
            uint32_t m_max_frames_in_flight = 2;
            uint32_t m_current_frame = 0;

            // queues
            float m_queue_priority = 1.0f;
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
            ~GraphicsDevice();

        public:
            // interface

            // changing the max frames count after other graphics objects that depend on it were created is not supported
            void setMaxFramesInFlight(uint32_t max_frames);
            uint32_t getMaxFramesInFlight() const;

            uint32_t beginFrame();
            void endFrame();

            uint32_t getCurrentFrameID() const;

            std::string info() const;
            void waitForProcessesToFinish();

        private:
            // initializing the GraphicsDevice

            void initLogicalDevice(const std::vector<const char*>& extensions);

            std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos();
            vk::PhysicalDeviceFeatures getDeviceFeatures();
            vk::DeviceCreateInfo getDeviceCreateInfo(std::vector<vk::DeviceQueueCreateInfo>* queue_infos, const std::vector<const char*>* extensions, vk::PhysicalDeviceFeatures* features);

            void initQueueHandles();
            void initCmdPools();

        public:
            // interface for other vulkan class

            // finds the right memory type for your needs
            uint32_t findMemory(const vk::MemoryType& type) const;

            vk::CommandBuffer beginSingleTimeCommand(vk::CommandPool cmd_pool) const;
            void endSingleTimeCommand(vk::CommandBuffer cmd_buffer, vk::CommandPool cmd_pool, vk::Queue queue) const;

            uint32_t getAnisotropyLimit() const;

        public:
            // creating objects on the gpu

            template<typename T>
            T create() const {
                return T(this);
            }

        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_DEVICE_H
