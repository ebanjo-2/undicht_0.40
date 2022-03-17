#include "window.h"

#include "debug.h"

namespace undicht {

    namespace graphics {

        Window::Window(const std::string &title, uint32_t width, uint32_t height) {

            open(title, width, height);
        }

        void Window::open(const std::string &title, uint32_t width, uint32_t height) {

            // only for opengl 3.3
            // glfwWindowHint(GLFW_VERSION_MAJOR, 3);
            // glfwWindowHint(GLFW_VERSION_MINOR, 3);
            // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            // only for vulkan
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // temporary. will need some work later

            // creating the window
            m_window = glfwCreateWindow(width, height, title.data(), 0, 0);

            if (m_window == NULL) {
                UND_ERROR << "failed to open window\n";
                return;
            }
        }

        void Window::close() {

            glfwDestroyWindow(m_window);
        }

        void Window::setTitle(const std::string &title) {

            glfwSetWindowTitle(m_window, title.data());
        }

        std::string Window::getTitle() const {

            return "";
        }

        void Window::setSize(uint32_t width, uint32_t height) {

            glfwSetWindowSize(m_window, width, height);
        }

        void Window::getSize(uint32_t &width, uint32_t &height) const {

            glfwGetWindowSize(m_window, (int *)&width, (int *)&height);
        }

        void Window::setFullscreen(Monitor* monitor) {

            m_fullscreen = true;

            uint32_t width, height, refresh_rate;
            monitor->getSize(width, height);
            refresh_rate = monitor->getRefreshRate();

            glfwSetWindowMonitor(m_window, monitor->m_monitor, 0, 0, width, height, refresh_rate);
        }

        void Window::setWindowed(uint32_t &width, uint32_t &height) {
            // undo fullscreen

            m_fullscreen = false;

            glfwSetWindowMonitor(m_window, 0, 0, 0, width, height, 0);
        } 

        bool Window::isFullscreen() const {

            return m_fullscreen;
        }

        void Window::update() {

            glfwPollEvents();
        }

        bool Window::shouldClose() {

            return glfwWindowShouldClose(m_window);
        }

    } // namespace graphics

} // namespace undicht