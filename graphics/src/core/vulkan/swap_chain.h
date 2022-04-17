#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#include "vulkan_declaration.h"
#include <vulkan/vulkan.hpp>

namespace undicht {

	namespace graphics {
		
		class GraphicsAPI;
		class GraphicsSurface;

		class SwapChain {

		private:

			// swap chain support details
			vk::SurfaceCapabilitiesKHR* m_capabilities = 0;
			std::vector<vk::SurfaceFormatKHR>* m_formats = 0;
			std::vector<vk::PresentModeKHR>* m_present_modes = 0;

			// settings
			vk::SurfaceFormatKHR* m_format = 0;
			vk::PresentModeKHR* m_present_mode = 0;

			vk::SwapChain* m_swap_chain;

			friend GraphicsAPI;

			SwapChain(vk::PhysicalDevice* device, vk::SurfaceKHR* surface);

			void getSupportDetails(vk::PhysicalDevice* device, vk::SurfaceKHR* surface);
			bool isFormatSupported(vk::SurfaceFormatKHR* format_khr);
			bool isPresentModeSupported(vk::PresentModeKHR* mode);

		public:

			~SwapChain();

		};

	}

} // undicht

#endif // SWAP_CHAIN_H
