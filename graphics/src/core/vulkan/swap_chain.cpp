#include "swap_chain.h"

#include "core/vulkan/vulkan_declaration.h"
#include "graphics_api.h"
#include "graphics_surface.h"
#include "debug.h"

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_structs.hpp>

#include <algorithm>

namespace undicht {

	namespace graphics {

		SwapChain::SwapChain(GraphicsDevice* device, vk::SurfaceKHR* surface) {
			
			m_device_handle = device->m_device;
			m_present_queue_handle = device->m_queues.present_queue;
			m_surface_handle = surface;

			// checking support details
			m_capabilities = new vk::SurfaceCapabilitiesKHR;
			m_formats = new std::vector<vk::SurfaceFormatKHR>;
			m_present_modes = new std::vector<vk::PresentModeKHR>;

			getSupportDetails(device->m_physical_device, surface);
			
			if((!m_formats->size()) || (!m_present_modes->size())) {
				UND_ERROR << "failed to  create swap chain: device does not support any formats or present modes\n";
				return;
			}

			// choosing the swap chain format	
			vk::SurfaceFormatKHR BGRA_srgb(vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);

			if(isFormatSupported(&BGRA_srgb)) {
				m_format = new vk::SurfaceFormatKHR(BGRA_srgb);
			} else { // fallback: using the first format declared (may be completly wrong)
 				UND_WARNING << "requested surface format is no available\n";
				m_format = new vk::SurfaceFormatKHR(m_formats->at(0));
			}

			// choosing the present mode
			vk::PresentModeKHR immediate(vk::PresentModeKHR::eImmediate);
			
			if(isPresentModeSupported(&immediate)) {
				m_present_mode = new vk::PresentModeKHR(immediate);
			} else { // fallback: vsync is the only present mode guaranteed to exist
				UND_WARNING << "requested present mode is not available\n";
				m_present_mode = new vk::PresentModeKHR(vk::PresentModeKHR::eFifo);
			}

			// determining the image count
			m_image_count = findImageCount();

			// initializing sync objects
			m_image_available = new vk::Semaphore;
			m_render_finished = new vk::Semaphore;
			m_frame_in_flight = new vk::Fence;;

			vk::SemaphoreCreateInfo semaphore_info;
			vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled); // set initial state as set

			*m_image_available = m_device_handle->createSemaphore(semaphore_info);
			*m_render_finished = m_device_handle->createSemaphore(semaphore_info);
			*m_frame_in_flight = m_device_handle->createFence(fence_info);
			
			// initializing other vulkan members
			m_extent = new vk::Extent2D;
			m_images = new std::vector<vk::Image>;
			m_image_views = new std::vector<vk::ImageView>;
			m_swap_chain = new vk::SwapchainKHR;

		}

		SwapChain::~SwapChain() {
		
			if(m_capabilities)
				delete m_capabilities;
				
			if(m_formats)
				delete m_formats;

			if(m_present_modes)
				delete m_present_modes;

			if(m_format)
				delete m_format;

			if(m_present_mode)
				delete m_present_mode;

			if(m_image_available) {
				m_device_handle->destroySemaphore(*m_image_available);
				delete m_image_available;
			}

			if(m_render_finished) {
				m_device_handle->destroySemaphore(*m_render_finished);
				delete m_render_finished;
			}

			if(m_frame_in_flight) {
				m_device_handle->destroyFence(*m_frame_in_flight);
				delete m_frame_in_flight;
			}

			if(m_extent)
				delete m_extent;

			if(m_images)
				delete m_images;

			if(m_image_views) {
				for(vk::ImageView& image_view : (*m_image_views))
					m_device_handle->destroyImageView(image_view);

				delete m_image_views;
			}

			if(m_swap_chain)
				delete m_swap_chain;
		
		}


		void SwapChain::getSupportDetails(vk::PhysicalDevice* device, vk::SurfaceKHR* surface) {
		
			// device capabilities
			*m_capabilities = device->getSurfaceCapabilitiesKHR(*surface);
			
			// supported formats
			*m_formats = device->getSurfaceFormatsKHR(*surface);
		
			// supported present modes
			*m_present_modes = device->getSurfacePresentModesKHR(*surface);
		}

		bool SwapChain::isFormatSupported(vk::SurfaceFormatKHR* format_khr) const {
			
			return std::find(m_formats->begin(), m_formats->end(), *format_khr) != m_formats->end();
		}

		bool SwapChain::isPresentModeSupported(vk::PresentModeKHR* mode) const {
			
			return std::find(m_present_modes->begin(), m_present_modes->end(),*mode) != m_present_modes->end();
		}

		uint32_t SwapChain::findImageCount() const {
   			// determines the amount of images in the swap chain
			
			uint32_t count = m_capabilities->minImageCount + 1;

			// maxImageCount of 0 means there is no limit
			if((m_capabilities->maxImageCount != 0) && (count > m_capabilities->maxImageCount))
				count = m_capabilities->maxImageCount;

			return count;
		}


		void SwapChain::update() {
			// updates the vk::SwapChain to represent the changes made
		
			vk::ImageUsageFlags flags = vk::ImageUsageFlagBits::eColorAttachment;
			vk::SharingMode sharing;

			if(m_queue_ids.size() <= 1)
				sharing = vk::SharingMode::eExclusive;
			else
				sharing = vk::SharingMode::eConcurrent;

			vk::SwapchainCreateInfoKHR info({}, *m_surface_handle, m_image_count, m_format->format, m_format->colorSpace, *m_extent, 1, flags, sharing, m_queue_ids.size(), 
											m_queue_ids.data(), m_capabilities->currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, *m_present_mode, VK_TRUE, {});	

			*m_swap_chain = m_device_handle->createSwapchainKHR(info);

			// retrieving the images
			*m_images = m_device_handle->getSwapchainImagesKHR(*m_swap_chain);
			
			m_image_views->resize(m_images->size());
			vk::ComponentMapping mapping; // defaults to vk::ComponentSwizzle::eIdentity for all components (rgba)
			vk::ImageSubresourceRange sub_resource(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
			
			for(int i = 0; i < m_images->size(); i++) {
				
				vk::ImageViewCreateInfo info({}, m_images->at(i), vk::ImageViewType::e2D, m_format->format, mapping, sub_resource);
				m_image_views->at(i) = m_device_handle->createImageView(info);

			}

		}

		int SwapChain::acquireNextImage() {
						
			return m_current_image;
		}

		void SwapChain::setExtent(uint32_t width, uint32_t height) {
			
			if((width == getWidth()) && (height == getHeight()))
				return;
			
			// it should be checked if the requested extent is supported
			// the surface capabilities specify the min and max extent
			
			if((width < m_capabilities->minImageExtent.width) || (width > m_capabilities->maxImageExtent.width)) {
				UND_ERROR << "Requested SwapChain Width is not supported: " << width << "\n";
				return;
			}
	
			if((height < m_capabilities->minImageExtent.height) || (height > m_capabilities->maxImageExtent.height)) {
				UND_ERROR << "Requested SwapChain Height is not supported: " << height << "\n";
				return;
			}
		
			m_extent->setWidth(width);
			m_extent->setHeight(height);

			update();
		}

		void SwapChain::getExtent(uint32_t& width, uint32_t& height) const {

			width = m_extent->width;
			height = m_extent->height;
		}

		uint32_t SwapChain::getWidth() const {
			
			return m_extent->width;
		}

		uint32_t SwapChain::getHeight() const {

			return m_extent->height;
		}

		void SwapChain::beginFrame() {
			// wait for previous frame to finsh	
			m_device_handle->waitForFences(1, m_frame_in_flight, VK_TRUE, UINT64_MAX);
			m_device_handle->resetFences(1, m_frame_in_flight);
			
			// acquire new swap chain image
			m_current_image = m_device_handle->acquireNextImageKHR(*m_swap_chain, UINT64_MAX, *m_image_available);

		}

		void SwapChain::endFrame() {
		
			// present the image once the processes using it have finished	
			std::vector<vk::Semaphore> wait_semaphores({*m_render_finished});
			std::vector<vk::SwapchainKHR> swap_chains({*m_swap_chain});
			std::vector<uint32_t> image_indices({m_current_image});
			vk::PresentInfoKHR present_info(wait_semaphores, swap_chains, image_indices);

			m_present_queue_handle->presentKHR(present_info);

		}


	} // graphics

} // undicht
