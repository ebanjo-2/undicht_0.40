#include "graphics_device.h"

#include <set>
#include <vector>

#include "vulkan/vulkan.hpp"

#include "debug.h"

namespace undicht {

    namespace graphics {

        GraphicsDevice::GraphicsDevice(vk::PhysicalDevice device, vk::SurfaceKHR* surface, QueueFamilyIDs queue_families, const std::vector<const char*>& extensions) {

            m_physical_device = new vk::PhysicalDevice;
            *m_physical_device = device;

            m_graphics_queue = new vk::Queue;
            m_present_queue = new vk::Queue;
            m_transfer_queue = new vk::Queue;

			m_graphics_queue_id = queue_families.graphics_queue;
            m_present_queue_id = queue_families.present_queue;
            m_transfer_queue_id = queue_families.transfer_queue;

            m_graphics_command_pool = new vk::CommandPool;
            m_transfer_command_pool = new vk::CommandPool;

            // getting info about which queue families the logical device needs
            float priority = 1.0f;
            std::vector<vk::DeviceQueueCreateInfo> queue_infos = getQueueCreateInfos(&priority);

            // specifying which device features are required
	   		vk::PhysicalDeviceFeatures features;
            features.samplerAnisotropy = VK_TRUE;

            // creating the logical device
	        vk::DeviceCreateInfo info({}, queue_infos.size(), queue_infos.data(), 0, {}, extensions.size(), extensions.data(), &features);
            m_device = new vk::Device;
            *m_device = m_physical_device->createDevice(info);

			retrieveQueueHandles();
            initCmdPools();
		}

        GraphicsDevice::~GraphicsDevice() {

            m_device->destroyCommandPool(*m_graphics_command_pool);
            m_device->destroyCommandPool(*m_transfer_command_pool);

            delete m_graphics_command_pool;
            delete m_transfer_command_pool;

            delete m_graphics_queue;
            delete m_present_queue;
            delete m_transfer_queue;

            m_device->destroy();
            delete m_device;
            delete m_physical_device;

        }


        /////////////////////////////// initializing the GraphicsDevice //////////////////////////

        void GraphicsDevice::retrieveQueueHandles() {
            // getting the queue handles from the logical device

            m_device->getQueue(m_graphics_queue_id, 0, m_graphics_queue);
            m_device->getQueue(m_present_queue_id, 0, m_present_queue);
            m_device->getQueue(m_transfer_queue_id, 0, m_transfer_queue);
        }

        std::vector<vk::DeviceQueueCreateInfo> GraphicsDevice::getQueueCreateInfos(float* priority) {

            // only requesting unique queue ids
            m_unique_queue_family_ids = {m_graphics_queue_id, m_present_queue_id, m_transfer_queue_id};
            std::vector<vk::DeviceQueueCreateInfo> queue_infos;

            for(const uint32_t& id : m_unique_queue_family_ids) {

                queue_infos.emplace_back(vk::DeviceQueueCreateInfo({}, id, 1, priority));
            }

            return queue_infos;
        }

        void GraphicsDevice::initCmdPools() {

            vk::CommandPoolCreateInfo cmd_pool_info;
            cmd_pool_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

            // creating the graphics command pool
            cmd_pool_info.setQueueFamilyIndex(m_graphics_queue_id);
            *m_graphics_command_pool = m_device->createCommandPool(cmd_pool_info);

            // creating the transfer command pool
            cmd_pool_info.setQueueFamilyIndex(m_transfer_queue_id);
            *m_transfer_command_pool = m_device->createCommandPool(cmd_pool_info);

        }

        ////////////////////////////// interface for other vulkan class //////////////////////////////

        uint32_t GraphicsDevice::findMemory(const vk::MemoryType& type) const {
            // finds the right memory type for your needs

            // getting the physical devices memory properties
            vk::PhysicalDeviceMemoryProperties properties;
            properties = m_physical_device->getMemoryProperties();

            // searching for the right memory
            for(int i = 0; i < properties.memoryTypeCount; i++) {

                if(!((1 << i) & type.heapIndex))
                    continue; // heapIndex is used as a bitfield which specifies the types that can be used

                if(properties.memoryTypes.at(i).propertyFlags == type.propertyFlags)
                    return i;

            }

            UND_ERROR << "failed to find the right type of vram\n";
            return 0;
        }

        vk::CommandBuffer GraphicsDevice::beginSingleTimeCommand(vk::CommandPool cmd_pool) const {

            vk::CommandBufferAllocateInfo info(cmd_pool);
            info.level = vk::CommandBufferLevel::ePrimary;
            info.commandBufferCount =  1;
            vk::CommandBuffer cmd_buffer = m_device->allocateCommandBuffers(info)[0];

            vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            cmd_buffer.begin(begin_info);

            return cmd_buffer;
        }

        void GraphicsDevice::endSingleTimeCommand(vk::CommandBuffer cmd_buffer, vk::CommandPool cmd_pool, vk::Queue queue) const {

            // ending & submitting the command buffer
            cmd_buffer.end();
            vk::SubmitInfo submit_info({},{},cmd_buffer, {});
            queue.submit(submit_info);
            queue.waitIdle();

            m_device->freeCommandBuffers(cmd_pool, cmd_buffer);

        }

        uint32_t GraphicsDevice::getAnisotropyLimit() const {

            vk::PhysicalDeviceProperties properties;
            properties = m_physical_device->getProperties();

            return properties.limits.maxSamplerAnisotropy;
        }

        //////////////////////////////////////////////// interface //////////////////////////////////////////////

        std::string GraphicsDevice::info() const {

            vk::PhysicalDeviceProperties properties;
            m_physical_device->getProperties(&properties);

            return properties.deviceName;
        }

		void GraphicsDevice::waitForProcessesToFinish() {

			// waiting for all processes to stop
			m_device->waitIdle();

		}


        ///////////////////////////////////////// creating objects on the gpu /////////////////////////////////////

		Shader GraphicsDevice::createShader() const {

			return Shader(m_device); 
		}

		Renderer GraphicsDevice::createRenderer() const {

			return Renderer(this);
		}

        VertexBuffer GraphicsDevice::createVertexBuffer() const {

            return VertexBuffer(this);
        }

        Texture GraphicsDevice::createTexture() const {

            return Texture(this);
        }

    }

} // namespace undicht
