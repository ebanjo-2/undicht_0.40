#include "monitor.h"

namespace undicht {

    namespace graphics {

        Monitor::Monitor(GLFWmonitor* monitor) : m_monitor(monitor){
            // only the WindowAPI can create a Monitor handle
        }
        
        void Monitor::getSize(uint32_t &width, uint32_t &height) const {
            
            const GLFWvidmode* mode = glfwGetVideoMode(m_monitor);
            width = mode->width;
            height = mode->height;
        }

        uint32_t Monitor::getRefreshRate() const {

            return glfwGetVideoMode(m_monitor)->refreshRate;
        }

        std::string Monitor::info() const {

            return glfwGetMonitorName(m_monitor);
        }

    } // namespace graphics

} // namespace undicht