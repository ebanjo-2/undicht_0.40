#ifndef RENDERER_H
#define RENDERER_H

#include "core/vulkan/vulkan_declaration.h"
#include "buffer_layout.h"
#include "core/vulkan/swap_chain.h"

#include "graphics_pipeline/vulkan/shader.h"
#include "graphics_pipeline/vulkan/render_subpass.h"
#include "graphics_pipeline/vulkan/vertex_buffer.h"
#include "graphics_pipeline/vulkan/uniform_buffer.h"
#include "graphics_pipeline/vulkan/texture.h"

namespace undicht {

	namespace graphics {

		class GraphicsDevice;

		class Renderer {
		
		private:

			// pipeline layout (settings)
            std::vector<vk::VertexInputBindingDescription>* m_vertex_bindings = 0;
            std::vector<vk::VertexInputAttributeDescription>* m_vertex_attributes = 0;
            vk::DescriptorSetLayout* m_shader_layout = 0;
            vk::DescriptorPool* m_shader_input_descriptor_pool = 0;
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
			std::vector<vk::CommandBuffer>* m_cmd_buffer = 0;

            // currently submitted objects
            const VertexBuffer* m_vbo = 0;
            std::vector<const UniformBuffer*> m_ubos;
            std::vector<const Texture*> m_textures;

            uint32_t m_max_frames_in_flight = 1;
            uint32_t m_current_frame = 0;

			const GraphicsDevice* m_device_handle = 0;
			const Shader* m_shader_handle = 0;
			
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
			void createCommandBuffers();
            void createShaderInputDescriptorPool();

		public:
			// pipeline settings

            void setMaxFramesInFlight(uint32_t num);
            void setCurrentFrameID(uint32_t frame);

			void setVertexBufferLayout(const VertexBuffer& vbo_prototype);
            void setShaderInput(uint32_t ubo_count, uint32_t tex_count);
			void setShader(Shader* shader);
			void setRenderTarget(SwapChain* swap_chain);
            void updateRenderTarget(SwapChain* swap_chain);
			// void setRenderTarget(const FrameBuffer& frame_buffer);

        private:
            // functions for creating settings objects

            vk::PipelineVertexInputStateCreateInfo getVertexInputState() const;
            vk::Viewport getViewport() const;
            vk::Rect2D getScissor() const; // the part of the viewport that gets displayed
            vk::PipelineLayoutCreateInfo getShaderInputLayout() const;

        public:
			// drawing

            void submit(const VertexBuffer* vbo);
            void submit(const UniformBuffer* ubo, uint32_t index);
            void submit(const Texture* tex, uint32_t index); // the texture index starts after the last ubo index

			void draw();

		private:

            void bindVertexBuffer(vk::CommandBuffer* cmd, const VertexBuffer* vbo);
            void bindUniformBuffer(vk::CommandBuffer* cmd, const UniformBuffer* ubo, uint32_t index);
            void bindTexture(vk::CommandBuffer* cmd, const Texture* tex, uint32_t index);

            void recordCommandBuffer(vk::CommandBuffer* cmd_buffer, const VertexBuffer* vbo);
			void submitCommandBuffer(vk::CommandBuffer* cmd_buffer, std::vector<vk::Semaphore>* wait_on, vk::Semaphore* render_finished, vk::Fence* render_finished_fence);

        public:
            // creating types that depend on the layout of the render pipeline
            // should only be created once the pipeline was linked

            UniformBuffer createUniformBuffer();

		};

	} // graphics

} // undicht

#endif // RENDERER_H
