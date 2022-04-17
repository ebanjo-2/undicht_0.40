#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

#include "string"

#include "vulkan_declaration.h"

#include "window/glfw/window.h"
#include "graphics_device.h"
#include "graphics_surface.h"
#include "swap_chain.h"

namespace undicht {

    namespace graphics {

        class GraphicsAPI {

          private:

            vk::Instance* m_instance = 0;

          public:

            GraphicsAPI();
            ~GraphicsAPI();
			
            std::string info() const;

          public:
			// creating a graphics device

            uint32_t getGraphicsDeviceCount() const;
            GraphicsDevice getGraphicsDevice(GraphicsSurface& surface, bool choose_best = true, uint32_t id = 0) const;

            uint32_t rateDevice(const GraphicsDevice& device) const;

          private:

            uint32_t rateDevice(vk::PhysicalDevice* device) const;
			bool findQueueFamilies(vk::PhysicalDevice* device, vk::SurfaceKHR* surface, QueueFamilyIDs& ids) const;			
			bool checkDeviceExtensions(vk::PhysicalDevice* device) const;
		  public:
			// creating a graphics surface
			
			GraphicsSurface createGraphicsSurface(const Window& window);

		  public:
			// creating a swap chain
			
			SwapChain createSwapChain(GraphicsDevice& device, GraphicsSurface& surface) const;

        };

    } // namespace graphics

} // namespace undicht

#endif // GRAPHICS_API_H
