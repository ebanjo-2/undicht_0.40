#ifndef RENDERER_H
#define RENDERER_H

#include "core/vulkan/vulkan_declaration.h"
#include "buffer_layout.h"
#include "core/vulkan/swap_chain.h"

#include "graphics_pipeline/vulkan/shader.h"
#include "graphics_pipeline/vulkan/render_subpass.h"
#include "graphics_pipeline/vulkan/vertex_buffer.h"
#include "graphics_pipeline/vulkan/uniform_buffer.h"

namespace undicht {

	namespace graphics {

		class GraphicsDevice;

		class Renderer {
		
		private:

			// pipeline layout (settings)
            std::vector<vk::VertexInputBindingDescription>* m_vertex_bindings = 0;
            std::vector<vk::VertexInputAttributeDescription>* m_vertex_attributes = 0;
            vk::DescriptorSetLayout* m_uniform_buffer_layout = 0;
            bool m_use_uniform_buffer = false;
			vk::PipelineLayout* m_layout = 0;

			// subpasses
			RenderSubpass m_subpass;

			// the renderpass
			vk::RenderPass* m_render_pass = 0;

			// pipeline
			vk::Pipeline* m_pipeline = 0;
				
			// swapchain
			SwapChain* m_swap_chain_handle = 0;
			std::vector<vk::Framebuffer> *m_swap_frame_buffers = 0;

			// command pool
			vk::CommandPool* m_graphics_cmds = 0;
			std::vector<vk::CommandBuffer>* m_cmd_buffer = 0;

            // currently submitted objects
            const VertexBuffer* m_vbo = 0;
            const UniformBuffer* m_ubo = 0;

            uint32_t m_max_frames_in_flight = 1;
            uint32_t m_current_frame = 0;

			vk::Device* m_device_handle = 0;
            uint32_t m_graphics_queue_index = 0;
			vk::Queue* m_graphics_queue_handle = 0;
			Shader* m_shader_handle = 0;
			
			friend GraphicsDevice;

			Renderer(const GraphicsDevice* device);

        public:

            ~Renderer();

        private:

            // destroys all vulkan objects, but keeps the settings
            // you then can change some of the settings, and link the pipeline once more
            // to use it again
            void cleanUp();

		public:

			void linkPipeline();

		private:
            // functions for creating the pipeline

			void getTextureAttachments(std::vector<vk::AttachmentDescription>* attachments, std::vector<vk::AttachmentReference>* refs);

			void createRenderPass();
			void createSwapChainFrameBuffers();
            void createCommandPool();
			void createCommandBuffers();

		public:
			// pipeline settings

            void setMaxFramesInFlight(uint32_t num);
            void setCurrentFrameID(uint32_t frame);

            void setUniformBufferLayout(const UniformBuffer& ubo_prototype);
			void setVertexBufferLayout(const VertexBuffer& vbo_prototype);
			void setShader(Shader* shader);
			void setRenderTarget(SwapChain* swap_chain);
            void updateRenderTarget(SwapChain* swap_chain);
			// void setRenderTarget(const FrameBuffer& frame_buffer);

        private:
            // functions for creating settings objects

            vk::PipelineVertexInputStateCreateInfo getVertexInputState() const;
            vk::Viewport getViewport() const;
            vk::Rect2D getScissor() const; // the part of the viewport that gets displayed
            vk::PipelineLayoutCreateInfo getUniformLayout() const;

        public:
			// drawing

            void submit(const VertexBuffer& vbo);
            void submit(const UniformBuffer& ubo);

			void draw();

		private:

            void recordCommandBuffer(vk::CommandBuffer* cmd_buffer, const VertexBuffer* vbo, const UniformBuffer* ubo);
			void submitCommandBuffer(vk::CommandBuffer* cmd_buffer, std::vector<vk::Semaphore>* wait_on, vk::Semaphore* render_finished, vk::Fence* render_finished_fence);

		};

	} // graphics

} // undicht

#endif // RENDERER_H
