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
		class GraphicsDevice;

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
			uint32_t m_current_image = 0;
			uint32_t m_max_frames_in_flight = 2;
			uint32_t m_current_frame = 0;

			// sync objects (one for each frame in flight)
			std::vector<vk::Semaphore>* m_image_available = 0;
			std::vector<vk::Semaphore>* m_render_finished = 0;
			std::vector<vk::Fence>* m_frame_in_flight = 0;

			
			// handles for objects needed to update the swap chain
			friend GraphicsAPI;
			friend Renderer;
            vk::PhysicalDevice* m_physical_device_handle = 0;
			vk::Device* m_device_handle = 0;
			vk::Queue* m_present_queue_handle = 0;
            const GraphicsSurface* m_surface_handle = 0;

			SwapChain(GraphicsDevice* device, GraphicsSurface* surface);

        public:

            ~SwapChain();

        private:

            // destroys all vulkan objects, but keeps the settings
            // you then can change some of the settings, and update() the swap chain
            // to use it again
            void cleanUp();

        private:
            // creating the swap chain

            bool checkDeviceCapabilities(vk::PhysicalDevice* device, vk::SurfaceKHR* surface);
            void chooseSwapImageFormat();
            void choosePresentMode();

			void getSupportDetails(vk::PhysicalDevice* device, vk::SurfaceKHR* surface);
			bool isFormatSupported(vk::SurfaceFormatKHR* format_khr) const;
			bool isPresentModeSupported(vk::PresentModeKHR* mode) const;
			uint32_t findImageCount() const; // determines the amount of images in the swap chain
            void initSyncObjects();

        private:
            // recreating the swap chain

			// updates the vk::SwapChain to represent the changes made
			void update();

            void retrieveSwapImages();

		public:

            // to be called when the window was resized
            void matchSurfaceExtent(const GraphicsSurface& surface);

			void setExtent(uint32_t width, uint32_t height);
			void getExtent(uint32_t& width, uint32_t & height) const;
			uint32_t getWidth() const;
			uint32_t getHeight() const;

			void setMaxFramesInFlight(uint32_t count);
			uint32_t getMaxFramesInFlight() const;
			uint32_t getCurrentFrameID() const;
            int getCurrentImageID() const;

			uint32_t beginFrame();
			void endFrame();
	
		};

	}

} // undicht

#endif // SWAP_CHAIN_H
