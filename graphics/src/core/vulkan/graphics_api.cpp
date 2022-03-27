#include "graphics_api.h"

// only here to convince vscode everything is fine
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

namespace undicht {

    namespace graphics {

        GraphicsAPI::GraphicsAPI() {

            // extensions
            uint32_t ext_count = 0;
            const char **extns = glfwGetRequiredInstanceExtensions(&ext_count);

            // validation layers
            uint32_t layer_count = 1;
            const char* layers = "VK_LAYER_KHRONOS_validation";

            // instance create structs
            vk::ApplicationInfo app_info("undicht");
            vk::InstanceCreateInfo create_info({}, &app_info, layer_count, &layers, ext_count, extns);
            
            // creating the instance
            m_instance = new vk::Instance;
            *m_instance = vk::createInstance(create_info);

        }

        GraphicsAPI::~GraphicsAPI() {

            if(m_instance) {
                m_instance->destroy();
                delete m_instance;
            }

        }

        uint32_t GraphicsAPI::getGraphicsDeviceCount() const {
            
            uint32_t count;
            m_instance->enumeratePhysicalDevices(&count, 0);

            return count;
        }

        GraphicsDevice GraphicsAPI::getGraphicsDevice(bool choose_best, uint32_t id) const {

            // getting the available devices
            uint32_t device_count = getGraphicsDeviceCount();
            std::vector<vk::PhysicalDevice> devices(device_count);
            m_instance->enumeratePhysicalDevices(&device_count, devices.data());

            if(choose_best) {
                // rating the devices
                uint32_t best = 0;
                uint32_t best_score = 0;

                for(int i = 0; i < devices.size(); i++) {

                    uint32_t device_score = rateDevice(&devices.at(i));

                    if(device_score > best_score) {
                        best = i;
                        best_score = device_score;
                    }

                }
                // choosing the best
                return GraphicsDevice(devices.at(best));
            } else {

                return GraphicsDevice(devices.at(id));
            }

        }

        std::string GraphicsAPI::info() const {

            return "vulkan";
        }

        uint32_t GraphicsAPI::rateDevice(vk::PhysicalDevice* device) const {

            uint32_t score = 0;

            // getting features and properties
            vk::PhysicalDeviceProperties properties;
            vk::PhysicalDeviceFeatures features;
            device->getProperties(&properties);
            device->getFeatures(& features);

            // Discrete GPUs have a significant performance advantage
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1000;

            // Maximum possible size of textures speeks for a better gpu
            score += properties.limits.maxImageDimension2D;

            // Application can't function without geometry shaders
            if (!features.geometryShader)
                return 0;

            return score;
        }

    } // namespace  graphics

} // namespace undicht