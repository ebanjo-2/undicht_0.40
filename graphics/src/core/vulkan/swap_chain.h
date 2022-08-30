#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#include "vulkan_declaration.h"
#include "graphics_pipeline/vulkan/framebuffer.h"

#include "vector"
#include "cstdint"

namespace undicht {

	namespace graphics {
		
		class GraphicsAPI;
		class GraphicsSurface;
		class Renderer;
		class GraphicsDevice;
        class Pipeline;

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
			/*std::vector<vk::Image>* m_images = 0;
			std::vector<vk::ImageView>* m_image_views = 0;*/
            std::vector<Texture> m_images;
			uint32_t m_current_image = 0;

            // the visible framebuffer
            Framebuffer m_framebuffer;

			// sync objects (one for each frame in flight)
			/*std::vector<vk::Semaphore>* m_image_available = 0;
			std::vector<vk::Semaphore>* m_render_finished = 0;
			std::vector<vk::Fence>* m_frame_in_flight = 0;*/
			
			// handles for objects needed to update the swap chain
			friend GraphicsAPI;
			friend Renderer;
            friend Pipeline;
            const GraphicsDevice* m_device_handle = 0;
            const GraphicsSurface* m_surface_handle = 0;

        public:

            SwapChain(GraphicsDevice* device, GraphicsSurface* surface);
            ~SwapChain();
            void cleanUp();

        private:
            // creating the swap chain

			void initSwapChain();
            void recreateSwapChain();
            void retrieveSwapImages();
            void initVisibleFramebuffer();

            bool checkDeviceCapabilities(vk::PhysicalDevice* device, vk::SurfaceKHR* surface);
            void chooseSwapImageFormat();
            void choosePresentMode();

			void getSupportDetails(vk::PhysicalDevice* device, vk::SurfaceKHR* surface);
			bool isFormatSupported(vk::SurfaceFormatKHR* format_khr) const;
			bool isPresentModeSupported(vk::PresentModeKHR* mode) const;
			uint32_t findImageCount() const; // determines the amount of images in the swap chain
            //void initSyncObjects();

		public:

            // to be called when the window was resized
            void matchSurfaceExtent(const GraphicsSurface& surface);

			void setExtent(uint32_t width, uint32_t height);
			void getExtent(uint32_t& width, uint32_t & height) const;
			uint32_t getWidth() const;
			uint32_t getHeight() const;

			uint32_t acquireNextImage();
			void presentImage(std::vector<Renderer*> wait_for = {});
            int getCurrentImageID() const;

            Framebuffer& getVisibleFramebuffer();

        };

	}

} // undicht

#endif // SWAP_CHAIN_H
