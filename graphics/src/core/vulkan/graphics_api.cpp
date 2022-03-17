#include "graphics_api.h"

// only here to convince vscode everything is fine
#include "vulkan/vulkan.hpp"

namespace undicht {

    namespace graphics {

        GraphicsAPI::GraphicsAPI() {

            vk::ApplicationInfo app_info("undicht");
            vk::InstanceCreateInfo create_info({}, &app_info, 0, 0, 0, 0);
            
            m_instance = new vk::Instance;
            *m_instance = vk::createInstance(create_info);

        }

        GraphicsAPI::~GraphicsAPI() {

            if(m_instance) {
                m_instance->destroy();
                delete m_instance;
            }

        }

        std::string GraphicsAPI::info() const {

            return "vulkan";
        }

    } // namespace  graphics

} // namespace undicht