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

            //m_attachments = new std::vector<std::vector<vk::ImageView>>;
            m_attachment_formats = new std::vector<vk::Format>;
            m_sub_pass_description = new vk::SubpassDescription;
            m_render_pass = new vk::RenderPass;
            m_render_finished = new std::vector<vk::Semaphore>;
            m_frame_buffers = new std::vector<vk::Framebuffer>;

        }

        Framebuffer::~Framebuffer() {

            cleanUp();

            //delete m_attachments;
            delete m_attachment_formats;
            delete m_sub_pass_description;
            delete m_render_pass;
            delete m_render_finished;
            delete m_frame_buffers;

            for(std::vector<Texture*> v : m_attachments)
                for(Texture* t : v)
                    delete t;

        }

        void Framebuffer::cleanUp() {

            for(vk::Framebuffer& fbo : *m_frame_buffers)
                m_device_handle->m_device->destroyFramebuffer(fbo);

            for(vk::Semaphore& sem : *m_render_finished)
                m_device_handle->m_device->destroySemaphore(sem);

            m_device_handle->m_device->destroyRenderPass(*m_render_pass);

        }

        /////////////////////////////// controlling the framebuffer size ///////////////////////////////

        void Framebuffer::changeSize(uint32_t width, uint32_t height) {

            m_width = width;
            m_height = height;

        }

        uint32_t Framebuffer::getWidth() const {

            return m_width;
        }

        uint32_t Framebuffer::getHeight() const {

            return m_height;
        }

        ///////////////////////////////////// adding attachments //////////////////////////////////

        void Framebuffer::setAttachment(unsigned int id, unsigned frame, const Texture &att) {

            if(att.m_width != m_width || att.m_height != m_height) {
                UND_ERROR << "failed to attach texture to framebuffer: texture has wrong size (id = " << id << ") \n";
                return;
            }


            // making sure that the Framebuffer has enough capacity for the amount of frames
            if(frame >= m_frame_buffers->size()) {
                m_attachments.resize(frame + 1);
                //m_frame_buffers->resize(frame + 1);
            }

            // making sure the attachment vector for the requested frame is big enough
            if(m_attachments.at(frame).size() <= id)
                m_attachments.at(frame).resize(id + 1, nullptr);

            // storing the attachment
            if(!m_attachments.at(frame).at(id))
                m_attachments.at(frame).at(id) = new Texture(m_device_handle);
            *m_attachments.at(frame).at(id) = att;
            m_attachments.at(frame).at(id)->m_own_image = false;

            // storing the attachment format
            if(m_attachment_formats->size() <= id)
                m_attachment_formats->resize(id + 1);

            m_attachment_formats->at(id) = *att.m_format;

            //setAttachment(id, frame, *att.m_image_view, *att.m_format);
        }

        bool Framebuffer::finalizeLayout() {

            createRenderPass();

            int frame_count = m_attachments.size();
            int att_count = m_attachment_formats->size();

            m_frame_buffers->resize(frame_count);

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
            m_render_finished->resize(frame_count);
            vk::SemaphoreCreateInfo semaphore_info;

            for(int i = 0; i < frame_count; i++)
                m_render_finished->at(i) = m_device_handle->m_device->createSemaphore(semaphore_info);

            return true;
        }

        /*void Framebuffer::setAttachment(unsigned int id, unsigned frame, const vk::ImageView &att, const vk::Format& format) {

            if(frame >= m_frame_buffers->size()) {
                m_attachments->resize(frame + 1);
                m_render_finished->resize(frame + 1);
                m_frame_buffers->resize(frame + 1);
            }

            // storing the image view
            if(m_attachments->at(frame).size() <= id)
                m_attachments->at(frame).resize(id + 1);

            m_attachments->at(frame).at(id) = att;

            // storing the attachment format
            if(m_attachment_formats->size() <= id)
                m_attachment_formats->resize(id + 1);

            m_attachment_formats->at(id) = format;

        }*/


        void Framebuffer::createRenderPass() {

            std::vector<vk::AttachmentDescription> attachments = createAttachmentDescriptions(*m_attachment_formats);
            std::vector<vk::AttachmentReference> attachment_refs = createAttachmentReferences(attachments);

            // creating the subpass description
            m_sub_pass_description->setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
            m_sub_pass_description->setColorAttachments(attachment_refs);
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

                vk::AttachmentDescription attachment({}, att_formats[i], vk::SampleCountFlagBits::e1);
                attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
                attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
                attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
                attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
                attachment.setInitialLayout(vk::ImageLayout::eUndefined);
                attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

                attachments.push_back(attachment);
            }

            return attachments;
        }

        std::vector<vk::AttachmentReference> Framebuffer::createAttachmentReferences(const std::vector<vk::AttachmentDescription>& attachments) const{
            // create references for the attachments that describe the attachments layout

            std::vector<vk::AttachmentReference> refs;

            for(const vk::AttachmentDescription& description : attachments) {
                // assuming that all attachments are color attachments for now

                vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
                refs.push_back(color_ref);
            }

            return refs;
        }

        std::vector<vk::Semaphore> Framebuffer::getImageReadySemaphores(uint32_t frame) const {

            std::vector<vk::Semaphore> semaphores;
            for(int i = 0; i < m_attachment_formats->size(); i++) {
                semaphores.push_back(*m_attachments.at(frame).at(i)->m_image_ready);
            }

            return semaphores;
        }

    } // graphics

} // undicht
