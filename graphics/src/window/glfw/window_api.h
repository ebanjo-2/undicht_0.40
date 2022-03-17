#ifndef WINDOW_API_H
#define WINDOW_API_H

#include "monitor.h"

namespace undicht {

    namespace graphics {

        class WindowAPI {

          public:

            WindowAPI();
            ~WindowAPI();

            uint32_t getMonitorCount() const;

            Monitor getMonitor(uint32_t id = 0);

            std::string info() const;

        };

    } // namespace graphics

} // namespace undicht

#endif // WINDOW_API_H