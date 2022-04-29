#include "graphics_surface.h"
#include "core/vulkan/vulkan_declaration.h"
#include "debug.h"

#include "sstream"

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

namespace undicht {

	namespace graphics {

		GraphicsSurface::GraphicsSurface(vk::Instance* instance, GLFWwindow* window) {

            m_instance_handle = instance;
            m_surface = new vk::SurfaceKHR;
            m_old_surfaces = new std::vector<vk::SurfaceKHR>;

            createOnWindow(window);
		}

        GraphicsSurface::~GraphicsSurface() {

            m_old_surfaces->push_back(*m_surface);

            cleanUp();
            delete m_surface;
            delete m_old_surfaces;
        }

        void GraphicsSurface::createOnWindow(GLFWwindow* window) {

            // destroy the old surface
            cleanUp();

            // creating the graphics surface
            VkSurfaceKHR surf;
            if (glfwCreateWindowSurface(*m_instance_handle, window, nullptr, &surf) != VK_SUCCESS) {
                UND_ERROR << "failed to create graphics surface\n";
            }

            // saving the vulkan.h surface in the vulkan.hpp
            m_old_surfaces->push_back(*m_surface);
            *m_surface = vk::SurfaceKHR(surf);

            update(window);
        }

        void GraphicsSurface::update(GLFWwindow* window) {

            // getting the size of the window
            glfwGetFramebufferSize(window, (int*)&m_width, (int*)&m_height);
        }


        void GraphicsSurface::cleanUp() {

            for(vk::SurfaceKHR& old_surface : (*m_old_surfaces))
                m_instance_handle->destroySurfaceKHR(old_surface);

        }

        ////////////////////////////////////////////// api functions ///////////////////////////////////////////////////

        void GraphicsSurface::matchWindowExtent(Window &window) {

            update(window.m_window);
        }

		std::string GraphicsSurface::info() const {

            std::stringstream s;
            s << "Surface Size: " << m_width << " : " << m_height;

			return s.str();
		}

	} // namespace graphics

} // namespace undicht
