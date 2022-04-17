#include "graphics_device.h"

#include <set>
#include <vector>

#include "vulkan/vulkan.hpp"

#include "debug.h"

namespace undicht {

    namespace graphics {

        GraphicsDevice::GraphicsDevice(vk::PhysicalDevice* device, vk::SurfaceKHR* surface, QueueFamilyIDs* queue_families, const std::vector<const char*>& extensions) {

            m_physical_device = new vk::PhysicalDevice;
            *m_physical_device = *device;
			
			m_queue_family_ids = *queue_families;

			// only requesting unique queue ids
			std::set<uint32_t> queue_familiy_ids = {m_queue_family_ids.graphics_queue, m_queue_family_ids.present_queue};
			std::vector<vk::DeviceQueueCreateInfo> queue_infos;

			float priority = 1.0f; // queue priority

			for(const uint32_t& id : queue_familiy_ids) {

				queue_infos.emplace_back(vk::DeviceQueueCreateInfo({}, id, 1, &priority));
			}


            // specifying which device features are required
	   		vk::PhysicalDeviceFeatures features;

            // creating the logical device
            m_device = new vk::Device;
	        vk::DeviceCreateInfo info({}, queue_infos.size(), queue_infos.data(), 0, {}, extensions.size(), extensions.data(), &features);                                                        
            *m_device = m_physical_device->createDevice(info);

			retrieveQueueHandles();
		}

        GraphicsDevice::~GraphicsDevice() {

			if(m_queues.graphics_queue)
				delete m_queues.graphics_queue;

			if(m_queues.present_queue)
				delete m_queues.present_queue;

            if(m_device){
        		m_device->destroy();
                delete m_device;
	   		}

            if(m_physical_device)
                delete m_physical_device;

        }

        std::string GraphicsDevice::info() const {

            vk::PhysicalDeviceProperties properties;
            m_physical_device->getProperties(&properties);

            return properties.deviceName;
        }

		void GraphicsDevice::retrieveQueueHandles() {
			// getting the queue handles from the logical device
			
			m_queues.graphics_queue = new vk::Queue;
			m_queues.present_queue = new vk::Queue;

			m_device->getQueue(m_queue_family_ids.graphics_queue, 0, m_queues.graphics_queue);
		    m_device->getQueue(m_queue_family_ids.present_queue, 0, m_queues.present_queue); 
			
		}	

    }

} // namespace undicht
