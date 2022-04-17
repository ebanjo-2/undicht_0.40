#include "graphics_surface.h"
#include "core/vulkan/vulkan_declaration.h"
#include "debug.h"

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

namespace undicht {

	namespace graphics {

		GraphicsSurface::GraphicsSurface(const vk::Instance* instance, GLFWwindow* window) {

			VkSurfaceKHR* tmp = new VkSurfaceKHR;

			if (glfwCreateWindowSurface(*instance, window, nullptr, tmp) != VK_SUCCESS) {
				
				UND_ERROR << "failed to create graphics surface\n";
			}

			m_surface = new vk::UniqueSurfaceKHR(*tmp, *instance);
			
			delete tmp;
		}

		GraphicsSurface::~GraphicsSurface() {
			
			if(m_surface) 
				delete m_surface;
			
		}


		std::string GraphicsSurface::info() const {

			return "";
		}

	} // namespace graphics

} // namespace undicht
