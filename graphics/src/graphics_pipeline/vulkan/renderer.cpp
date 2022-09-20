#include "renderer.h"
#include "debug.h"


#include "vulkan/vulkan.hpp"

#include "vector"
#include "tuple"

#include "core/vulkan/graphics_device.h"

namespace undicht {

	namespace graphics {
		
		Renderer::Renderer(const GraphicsDevice* device) : m_pipeline(device), m_render_pass(device){

            m_device_handle = device;

			// creating the command buffers
            m_render_finished = new std::vector<vk::Fence>;

            // creating other member objects
			//m_swap_frame_buffers = new std::vector<vk::Framebuffer>;
            m_render_started.resize(device->getMaxFramesInFlight());
            m_render_finished->resize(device->getMaxFramesInFlight());
            for(vk::Fence& fence : *m_render_finished)
                fence = m_device_handle->m_device->createFence(vk::FenceCreateInfo());

		}

		Renderer::~Renderer() {

            cleanUp();

            // actual pipeline objects
            delete m_render_finished;

		}

        void Renderer::cleanUp() {

            m_device_handle->m_device->waitIdle();

            for(vk::Fence& fence : *m_render_finished)
                m_device_handle->m_device->destroyFence(fence);

        }

        ////////////////////////////////////////////// creating the renderer ///////////////////////////////////////////

        void Renderer::setFramebufferLayout(const Framebuffer& fbo) {
            m_pipeline.setFramebufferLayout(fbo);
            m_pipeline.setViewport(fbo.getWidth(), fbo.getHeight());
        }

        void Renderer::setVertexBufferLayout(const VertexBuffer& vbo_prototype) {

            m_pipeline.setVertexBufferLayout(vbo_prototype);
        }

        void Renderer::setShader(Shader* shader) {

            m_pipeline.setShader(shader);
        }


        void Renderer::setShaderInput(uint32_t ubo_count, uint32_t tex_count) {

            m_ubos.resize(ubo_count);
            m_textures.resize(tex_count);

            m_ubos_updated_for_frame.resize(ubo_count, std::vector<bool>(m_device_handle->getMaxFramesInFlight()));
            m_text_updated_for_frame.resize(tex_count, std::vector<bool>(m_device_handle->getMaxFramesInFlight()));

            m_pipeline.setShaderInput(ubo_count, tex_count);

        }

        void Renderer::setViewport(unsigned width, unsigned height) {

            m_pipeline.setViewport(width, height);
        }

        void Renderer::setDepthTest(bool test, bool write) {

            m_pipeline.setDepthTest(test, write);
        }

        void Renderer::linkPipeline() {

            m_pipeline.linkPipeline();

        }


        /////////////////////////  managed by the swap chain /////////////////////////////////

        void Renderer::beginNewFrame(uint32_t frame_id) {

            if(!renderStarted(frame_id))
                return;

            m_device_handle->m_device->waitForFences(1, &m_render_finished->at(frame_id), VK_TRUE,UINT64_MAX);
            m_device_handle->m_device->resetFences(1, &m_render_finished->at(frame_id));
            m_render_started.at(frame_id) = false;

        }

        bool Renderer::renderStarted(uint32_t frame_id) {

            return m_render_started.at(frame_id);
        }

		/////////////////////////////////////// drawing /////////////////////////////////////


        void Renderer::submit(UniformBuffer *ubo, uint32_t index) {

            if(m_ubos.size() <= index) {
                UND_ERROR << "failed to submit ubo: the index was is to big for this renderer\n";
                return;
            }

            if(m_ubos.at(index) != ubo) {

                std::fill(m_ubos_updated_for_frame.at(index).begin(), m_ubos_updated_for_frame.at(index).end(), false);
                m_ubos.at(index) = ubo;
            }

            uint32_t current_frame = m_device_handle->getCurrentFrameID();
            //if(!m_ubos_updated_for_frame.at(index).at(current_frame)) {

                ubo->writeDescriptorSet(m_pipeline.getShaderInputDescriptor(current_frame, m_current_draw_call), index, current_frame);
               // m_ubos_updated_for_frame.at(index).at(current_frame) = true;
            //}

            ubo->updateBuffer(current_frame);

        }

        void Renderer::submit(const Texture* tex, uint32_t index) {

            // in the shader the texture is accessed by an index
            // that comes after the uniform buffers
            // calculating the actual index of the texture
            index -= m_ubos.size();

            if(m_textures.size() <= index) {
                UND_ERROR << "failed to submit texture: the index is to big for this renderer\n";
                return;
            }

            if(m_textures.at(index) != tex) {

                std::fill(m_text_updated_for_frame.at(index).begin(), m_text_updated_for_frame.at(index).end(), false);
                m_textures.at(index) = tex;
            }

            uint32_t current_frame = m_device_handle->getCurrentFrameID();
            //if(!m_text_updated_for_frame.at(index).at(current_frame)) {

                tex->writeDescriptorSet(m_pipeline.getShaderInputDescriptor(current_frame, m_current_draw_call), index + m_ubos.size(), current_frame);
            //    m_text_updated_for_frame.at(index).at(current_frame) = true;
            //}

        }

		void Renderer::draw(const VertexBuffer* vbo) {

            m_vbo = vbo;

            uint32_t current_frame = m_device_handle->getCurrentFrameID();

            m_render_pass.bindVertexBuffer(m_vbo);
            m_render_pass.bindDescriptorSets(m_pipeline.m_layout, m_pipeline.getShaderInputDescriptor(current_frame, m_current_draw_call));
            m_render_pass.draw(m_vbo->getVertexCount(), m_vbo->usesIndices(), m_vbo->getInstanceCount());

            m_current_draw_call++;
        }

        void Renderer::beginRenderPass(Framebuffer* fbo) {

            m_fbo = fbo;

            m_pipeline.setFramebufferLayout(*m_fbo);
            m_pipeline.setViewport(m_fbo->getWidth(), m_fbo->getHeight());

            // defining clear values for the framebuffer
            std::vector<vk::ClearValue> clear_values(2);
            clear_values.at(0).color = vk::ClearColorValue(std::array<float, 4>({0.05f, 0.05f, 0.05f, 1.0f}));
            clear_values.at(1).depthStencil = vk::ClearDepthStencilValue(1.0f, 0.0f);

            // recording the command buffer
            m_render_pass.beginRenderPass(m_pipeline.m_render_pass, m_fbo, &clear_values, {m_pipeline.m_view_width, m_pipeline.m_view_height});
            m_render_pass.bindPipeline(m_pipeline.m_pipeline);

            m_current_draw_call = 0;

        }

        void Renderer::endRenderPass() {
            // the renderpass will be executed by the gpu

            uint32_t current_frame = m_device_handle->getCurrentFrameID();

            // ending the renderpass
            m_render_pass.endRenderPass();

            // signal objects
            std::vector<vk::Semaphore> wait_signals = m_fbo->getImageReadySemaphores(current_frame);
            vk::Semaphore* finished_signal = &m_fbo->m_render_finished->at(current_frame); // signaled once rendering is finished
            vk::Fence* render_finished_fence = &m_render_finished->at(current_frame);

            // the stages at which to wait on the signals
            std::vector<vk::PipelineStageFlags> wait_stages(wait_signals.size(), vk::PipelineStageFlagBits::eColorAttachmentOutput); // the stage at which to wait

            // submitting the command buffer
            vk::SubmitInfo submit_info(wait_signals, wait_stages, {}, *finished_signal);
            m_render_pass.submit(m_device_handle->m_graphics_queue, &submit_info, render_finished_fence);

            m_render_started.at(current_frame) = true;
        }


    } // graphics

} // undicht
