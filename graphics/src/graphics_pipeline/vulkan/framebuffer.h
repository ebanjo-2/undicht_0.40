//
// Created by eike on 18.06.22.
//

#ifndef UNDICHT_FRAMEBUFFER_H
#define UNDICHT_FRAMEBUFFER_H

#include "vector"

#include "types.h"

#include "core/vulkan/vulkan_declaration.h"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;

        class Framebuffer {

        protected:

            std::vector<vk::Format>* m_attachment_formats;
            std::vector<std::vector<vk::ImageView>>* m_attachments; // one vector of attachments for each frame

            std::vector<vk::Semaphore>* m_render_finished; // one for every frame

            std::vector<vk::Framebuffer>* m_frame_buffers;

            unsigned m_width = 0;
            unsigned m_height = 0;

            const GraphicsDevice* m_device_handle = 0;

        public:

            Framebuffer(const GraphicsDevice* device);
            virtual ~Framebuffer();
            void cleanUp();

        public:
            // adding attachments

            void setAttachment(unsigned id, unsigned frame, const Texture& att);
            bool finalizeLayout(); // to be called when all attachments are set

        protected:

            void setAttachment(unsigned id, unsigned frame, const vk::ImageView& att, const vk::Format& format);

        };

    } // graphics

} // undicht


#endif //UNDICHT_FRAMEBUFFER_H
