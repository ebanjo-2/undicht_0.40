#include "renderer.h"
#include "debug.h"


#include "vulkan/vulkan.hpp"

#include "vector"
#include "tuple"

#include "core/vulkan/graphics_device.h"

namespace undicht {

	namespace graphics {
		
		Renderer::Renderer(const GraphicsDevice* device) : Pipeline(device){

			// creating the command buffers
			m_cmd_buffer = new std::vector<vk::CommandBuffer>;

			// creating other member objects
			m_swap_frame_buffers = new std::vector<vk::Framebuffer>;

		}

		Renderer::~Renderer() {

            cleanUp();

            // actual pipeline objects
            delete m_swap_frame_buffers;
			delete m_cmd_buffer;

		}

        void Renderer::cleanUp() {

            m_device_handle->m_device->waitIdle();

            for(vk::Framebuffer fbo : (*m_swap_frame_buffers))
                m_device_handle->m_device->destroyFramebuffer(fbo);

        }

        ////////////////////////////////////////////// creating the renderer ///////////////////////////////////////////


        void Renderer::setShaderInput(uint32_t ubo_count, uint32_t tex_count) {

            m_ubos.resize(ubo_count);
            m_textures.resize(tex_count);

            m_ubos_updated_for_frame.resize(ubo_count, std::vector<bool>(m_device_handle->getMaxFramesInFlight()));
            m_text_updated_for_frame.resize(tex_count, std::vector<bool>(m_device_handle->getMaxFramesInFlight()));

            Pipeline::setShaderInput(ubo_count, tex_count);
        }

		void Renderer::linkPipeline() {

            Pipeline::linkPipeline();

            createCommandBuffers();
            createSwapChainFrameBuffers();

        }

		///////////////////////////////// private functions for creating the renderer ///////////////////////////////////


		void Renderer::createSwapChainFrameBuffers() {
            // creating frame buffers to access the swap chain images

            m_swap_frame_buffers->resize(m_swap_chain_handle->m_image_count);

			for(int i = 0; i < m_swap_chain_handle->m_image_count; i++) {

				vk::FramebufferCreateInfo fbo_info({}, *m_render_pass, 1, &m_swap_chain_handle->m_image_views->at(i));
				fbo_info.setWidth(m_swap_chain_handle->getWidth());
				fbo_info.setHeight(m_swap_chain_handle->getHeight());
				fbo_info.setLayers( 1);

				m_swap_frame_buffers->at(i) = m_device_handle->m_device->createFramebuffer(fbo_info);
			}

		}

		void Renderer::createCommandBuffers() {

            if(!m_swap_chain_handle) {
                UND_ERROR << "no swap chain was submitted to the renderer\n";
                return;
            }

            // creating a command buffer for each frame in flight
			vk::CommandBufferAllocateInfo allocate_info(*m_device_handle->m_graphics_command_pool, vk::CommandBufferLevel::ePrimary, m_device_handle->getMaxFramesInFlight());
			*m_cmd_buffer = m_device_handle->m_device->allocateCommandBuffers(allocate_info);

		}

		/////////////////////////////////////// drawing /////////////////////////////////////

        void Renderer::submit(const VertexBuffer *vbo) {

            m_vbo = vbo;
        }

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
            if(!m_ubos_updated_for_frame.at(index).at(current_frame)) {

                ubo->writeDescriptorSets(m_shader_descriptors, index, current_frame);
                m_ubos_updated_for_frame.at(index).at(current_frame) = true;
            }

            ubo->updateBuffer(current_frame);

        }

        void Renderer::submit(const Texture* tex, uint32_t index) {

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
            if(!m_text_updated_for_frame.at(index).at(current_frame)) {

                tex->writeDescriptorSets(m_shader_descriptors, index + m_ubos.size(), current_frame);
                m_text_updated_for_frame.at(index).at(current_frame) = true;
            }

        }

		void Renderer::draw() {

            if(!m_vbo) {
                UND_ERROR << "failed to draw: no vbo submitted\n";
                return;
            }

            uint32_t current_frame = m_device_handle->getCurrentFrameID();

            // getting the objects that belong to this frame
            vk::CommandBuffer* cmd = &m_cmd_buffer->at(current_frame);
            vk::Semaphore* image_ready = &m_swap_chain_handle->m_image_available->at(current_frame);
            vk::Semaphore* render_finished = &m_swap_chain_handle->m_render_finished->at(current_frame);
            vk::Fence* render_finished_fence = &m_swap_chain_handle->m_frame_in_flight->at(current_frame);

            // the semaphores to wait on before drawing to the render target
            std::vector<vk::Semaphore> wait_on({*image_ready});

            // record the command buffer
            recordCommandBuffer(cmd);

			// submit the command buffer
			submitCommandBuffer(cmd, &wait_on, render_finished, render_finished_fence);
		}

		/////////////////////////////////////// private draw functions ////////////////////////////

        void Renderer::bindVertexBuffer(vk::CommandBuffer* cmd) {

            // binding the per vertex data
            cmd->bindVertexBuffers(0, *m_vbo->m_vertex_data.m_buffer, {0});

            // binding the per instance data
            if(m_vbo->usesInstancing())
                cmd->bindVertexBuffers(1, *m_vbo->m_instance_data.m_buffer, {0});
        }

        void Renderer::bindDescriptorSets(vk::CommandBuffer* cmd) {

            cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_layout, 0, m_shader_descriptors->at(m_device_handle->getCurrentFrameID()), nullptr);

        }


        void Renderer::recordCommandBuffer(vk::CommandBuffer* cmd_buffer) {

            // getting the framebuffer that gets drawn to
            int image_index = m_swap_chain_handle->getCurrentImageID();
            vk::Framebuffer* fbo = &m_swap_frame_buffers->at(image_index);

            // viewport size
            vk::Rect2D render_area = getScissor();

            // clear value
            std::vector<vk::ClearValue> clear_values = {vk::ClearColorValue(std::array<float, 4>({0.05f, 0.05f, 0.05f, 1.0f}))};

            // recording the draw buffer
            cmd_buffer->reset();

            vk::CommandBufferBeginInfo begin_info({}, nullptr);
            cmd_buffer->begin(begin_info);

            // beginning the render pass
            vk::RenderPassBeginInfo render_pass_info(*m_render_pass, *fbo, render_area, clear_values);
            cmd_buffer->beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

            // draw commands
            recordDrawCommands(cmd_buffer);

            // ending the render pass
            cmd_buffer->endRenderPass();

            // finishing the draw buffer
            cmd_buffer->end();
        }

        void Renderer::recordDrawCommands(vk::CommandBuffer* cmd_buffer) {

            cmd_buffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
            bindVertexBuffer(cmd_buffer);
            bindDescriptorSets(cmd_buffer);

            if(m_vbo->usesIndices()) {
                cmd_buffer->bindIndexBuffer(*m_vbo->m_index_data.m_buffer, 0, vk::IndexType::eUint32);
                cmd_buffer->drawIndexed(m_vbo->getVertexCount(), m_vbo->getInstanceCount(), 0, 0, 0);
            } else {
                cmd_buffer->draw(m_vbo->getVertexCount(), m_vbo->getInstanceCount(), 0, 0);
            }

        }

		void Renderer::submitCommandBuffer(vk::CommandBuffer* cmd_buffer, std::vector<vk::Semaphore>* wait_on, vk::Semaphore* render_finished, vk::Fence* render_finished_fence) {

			// waiting on other processes to finish
			std::vector<vk::PipelineStageFlags> wait_stages(wait_on->size(), vk::PipelineStageFlagBits::eColorAttachmentOutput); // the stage at which to wait

			// sync object that gets signaled once drawing finishes
			std::vector<vk::Semaphore> signal_once_finished({*render_finished});

			// the command buffers to submit
			std::vector<vk::CommandBuffer> cmd_buffers({*cmd_buffer});

			vk::SubmitInfo submit_info(*wait_on, wait_stages, cmd_buffers, signal_once_finished);
			m_device_handle->m_graphics_queue->submit(1, &submit_info, *render_finished_fence);

		}


	} // graphics

} // undicht
