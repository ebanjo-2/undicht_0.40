#include "vulkan/vulkan.hpp"

#include "graphics_device.h"

namespace undicht {

    namespace graphics {

        GraphicsDevice::GraphicsDevice(vk::PhysicalDevice device) {

            m_physical_device = new vk::PhysicalDevice;
            *m_physical_device = device;

        }

        GraphicsDevice::~GraphicsDevice() {

            if(m_physical_device)
                delete m_physical_device;

        }

        std::string GraphicsDevice::info() const {

            vk::PhysicalDeviceProperties properties;
            m_physical_device->getProperties(&properties);

            return properties.deviceName;
        }

    }

} // namespace undicht