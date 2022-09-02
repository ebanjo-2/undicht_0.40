#include "draw_call.h"

namespace undicht {

    namespace graphics {

        DrawCall::DrawCall(const GraphicsDevice* device) {

            m_device_handle = device;
            m_cmd_buffers = new std::vector<vk::CommandBuffer>;

            // create a command buffer for every frame
            unsigned max_frames = m_device_handle->getMaxFramesInFlight();
            vk::CommandBufferAllocateInfo allocate_info(*m_device_handle->m_graphics_command_pool, vk::CommandBufferLevel::ePrimary, max_frames);
            *m_cmd_buffers = m_device_handle->m_device->allocateCommandBuffers(allocate_info);

        }

        DrawCall::~DrawCall() {

            delete m_cmd_buffers;
        }



        //////////////////////////////////////////// recording commands ////////////////////////////////////////////

        void DrawCall::beginRenderPass(const vk::RenderPass* render_pass, const Framebuffer* fbo, std::vector<vk::ClearValue>* clear_values) {

            unsigned frame = m_device_handle->getCurrentFrameID();

            // resetting the command buffer
            m_cmd_buffers->at(frame).reset();

            // beginning the new command buffer
            vk::CommandBufferBeginInfo begin_info({}, nullptr);
            m_cmd_buffers->at(frame).begin(begin_info);

            // beginning the new render pass
            vk::Rect2D render_area(vk::Offset2D(0,0), vk::Extent2D(fbo->getWidth(), fbo->getHeight()));
            vk::RenderPassBeginInfo render_pass_info(*render_pass, *fbo->getCurrentFramebuffer(), render_area, *clear_values);
            m_cmd_buffers->at(frame).beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

            // next the commands should be called
        }

        void DrawCall::endRenderPass() {

            unsigned frame = m_device_handle->getCurrentFrameID();

            // ending the render pass
            m_cmd_buffers->at(frame).endRenderPass();

            // finishing the draw buffer
            m_cmd_buffers->at(frame).end();

        }

        //////////////////////////////////////////// commands ////////////////////////////////////////////

        void DrawCall::bindPipeline(const vk::Pipeline* pipe) {

            unsigned frame = m_device_handle->getCurrentFrameID();

            m_cmd_buffers->at(frame).bindPipeline(vk::PipelineBindPoint::eGraphics, *pipe);
        }

        void DrawCall::bindVertexBuffer(const VertexBuffer* vbo) {

            unsigned frame = m_device_handle->getCurrentFrameID();

            // binding the per vertex data
            m_cmd_buffers->at(frame).bindVertexBuffers(0, *vbo->m_vertex_data.m_buffer, {0});

            // binding the per instance data
            if(vbo->usesInstancing())
                m_cmd_buffers->at(frame).bindVertexBuffers(1, *vbo->m_instance_data.m_buffer, {0});

            if(vbo->usesIndices())
                m_cmd_buffers->at(frame).bindIndexBuffer(*vbo->m_index_data.m_buffer, 0, vk::IndexType::eUint32);

        }

        void DrawCall::bindDescriptorSets(const vk::PipelineLayout* layout, const vk::DescriptorSet* descriptors) {

            unsigned frame = m_device_handle->getCurrentFrameID();
            m_cmd_buffers->at(frame).bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *layout, 0, *descriptors, nullptr);

        }

        void DrawCall::draw(uint32_t vertex_count, bool use_indices, uint32_t instances) {

            unsigned frame = m_device_handle->getCurrentFrameID();

            if(use_indices) {

                m_cmd_buffers->at(frame).drawIndexed(vertex_count, instances, 0, 0, 0);
            } else {

                m_cmd_buffers->at(frame).draw(vertex_count, instances, 0, 0);
            }

        }

        /////////////////////////// submitting the command buffer onto a queue //////////////////////////

        void DrawCall::submit(vk::Queue* queue, vk::SubmitInfo* info, vk::Fence* finished_fence) {

            unsigned frame = m_device_handle->getCurrentFrameID();

            info->commandBufferCount = 1;
            info->pCommandBuffers = &m_cmd_buffers->at(frame);

            queue->submit(1, info, *finished_fence);
        }

    }

}