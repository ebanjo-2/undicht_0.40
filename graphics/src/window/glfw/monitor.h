#ifndef MONITOR_H
#define MONITOR_H

#include "string"
#include "GLFW/glfw3.h"

namespace undicht {

    namespace graphics {

        class WindowAPI;
        class Window;

        class Monitor {

          private:

            GLFWmonitor* m_monitor = 0;

            friend WindowAPI;
            friend Window;

            Monitor(GLFWmonitor* monitor); // only the WindowAPI can create a Monitor handle

          public:

            void getSize(uint32_t& width, uint32_t& height) const;
            
            uint32_t getRefreshRate() const;

            std::string info() const;

        };

    } // namespace graphics

} // namespace undicht

#endif // MONITOR_H