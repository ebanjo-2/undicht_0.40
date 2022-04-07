#include "graphics_device.h"

#include <set>
#include <vector>

#include "vulkan/vulkan.hpp"

#include "debug.h"

namespace undicht {

    namespace graphics {

        GraphicsDevice::GraphicsDevice(vk::PhysicalDevice device) {

            m_physical_device = new vk::PhysicalDevice;
            *m_physical_device = device;

            // getting queue family ids
            if(!findQueueFamilies())
                UND_ERROR << "failed to find the necessary queue families \n";
			
			// only requesting unique queue ids
			std::set<uint32_t> queue_familiy_ids = {m_queues.graphics_queue}; // , m_queues.present_queue};
			std::vector<vk::DeviceQueueCreateInfo> queue_infos;

			float priority = 0.1f; // queue priority

			for(const uint32_t& id : queue_familiy_ids) {

				queue_infos.emplace_back(vk::DeviceQueueCreateInfo({}, id, 1, &priority));
			}


            // specifying which device features are required
	   		vk::PhysicalDeviceFeatures features;

            // creating the logical device
            m_device = new vk::Device;
	        vk::DeviceCreateInfo info({}, queue_infos.size(), queue_infos.data(), 0, {}, 0, {}, &features);                                                        
            *m_device = m_physical_device->createDevice(info);
        }

        GraphicsDevice::~GraphicsDevice() {

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

        bool GraphicsDevice::findQueueFamilies() {

            bool graphics_queue = false;
            bool present_queue = false;

            std::vector<vk::QueueFamilyProperties> queues;
            queues = m_physical_device->getQueueFamilyProperties();

            // finding a queue family which supports the graphics queue
            for(int i =  0; i < queues.size(); i++) {

                if(queues[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                    m_queues.graphics_queue = i;
                    graphics_queue = true;
                }

            }

            // return true if all queues were found
            return graphics_queue;
        }

    }

} // namespace undicht
