#include "graphics_api.h"

// only here to convince vscode everything is fine
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include <stdint.h>
#include <vector>
#include <set>

#include "debug.h"

namespace undicht {

    namespace graphics {

		// the extensions a graphics device needs to support
		const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};


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

            m_instance->destroy();
            delete m_instance;

        }

        std::string GraphicsAPI::info() const {
  
            return "vulkan";
        }


		//////////////////////////////////// creating a graphics device /////////////////////////

        uint32_t GraphicsAPI::getGraphicsDeviceCount() const {

            return m_instance->enumeratePhysicalDevices().size();
        }

        GraphicsDevice GraphicsAPI::getGraphicsDevice(GraphicsSurface& surface, bool choose_best, uint32_t id) const {

            // getting the available devices
            std::vector<vk::PhysicalDevice> devices = m_instance->enumeratePhysicalDevices();
			QueueFamilyIDs queue_families;

			vk::SurfaceKHR* surf = surface.m_surface;
				
			if(choose_best) {
				//determining the best device
	
				uint32_t highest_score = 0;
				int best_device = 0;

				for(int i = 0; i < devices.size(); i++) {
					
					int score = rateDevice(&devices[i]);
					score *= findQueueFamilies(&devices[i], surf, queue_families);
					score *= checkDeviceExtensions(&devices[i]);

					if(score > highest_score) {
						best_device = i;
						highest_score = score;
					}

				}	
				// the best device has been found
                if(!highest_score)
                    UND_ERROR << "Failed to find a suitable Graphics Device\n";
			}

			vk::PhysicalDevice device = devices.at(id);
			findQueueFamilies(&device, surf, queue_families);

			return GraphicsDevice(device, surf, queue_families, REQUIRED_DEVICE_EXTENSIONS);
        }

        uint32_t GraphicsAPI::rateDevice(const GraphicsDevice& device) const {

            return rateDevice(device.m_physical_device);
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

            if(!features.samplerAnisotropy)
                return 0;

            return score;
        }
	
		bool GraphicsAPI::findQueueFamilies(vk::PhysicalDevice* device, vk::SurfaceKHR* surface, QueueFamilyIDs& ids) const{
			
			bool graphics_queue = false;
			bool present_queue = false;
            bool transfer_queue = false;
	
            std::vector<vk::QueueFamilyProperties> queues = device->getQueueFamilyProperties();				
	
            for(int i =  0; i < queues.size(); i++) {

				// graphics queue
                if((!graphics_queue) && (queues[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
                    ids.graphics_queue = i;
                    graphics_queue = true;
                }

				// present queue
				vk::Bool32 present_support = false;
				device->getSurfaceSupportKHR(i, *surface, &present_support);

				if((!present_queue) && present_support) {
					ids.present_queue = i;
					present_queue = true;	
				}

                // transfer queue (must be different to the graphics queue)
                if((!transfer_queue) && (queues[i].queueFlags & vk::QueueFlagBits::eTransfer) &&  (!(queues[i].queueFlags & vk::QueueFlagBits::eGraphics))) {
                    ids.transfer_queue = i;
                    transfer_queue = true;
                }
            }

			return graphics_queue && present_queue && transfer_queue;
		}

		bool GraphicsAPI::checkDeviceExtensions(vk::PhysicalDevice* device) const{
			
			std::vector<vk::ExtensionProperties> extensions =  device->enumerateDeviceExtensionProperties();

			std::set<std::string> required_extensions(REQUIRED_DEVICE_EXTENSIONS.begin(), REQUIRED_DEVICE_EXTENSIONS.end());
			
			// removing all of the available extensions from the ones that are required
			for(vk::ExtensionProperties& p :extensions)
				required_extensions.erase(p.extensionName);


			return required_extensions.empty();
		}

		/////////////////////////////// creating a graphics surface //////////////////////////////

        GraphicsSurface GraphicsAPI::createGraphicsSurface(const Window& window) {

			return GraphicsSurface(m_instance, window.m_window);
		}
		
		////////////////////////////////// creating a swap chain //////////////////////////////
		
		SwapChain GraphicsAPI::createSwapChain(GraphicsDevice& device, GraphicsSurface& surface) const{

			return SwapChain(&device, &surface);
		}

    } // namespace  graphics

} // namespace undicht
