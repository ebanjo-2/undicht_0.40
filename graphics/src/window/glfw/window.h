#ifndef WINDOW_H
#define WINDOW_H

#include "GLFW/glfw3.h"
#include "string"

#include "monitor.h"

namespace undicht {

    namespace graphics {

        class Window {
            
            private:

                GLFWwindow* m_window = 0;
                bool m_fullscreen = false;

            public:

                Window( const std::string& title = "", uint32_t width = 800, uint32_t height = 600);

                void open(const std::string& title = "", uint32_t width = 800, uint32_t height = 600);
                void close();

                void setTitle(const std::string& title);
                std::string getTitle() const;

                void setSize(uint32_t width, uint32_t height);
                void getSize(uint32_t &width, uint32_t &height) const;

                void setFullscreen(Monitor* monitor);
                void setWindowed(uint32_t &width, uint32_t &height); // undo fullscreen
                bool isFullscreen() const;

                void update();

                bool shouldClose();

        };

    } // namespace graphics

} // namespace undicht

#endif // WINDOW_H