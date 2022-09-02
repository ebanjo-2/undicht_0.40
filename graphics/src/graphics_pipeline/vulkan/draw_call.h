#ifndef DRAW_CALL_H
#define DRAW_CALL_H

#include "core/vulkan/vulkan_declaration.h"
#include "core/vulkan/graphics_device.h"

#include "graphics_pipeline/vulkan/vertex_buffer.h"

namespace undicht {

    namespace graphics {

        class Renderer;

        class DrawCall {

            friend Renderer;

        private:

            const GraphicsDevice* m_device_handle = 0;

            std::vector<vk::CommandBuffer>* m_cmd_buffers = 0;

        public:

            DrawCall(const GraphicsDevice* device);
            virtual ~DrawCall();

        public:
            // recording commands

            void beginRenderPass(const vk::RenderPass* render_pass, const Framebuffer* fbo, std::vector<vk::ClearValue>* clear_values); // call this before recording any commands
            void endRenderPass();

        public:
            // commands

            void bindPipeline(const vk::Pipeline* pipe);
            void bindVertexBuffer(const VertexBuffer* vbo);
            void bindDescriptorSets(const vk::PipelineLayout* layout, const vk::DescriptorSet* descriptors);
            void draw(uint32_t vertex_count, bool use_indices = false, uint32_t instances = 1);

        public:
            // submitting the command buffer onto a queue

            void submit(vk::Queue* queue, vk::SubmitInfo* info, vk::Fence* finished_fence = 0);

        };

    }

}

#endif