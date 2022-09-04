//
// Created by eike on 18.06.22.
//

#include "framebuffer.h"

namespace undicht {

    namespace graphics {


        Framebuffer::Framebuffer(const GraphicsDevice *device, uint32_t width, uint32_t height) {

            m_width = width;
            m_height = height;

            m_device_handle = device;

            m_attachment_formats = new std::vector<vk::Format>;
            m_sub_pass_description = new vk::SubpassDescription;
            m_render_pass = new vk::RenderPass;
            m_render_finished = new std::vector<vk::Semaphore>;
            m_frame_buffers = new std::vector<vk::Framebuffer>;

        }

        Framebuffer::~Framebuffer() {

            cleanUp();

            delete m_attachment_formats;
            delete m_sub_pass_description;
            delete m_render_pass;
            delete m_render_finished;
            delete m_frame_buffers;

        }

        void Framebuffer::cleanUp() {

            for(vk::Framebuffer& fbo : *m_frame_buffers)
                m_device_handle->m_device->destroyFramebuffer(fbo);

            m_frame_buffers->clear();

            for(vk::Semaphore& sem : *m_render_finished)
                m_device_handle->m_device->destroySemaphore(sem);

            m_render_finished->clear();

            m_attachments.clear();

            m_device_handle->m_device->destroyRenderPass(*m_render_pass);

        }

        void Framebuffer::setCurrentFrame(uint32_t frame_id) {
            // vulkan determines which framebuffer to render to next (use SwapChain.acquireNextImage())
            // this image might be a different number than the frame id

            m_current_frame = frame_id;
        }

        /////////////////////////////// controlling the framebuffer size ///////////////////////////////

        void Framebuffer::changeSize(uint32_t width, uint32_t height) {

            m_width = width;
            m_height = height;
            cleanUp();
        }

        uint32_t Framebuffer::getWidth() const {

            return m_width;
        }

        uint32_t Framebuffer::getHeight() const {

            return m_height;
        }

        ///////////////////////////////////// adding attachments //////////////////////////////////

        void Framebuffer::setAttachment(unsigned int id, unsigned frame, Texture *att) {

            if(att->m_width != m_width || att->m_height != m_height) {
                UND_ERROR << "failed to attach texture to framebuffer: texture has wrong size (id = " << id << ") \n";
                return;
            }

            if(m_attachments.size() <= frame)
                m_attachments.resize(frame + 1);

            if(m_attachments.at(frame).size() <= id)
                m_attachments.at(frame).resize(id + 1, nullptr);

            if(m_attachment_formats->size() <= id)
                m_attachment_formats->resize(id + 1);

            m_attachments.at(frame).at(id) = att;
            m_attachment_formats->at(id) = *att->m_format;

        }

        bool Framebuffer::finalizeLayout() {

            createRenderPass();

            int frame_count = m_attachments.size();
            int att_count = m_attachment_formats->size();

            m_frame_buffers->resize(frame_count);

            // attaching the textures to the framebuffer for each frame
            for(int frame = 0; frame < frame_count; frame++) {

                std::vector<vk::ImageView> attachments;

                for(int att = 0; att < att_count; att++) {

                    attachments.push_back(*m_attachments.at(frame).at(att)->m_image_view);
                }

                vk::FramebufferCreateInfo fbo_info({}, *m_render_pass, attachments);
                fbo_info.setWidth(getWidth());
                fbo_info.setHeight(getHeight());
                fbo_info.setLayers(1);

                m_frame_buffers->at(frame) = m_device_handle->m_device->createFramebuffer(fbo_info);

            }

            // init semaphores
            for(int i = m_render_finished->size(); i < frame_count; i++) {
                vk::SemaphoreCreateInfo semaphore_info;
                m_render_finished->push_back(m_device_handle->m_device->createSemaphore(semaphore_info));
            }

            return true;
        }


        void Framebuffer::createRenderPass() {

            std::vector<vk::AttachmentDescription> attachments = createAttachmentDescriptions(*m_attachment_formats);
            std::vector<vk::AttachmentReference> color_attachment_refs = createColorAttachmentReferences(attachments);
            vk::AttachmentReference depth_attachment_ref = createDepthAttachmentReference(attachments);


            // creating the subpass description
            m_sub_pass_description->setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
            m_sub_pass_description->setColorAttachments(color_attachment_refs);
            m_sub_pass_description->setPDepthStencilAttachment(&depth_attachment_ref);
            std::vector<vk::SubpassDescription> subpasses({*m_sub_pass_description});

            // declaring the stages the subpass depends on
            vk::SubpassDependency subpass_dependency(VK_SUBPASS_EXTERNAL, 0);
            subpass_dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
            subpass_dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
            subpass_dependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
            subpass_dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            std::vector<vk::SubpassDependency> subpass_dependencies({subpass_dependency});

            // creating the render pass
            vk::RenderPassCreateInfo render_pass_info({}, attachments, subpasses, subpass_dependencies);
            *m_render_pass = m_device_handle->m_device->createRenderPass(render_pass_info);

        }

        std::vector<vk::AttachmentDescription> Framebuffer::createAttachmentDescriptions(const std::vector<vk::Format>& att_formats) const{
            // create descriptions of the textures that are going to be drawn to

            std::vector<vk::AttachmentDescription> attachments;

            for(int i = 0; i < att_formats.size(); i++) {

                FixedType att_format = translateVulkanFormat(att_formats.at(i));

                vk::AttachmentDescription attachment({}, att_formats[i], vk::SampleCountFlagBits::e1);

                if(att_format.m_type == Type::COLOR_RGBA || att_format.m_type == Type::COLOR_BGRA) {
                    attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
                    attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
                    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
                    attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
                    attachment.setInitialLayout(vk::ImageLayout::eUndefined);
                    attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
                }

                if(att_format.m_type == Type::DEPTH_BUFFER || att_format.m_type == Type::DEPTH_STENCIL_BUFFER) {
                    attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
                    attachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
                    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
                    attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
                    attachment.setInitialLayout(vk::ImageLayout::eUndefined);
                    attachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
                }

                attachments.push_back(attachment);
            }

            return attachments;
        }

        std::vector<vk::AttachmentReference> Framebuffer::createColorAttachmentReferences(const std::vector<vk::AttachmentDescription>& attachments) const{
            // create references for the attachments that describe the attachments layout

            std::vector<vk::AttachmentReference> refs;

            for(int i = 0; i < attachments.size(); i++) {
                // assuming that all attachments are color attachments for now

                FixedType und_format = translateVulkanFormat(attachments.at(i).format);

                if(und_format.m_type == Type::COLOR_BGRA || und_format.m_type == Type::COLOR_RGBA) {
                    vk::AttachmentReference color_ref(i, vk::ImageLayout::eColorAttachmentOptimal);
                    refs.push_back(color_ref);
                }

            }

            return refs;
        }

        vk::AttachmentReference Framebuffer::createDepthAttachmentReference(const std::vector<vk::AttachmentDescription>& attachments) const{
            // create references for the attachments that describe the attachments layout

            for(int i = 0; i < attachments.size(); i++) {
                // assuming that all attachments are color attachments for now

                FixedType und_format = translateVulkanFormat(attachments.at(i).format);

                if(und_format.m_type == Type::DEPTH_BUFFER || und_format.m_type == Type::DEPTH_STENCIL_BUFFER) {
                    vk::AttachmentReference depth_ref(i, vk::ImageLayout::eDepthStencilAttachmentOptimal);
                    return depth_ref;
                }

            }

            return {};
        }

        const vk::Framebuffer* Framebuffer::getCurrentFramebuffer() const{

            return &m_frame_buffers->at(m_current_frame);
        }

        std::vector<vk::Semaphore> Framebuffer::getImageReadySemaphores(unsigned frame) const {
            // image ready signals for the current framebuffer

            std::vector<vk::Semaphore> semaphores;
            for(int i = 0; i < m_attachment_formats->size(); i++) {

                if(!m_attachments.at(frame).at(i)->m_own_image)
                    semaphores.push_back(*m_attachments.at(frame).at(i)->m_image_ready);

            }

            return semaphores;
        }

    } // graphics

} // undicht
