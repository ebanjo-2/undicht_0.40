#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#include "vulkan_declaration.h"

#include "vector"
#include "cstdint"

namespace undicht {

	namespace graphics {
		
		class GraphicsAPI;
		class GraphicsSurface;
		class Renderer;

		class SwapChain {

		private:

			vk::SwapchainKHR* m_swap_chain = 0;

			// swap chain support details
			vk::SurfaceCapabilitiesKHR* m_capabilities = 0;
			std::vector<vk::SurfaceFormatKHR>* m_formats = 0;
			std::vector<vk::PresentModeKHR>* m_present_modes = 0;

			// settings
			vk::SurfaceFormatKHR* m_format = 0;
			vk::PresentModeKHR* m_present_mode = 0;
			vk::Extent2D* m_extent = 0;
			uint32_t m_image_count = 0;
			std::vector<uint32_t> m_queue_ids; // queue families accessing the swap images

			// swap images
			std::vector<vk::Image>* m_images = 0;
			std::vector<vk::ImageView>* m_image_views = 0;
			
			// handles for objects needed to update the swap chain
			friend GraphicsAPI;
			friend Renderer;
			vk::Device* m_device_handle = 0;
			vk::SurfaceKHR* m_surface_handle = 0;


			SwapChain(vk::PhysicalDevice* device, vk::Device* device_handle, vk::SurfaceKHR* surface);

			void getSupportDetails(vk::PhysicalDevice* device, vk::SurfaceKHR* surface);
			bool isFormatSupported(vk::SurfaceFormatKHR* format_khr) const;
			bool isPresentModeSupported(vk::PresentModeKHR* mode) const;
			uint32_t findImageCount() const; // determines the amount of images in the swap chain
			
			// updates the vk::SwapChain to represent the changes made
			void update();

		public:

			~SwapChain();

			void setExtent(uint32_t width, uint32_t height);
			void getExtent(uint32_t& width, uint32_t & height) const;
			uint32_t getWidth() const;
			uint32_t getHeight() const;
		};

	}

} // undicht

#endif // SWAP_CHAIN_H
