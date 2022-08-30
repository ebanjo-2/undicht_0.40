//
// Created by eike on 18.06.22.
//

#ifndef UNDICHT_FRAMEBUFFER_H
#define UNDICHT_FRAMEBUFFER_H

#include "vector"

#include "types.h"

#include "core/vulkan/vulkan_declaration.h"
#include "graphics_pipeline/vulkan/texture.h"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;
        class SwapChain;
        class Pipeline;
        class Renderer;

        class Framebuffer {

        protected:

            std::vector<vk::Format>* m_attachment_formats;
            //std::vector<std::vector<vk::ImageView>>* m_attachments; // one vector of attachments for each frame
            std::vector<std::vector<Texture>> m_attachments; // one vector of attachments for each frame
            vk::RenderPass* m_render_pass = 0;
            vk::SubpassDescription* m_sub_pass_description = 0;

            std::vector<vk::Semaphore>* m_render_finished; // one for every frame

            std::vector<vk::Framebuffer>* m_frame_buffers; // one for every frame

            unsigned m_width = 0;
            unsigned m_height = 0;

            const GraphicsDevice* m_device_handle = 0;

            friend SwapChain;
            friend Pipeline;
            friend Renderer;

        public:

            Framebuffer(const GraphicsDevice* device, uint32_t width, uint32_t height);
            virtual ~Framebuffer();

            void cleanUp();

        public:
            // controlling the framebuffer size

            // after the size of the framebuffer was changed,
            // all attachments need to be resized and readded to the framebuffer
            // after all attachments were reattached, call finalizeLayout()
            void changeSize(uint32_t width, uint32_t height);
            uint32_t getWidth() const;
            uint32_t getHeight() const;

        public:
            // adding attachments

            void setAttachment(unsigned id, unsigned frame, const Texture& att);
            bool finalizeLayout(); // to be called when all attachments are set

        protected:

            // make sure that the images size matches the size of the framebuffer
            void setAttachment(unsigned id, unsigned frame, const vk::ImageView& att, const vk::Format& format);

            /* a render pass contains information about the usage of the framebuffer and its attachments
             * when it is used as a render target by a graphics pipeline */
            void createRenderPass();
            // create descriptions of the textures that are going to be drawn to
            std::vector<vk::AttachmentDescription> createAttachmentDescriptions(const std::vector<vk::Format>& att_formats) const;
            // create references for the attachments that describe the attachments layout
            std::vector<vk::AttachmentReference> createAttachmentReferences(const std::vector<vk::AttachmentDescription>& attachments) const;

            std::vector<vk::Semaphore> getImageReadySemaphores(uint32_t frame) const;

        };

    } // graphics

} // undicht


#endif //UNDICHT_FRAMEBUFFER_H
