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

            // getting info about which queue families the logical device needs
            float priority = 1.0f;
            std::vector<vk::DeviceQueueCreateInfo> queue_infos = getQueueCreateInfos(&priority);

            // specifying which device features are required
	   		vk::PhysicalDeviceFeatures features;

            // creating the logical device
	        vk::DeviceCreateInfo info({}, queue_infos.size(), queue_infos.data(), 0, {}, extensions.size(), extensions.data(), &features);
            m_device = new vk::Device;
            *m_device = m_physical_device->createDevice(info);

			retrieveQueueHandles();
		}

        GraphicsDevice::~GraphicsDevice() {

            delete m_graphics_queue;
            delete m_present_queue;
            delete m_transfer_queue;

            m_device->destroy();
            delete m_device;
            delete m_physical_device;

        }

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

    }

} // namespace undicht
