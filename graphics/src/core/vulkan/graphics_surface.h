#ifndef GRAPHICS_SURFACE_H
#define GRAPHICS_SURFACE_H

#include "vulkan_declaration.h"
#include "GLFW/glfw3.h"

#include "string"
#include "window/glfw/window.h"

namespace undicht {

	namespace graphics {

		class GraphicsAPI;
        class SwapChain;

		class GraphicsSurface {
			// creates a "drawable" surface on a window
			// can give a framebuffer to the renderer that gets displayed once the window is updated
		private:

			vk::SurfaceKHR* m_surface = 0;
            std::vector<vk::SurfaceKHR>* m_old_surfaces = 0; // old surfaces that can be deleted once the swap-chain using it was destroyed

			uint32_t m_width = 0;
			uint32_t m_height = 0;

            vk::Instance* m_instance_handle = 0;
	
			friend GraphicsAPI;
            friend SwapChain;

			GraphicsSurface(vk::Instance* instance, GLFWwindow* window);

            void createOnWindow(GLFWwindow* window);
            void update(GLFWwindow* window);
			
		public:

			~GraphicsSurface();

        private:

            void cleanUp();

        public:

            void matchWindowExtent(Window& window);

			std::string info() const;

		};


	} // graphics

} // undicht

#endif // GRAPHICS_SURFACE_H
