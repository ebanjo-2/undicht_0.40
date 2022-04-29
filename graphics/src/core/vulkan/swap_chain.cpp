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

            m_physical_device_handle = device->m_physical_device;
			m_device_handle = device->m_device;
			m_present_queue_handle = device->m_queues.present_queue;
			m_surface_handle = surface;

            // checking device capabilities
            if(!checkDeviceCapabilities(device->m_physical_device, surface)) {
                UND_ERROR << "failed to  create swap chain: device does not support any formats or present modes\n";
                return;
            }

			// choosing the swap chain format	
            chooseSwapImageFormat();

			// choosing the present mode
            choosePresentMode();

			// determining the image count
			m_image_count = findImageCount();

            // initializing sync objects
            m_image_available = new std::vector<vk::Semaphore>;
            m_render_finished = new std::vector<vk::Semaphore>;
            m_frame_in_flight = new std::vector<vk::Fence>;
			
			// initializing other vulkan members
			m_extent = new vk::Extent2D;
			m_images = new std::vector<vk::Image>;
			m_image_views = new std::vector<vk::ImageView>;
			m_swap_chain = new vk::SwapchainKHR;

		}

		SwapChain::~SwapChain() {

            cleanUp();

            delete m_capabilities;
            delete m_formats;
            delete m_present_modes;
            delete m_format;
            delete m_present_mode;
            delete m_image_available;
            delete m_render_finished;
            delete m_frame_in_flight;
            delete m_extent;
            delete m_images;
            delete m_image_views;
            delete m_swap_chain;
		
		}


        void SwapChain::cleanUp() {
            // destroys all vulkan objects, but keeps the settings
            // you then can change some of the settings, and update() the swap chain
            // to use it again

            // waiting for the device to finish whatever it was doing
            // to make sure it isn't using any of the objects that are going to be modified
            m_device_handle->waitIdle();

            for(vk::Semaphore& s : (*m_image_available))
                m_device_handle->destroySemaphore(s);

            for(vk::Semaphore& s : (*m_render_finished))
                m_device_handle->destroySemaphore(s);

            for(vk::Fence& f : (*m_frame_in_flight))
                m_device_handle->destroyFence(f);

            for(vk::ImageView& image_view : (*m_image_views))
                m_device_handle->destroyImageView(image_view);

            m_device_handle->destroySwapchainKHR(*m_swap_chain);
        }

        //////////////////////////////////// creating the swap chain ///////////////////////////////////////////////////

        bool SwapChain::checkDeviceCapabilities(vk::PhysicalDevice* device, vk::SurfaceKHR* surface) {
            // checking support details
            m_capabilities = new vk::SurfaceCapabilitiesKHR;
            m_formats = new std::vector<vk::SurfaceFormatKHR>;
            m_present_modes = new std::vector<vk::PresentModeKHR>;

            getSupportDetails(device, surface);

            if((!m_formats->size()) || (!m_present_modes->size()))
                return false;

            return true;
        }

        void SwapChain::chooseSwapImageFormat() {

            // preferred format
            vk::SurfaceFormatKHR BGRA_srgb(vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);

            if(isFormatSupported(&BGRA_srgb)) {
                m_format = new vk::SurfaceFormatKHR(BGRA_srgb);
            } else { // fallback: using the first format declared (maybe completely wrong)
                UND_WARNING << "requested surface format is no available\n";
                m_format = new vk::SurfaceFormatKHR(m_formats->at(0));
            }

        }

        void SwapChain::choosePresentMode() {

            // preferred present mode
            vk::PresentModeKHR immediate(vk::PresentModeKHR::eFifoRelaxed);

            if(isPresentModeSupported(&immediate)) {
                m_present_mode = new vk::PresentModeKHR(immediate);
            } else { // fallback: vsync is the only present mode guaranteed to exist
                UND_WARNING << "requested present mode is not available\n";
                m_present_mode = new vk::PresentModeKHR(vk::PresentModeKHR::eFifo);
            }

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

        void SwapChain::initSyncObjects() {

            // creating new sync objects
            vk::SemaphoreCreateInfo semaphore_info;
            vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled); // set initial state as set

            m_image_available->resize(m_max_frames_in_flight);
            m_render_finished->resize(m_max_frames_in_flight);
            m_frame_in_flight->resize(m_max_frames_in_flight);

            for(int i = 0; i < m_max_frames_in_flight; i++) {

                m_image_available->at(i) = m_device_handle->createSemaphore(semaphore_info);
                m_render_finished->at(i) = m_device_handle->createSemaphore(semaphore_info);
                m_frame_in_flight->at(i) = m_device_handle->createFence(fence_info);

            }
        }

        /////////////////////////////////////////// recreating the swap chain //////////////////////////////////////////

		void SwapChain::update() {
			// updates the vk::SwapChain to represent the changes made

            // destroying the old swapchain objects
            cleanUp();

            // creating sync objects
            initSyncObjects();

            // determining if the swap chain is going to be shared between queue families
			vk::SharingMode sharing;

			if(m_queue_ids.size() <= 1)
				sharing = vk::SharingMode::eExclusive;
			else
				sharing = vk::SharingMode::eConcurrent;

            // creating the swap chain
			vk::SwapchainCreateInfoKHR info({}, *m_surface_handle, m_image_count);
            info.setImageFormat(m_format->format);
            info.setImageColorSpace(m_format->colorSpace);
            info.setImageExtent(*m_extent);
            info.setImageArrayLayers(1);
            info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
            info.setImageSharingMode(sharing);
            info.setQueueFamilyIndices(m_queue_ids);
            info.setPreTransform(m_capabilities->currentTransform);
            info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
            info.setPresentMode(*m_present_mode);
            info.setClipped(VK_TRUE);

			*m_swap_chain = m_device_handle->createSwapchainKHR(info);

			// retrieving the swap images
            retrieveSwapImages();

		}

        void SwapChain::retrieveSwapImages() {

            *m_images = m_device_handle->getSwapchainImagesKHR(*m_swap_chain);

            m_image_views->resize(m_images->size());
            vk::ComponentMapping mapping; // defaults to vk::ComponentSwizzle::eIdentity for all components (rgba)
            vk::ImageSubresourceRange sub_resource(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

            for(int i = 0; i < m_images->size(); i++) {

                vk::ImageViewCreateInfo info({}, m_images->at(i), vk::ImageViewType::e2D, m_format->format, mapping, sub_resource);
                m_image_views->at(i) = m_device_handle->createImageView(info);

            }

        }

        ////////////////////////////////////////// swap chain settings /////////////////////////////////////////////////

        void SwapChain::matchSurfaceExtent(const GraphicsSurface& surface) {

            m_surface_handle = surface.m_surface;

            // update the surface capabilities
            getSupportDetails(m_physical_device_handle, m_surface_handle);

            // set the extent
            setExtent(surface.m_width, surface.m_height);

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

		void SwapChain::setMaxFramesInFlight(uint32_t count) {
			// once a frame is finished it is presented
			// instead of waiting for the frame to be done with being presented, 
			// the gpu may also start working on the next frame
			// even though this has the potential to increase performance, 
			// it also needs more resources, since objects like command buffers can only be used by one frame at a time
			// so they need to be copied for each frame in flight

			m_max_frames_in_flight = count;

            update();
            //initSyncObjects();
		}

		uint32_t SwapChain::getMaxFramesInFlight() const {
		
			return m_max_frames_in_flight;
		}

		uint32_t SwapChain::getCurrentFrameID() const {

			return m_current_frame;
		}

        int SwapChain::getCurrentImageID() const{

            return m_current_image;
        }

		void SwapChain::beginFrame() {

            int current_frame = getCurrentFrameID();

            // wait for previous frame to finish
			m_device_handle->waitForFences(1, &m_frame_in_flight->at(current_frame), VK_TRUE, UINT64_MAX);
			m_device_handle->resetFences(1, &m_frame_in_flight->at(current_frame));
			
			// acquire new swap chain image
            m_current_image = m_device_handle->acquireNextImageKHR(*m_swap_chain, UINT64_MAX,m_image_available->at(current_frame));


		}

		void SwapChain::endFrame() {
		
			// present the image once the processes using it have finished
            std::vector<vk::Semaphore> render_finished({m_render_finished->at(m_current_frame)});
			std::vector<vk::SwapchainKHR> swap_chains({*m_swap_chain});
			std::vector<uint32_t> image_indices({m_current_image});
			vk::PresentInfoKHR present_info(render_finished, swap_chains, image_indices);

            try {
                m_present_queue_handle->presentKHR(present_info);
            } catch(const vk::OutOfDateKHRError& error) {
                // most likely the window was resized
                UND_WARNING << "Swap Chain is out of date (most likely the window was resized)\n    The Swap Chain Extent now needs to be updated as well\n";
            }

			// increasing the frame counter
			m_current_frame = (m_current_frame + 1)  % m_max_frames_in_flight;
		}


	} // graphics

} // undicht
