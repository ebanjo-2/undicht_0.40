#ifndef GRAPHICS_SURFACE_H
#define GRAPHICS_SURFACE_H

#include "vulkan_declaration.h"
#include "GLFW/glfw3.h"

#include "string"

namespace undicht {

	namespace graphics {

		class GraphicsAPI;

		class GraphicsSurface {
			// creates a "drawable" surface on a window
			// can give a framebuffer to the renderer that gets displayed once the window is updated
		private:

			vk::UniqueSurfaceKHR* m_surface = 0;

			uint32_t m_width = 0;
			uint32_t m_height = 0;
	
			friend GraphicsAPI;

			GraphicsSurface(const vk::Instance* instance, GLFWwindow* window);
			
		public:

			~GraphicsSurface();

			std::string info() const;

		};


	} // graphics

} // undicht

#endif // GRAPHICS_SURFACE_H
