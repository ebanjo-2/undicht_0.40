#ifndef WINDOW_H
#define WINDOW_H

#include "GLFW/glfw3.h"
#include "string"

#include "monitor.h"

namespace undicht {

    namespace graphics {

		class GraphicsAPI;
        class SwapChain;
        class GraphicsSurface;

        class Window {
            
            private:

				friend GraphicsAPI;
                friend SwapChain;
                friend GraphicsSurface;

                GLFWwindow* m_window = 0;
                bool m_fullscreen = false;

                uint32_t m_width = 0;
                uint32_t m_height = 0;
                bool m_has_resized = 0; // since the last frame

            public:

                Window( const std::string& title = "", uint32_t width = 800, uint32_t height = 600);

                void open(const std::string& title = "", uint32_t width = 800, uint32_t height = 600);
                void close();

                void setTitle(const std::string& title);
                std::string getTitle() const;

                void setSize(uint32_t width, uint32_t height);
                void getSize(uint32_t &width, uint32_t &height) const;
                uint32_t getWidth() const;
                uint32_t getHeight() const;

                void setFullscreen(Monitor* monitor);
                void setWindowed(uint32_t &width, uint32_t &height); // undo fullscreen
                bool isFullscreen() const;

                void update();
                void waitForEvent();

                // events
                bool shouldClose() const;
                bool hasResized() const;
                bool isMinimized() const;

        };

    } // namespace graphics

} // namespace undicht

#endif // WINDOW_H
