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

		SwapChain::SwapChain(GraphicsDevice* device, GraphicsSurface* surface)
        : m_framebuffer(device, 0, 0), m_depth_buffer(device) {

			m_device_handle = device;
			m_surface_handle = surface;

            m_extent = new vk::Extent2D;
            m_swap_chain = new vk::SwapchainKHR;

            initSwapChain();
        }

		SwapChain::~SwapChain() {

            cleanUp();

            for(Texture* t : m_images)
                delete t;

            delete m_capabilities;
            delete m_formats;
            delete m_present_modes;
            delete m_format;
            delete m_present_mode;
            delete m_extent;
            delete m_swap_chain;
		
		}


        void SwapChain::cleanUp() {
            // destroys all vulkan objects, but keeps the settings
            // you then can change some of the settings, and update() the swap chain
            // to use it again

            // waiting for the device to finish whatever it was doing
            // to make sure it isn't using any of the objects that are going to be modified
            m_device_handle->m_device->waitIdle();

            m_device_handle->m_device->destroySwapchainKHR(*m_swap_chain);
        }

        //////////////////////////////////// creating the swap chain ///////////////////////////////////////////////////

        void SwapChain::initSwapChain() {

            // checking device capabilities
            if(!checkDeviceCapabilities(m_device_handle->m_physical_device, m_surface_handle->m_surface)) {
                UND_ERROR << "failed to  create swap chain: device does not support any formats or present modes\n";
                return;
            }

            // choosing the swap chain format
            chooseSwapImageFormat();

            // choosing the present mode
            choosePresentMode();

            // determining the image count
            m_image_count = findImageCount();

            // determining the queues that are going to access the swap chain
            m_queue_ids = std::vector<uint32_t>(m_device_handle->m_unique_queue_family_ids.begin(), m_device_handle->m_unique_queue_family_ids.end());

            // this will get the size of the surface and init the swap chain with recreateSwapChain()
            matchSurfaceExtent(*m_surface_handle);
        }

        void SwapChain::recreateSwapChain() {
            // updates the vk::SwapChain to represent the changes made

            // destroying the old swapchain objects
            cleanUp();

            // determining if the swap chain is going to be shared between queue families
            vk::SharingMode sharing;

            if(m_queue_ids.size() <= 1)
                sharing = vk::SharingMode::eExclusive;
            else
                sharing = vk::SharingMode::eConcurrent;

            // creating the swap chain
            vk::SwapchainCreateInfoKHR info({}, *m_surface_handle->m_surface, m_image_count);
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

            *m_swap_chain = m_device_handle->m_device->createSwapchainKHR(info);

            // retrieving the swap images
            retrieveSwapImages();
            initVisibleFramebuffer();
        }

        void SwapChain::retrieveSwapImages() {

            std::vector<vk::Image> images = m_device_handle->m_device->getSwapchainImagesKHR(*m_swap_chain);
            m_images.resize(images.size(), nullptr);

            for(int i = 0; i < images.size(); i++) {

                if(m_images.at(i))
                    delete m_images.at(i);

                m_images.at(i) = new Texture(m_device_handle);

                *m_images.at(i)->m_image = images.at(i);
                m_images.at(i)->m_own_image = false;
                m_images.at(i)->setSize(getWidth(), getHeight());
                m_images.at(i)->setFormat(translateVulkanFormat(m_format->format));
                m_images.at(i)->finalizeLayout();

            }

            // initializing the depth buffer
            m_depth_buffer.setSize(getWidth(), getHeight());
            m_depth_buffer.setFormat(UND_DEPTH32F);
            m_depth_buffer.finalizeLayout();

        }

        void SwapChain::initVisibleFramebuffer() {

            m_framebuffer.changeSize(getWidth(), getHeight());

            for(int i = 0; i < m_images.size(); i++) {

                m_framebuffer.setAttachment(0, i, m_images.at(i));
                m_framebuffer.setAttachment(1, i, &m_depth_buffer); // apparently one depth buffer is enough (according to vulkan-tutorial.com)
            }

            m_framebuffer.finalizeLayout();

        }

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


        ////////////////////////////////////////// swap chain settings /////////////////////////////////////////////////

        void SwapChain::matchSurfaceExtent(const GraphicsSurface& surface) {

            m_surface_handle = &surface;

            // update the surface capabilities
            getSupportDetails(m_device_handle->m_physical_device, m_surface_handle->m_surface);

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

			recreateSwapChain();
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

        uint32_t SwapChain::acquireNextImage(std::vector<Renderer*> wait_for) {

            // the graphics device advances the frame id
            int current_frame = m_device_handle->getCurrentFrameID();

            // wait for previous frame to finish
            for(Renderer* r : wait_for)
                r->beginNewFrame(current_frame);

			// acquire new swap chain image
            m_current_image = m_device_handle->m_device->acquireNextImageKHR(*m_swap_chain, UINT64_MAX, *m_images.at(current_frame)->m_image_ready).value;

            m_framebuffer.setCurrentFrame(m_current_image);

            return m_current_image;
		}

		void SwapChain::presentImage() {

            uint32_t current_frame = m_device_handle->getCurrentFrameID();
		
			// present the image once the processes using it have finished
            std::vector<vk::Semaphore> wait_signals({m_framebuffer.m_render_finished->at(current_frame)});
			std::vector<vk::SwapchainKHR> swap_chains({*m_swap_chain});
			std::vector<uint32_t> image_indices({m_current_image});
			vk::PresentInfoKHR present_info(wait_signals, swap_chains, image_indices);

            try {
                m_device_handle->m_present_queue->presentKHR(present_info);
            } catch(const vk::OutOfDateKHRError& error) {
                // most likely the window was resized
                UND_WARNING << "Swap Chain is out of date (most likely the window was resized)\n    The Swap Chain Extent now needs to be updated as well\n";
            }

		}

        int SwapChain::getCurrentImageID() const{

            return m_current_image;
        }

        Framebuffer& SwapChain::getVisibleFramebuffer() {

            return m_framebuffer;
        }

	} // graphics

} // undicht
