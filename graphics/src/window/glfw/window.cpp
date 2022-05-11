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
            //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // temporary. will need some work later

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

            if((width == m_width) && (m_height == height))
                return;

            m_width = width;
            m_height = height;
            m_has_resized = true;

            glfwSetWindowSize(m_window, width, height);
        }

        void Window::getSize(uint32_t &width, uint32_t &height) const {

            width = m_width;
            height = m_height;
        }

        uint32_t Window::getWidth() const {

            return m_width;
        }

        uint32_t Window::getHeight() const {

            return m_height;
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

            // checking if the window has resized
            int new_width, new_height;
            glfwGetFramebufferSize(m_window, &new_width, &new_height);

            if((m_width != new_width) || (m_height != new_height)) {

                m_width = new_width;
                m_height = new_height;
                m_has_resized = true;
            } else {

                m_has_resized = false;
            }

        }

        void Window::waitForEvent() {

            glfwWaitEvents();
        }

        bool Window::shouldClose() const{

            return glfwWindowShouldClose(m_window);
        }

        bool Window::hasResized() const {

            return m_has_resized;
        }

        bool Window::isMinimized() const {

            return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
        }

    } // namespace graphics

} // namespace undicht