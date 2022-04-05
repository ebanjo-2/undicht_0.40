#include "vulkan/vulkan.hpp"

#include "debug.h"
#include "graphics_device.h"

namespace undicht {

    namespace graphics {

        GraphicsDevice::GraphicsDevice(vk::PhysicalDevice device) {

            m_physical_device = new vk::PhysicalDevice;
            *m_physical_device = device;

            // getting queue family ids
            if(!findQueueFamilies())
                UND_ERROR << "failed to find the necessary queue families \n";

            // specifying which device features are required
            //vk::	

            // creating the logical device
            m_device = new vk::Device;
	    
	        float priority = 1.0f; // info about the queue families the device will use
	        vk::DeviceQueueCreateInfo graphics_queue_info({}, m_queues.graphics_queue, 1,&priority);

	        vk::DeviceCreateInfo info({}, 1,&graphics_queue_info);                                                        

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
