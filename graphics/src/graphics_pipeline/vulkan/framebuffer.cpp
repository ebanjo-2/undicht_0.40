//
// Created by eike on 18.06.22.
//

#include "framebuffer.h"

namespace undicht {

    namespace graphics {


        Framebuffer::Framebuffer(const GraphicsDevice *device) {

            m_device_handle = device;

            m_attachments = new std::vector<std::vector<vk::ImageView>>;
            m_attachment_formats = new std::vector<vk::Format>;
            m_render_finished = new std::vector<vk::Semaphore>;
            m_frame_buffers = new std::vector<vk::Framebuffer>;

        }

        Framebuffer::~Framebuffer() {

            delete m_attachments;
            delete m_attachment_formats;
            delete m_render_finished;
            delete m_frame_buffers;

        }

        void Framebuffer::cleanUp() {

        }

        ///////////////////////////////////// adding attachments

        void Framebuffer::setAttachment(unsigned int id, unsigned frame, const Texture &att) {

            if(m_width || m_height) {

                if(att.m_width != m_width || att.m_height != m_height) {
                    UND_ERROR << "failed to attach texture to framebuffer: texture has wrong size (id = " << id << ") \n";
                    return;
                }
            }

            setAttachment(id, frame, *att.m_image_view, *att.m_format);
        }

        bool Framebuffer::finalizeLayout() {


        }

        void Framebuffer::setAttachment(unsigned int id, unsigned frame, const vk::ImageView &att, const vk::Format& format) {

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

        }

    } // graphics

} // undicht
