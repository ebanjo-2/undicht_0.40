#include "swap_chain.h"

#include "core/vulkan/vulkan_declaration.h"
#include "graphics_api.h"
#include "graphics_surface.h"
#include "debug.h"

#include "vulkan/vulkan.hpp"

namespace undicht {

	namespace graphics {

		SwapChain::SwapChain(vk::PhysicalDevice* device, vk::SurfaceKHR* surface) {
			
			// checking support details
			m_capabilities = new vk::SurfaceCapabilitiesKHR;
			m_formats = new std::vector<vk::SurfaceFormatKHR>;
			m_present_modes = new std::vector<vk::PresentModeKHR>;

			getSupportDetails(device, surface);
			
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


		}

		void SwapChain::getSupportDetails(vk::PhysicalDevice* device, vk::SurfaceKHR* surface) {
		
			// device capabilities
			*m_capabilities = device->getSurfaceCapabilitiesKHR(*surface);
			
			// supported formats
			*m_formats = device->getSurfaceFormatsKHR(*surface);
		
			// supported present modes
			*m_present_modes = device->getSurfacePresentModesKHR(*surface);
		}

		bool SwapChain::isFormatSupported(vk::SurfaceFormatKHR* format_khr) {
			
			return std::find(m_formats->begin(), m_formats->end(), *format_khr) != m_formats->end();
		}

		bool SwapChain::isPresentModeSupported(vk::PresentModeKHR* mode) {
			
			return std::find(m_present_modes->begin(), m_present_modes->end(),*mode) != m_present_modes->end();
		}

		SwapChain::~SwapChain() {
		
			if(m_capabilities);
				delete m_capabilities;
				
			if(m_formats)
				delete m_formats;

			if(m_present_modes)
				delete m_present_modes;

			if(m_format)
				delete m_format;

			if(m_present_mode)
				delete m_present_mode;
		
		}

	} // graphics

} // undicht
