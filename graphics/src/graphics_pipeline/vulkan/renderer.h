#ifndef RENDERER_H
#define RENDERER_H

#include "core/vulkan/vulkan_declaration.h"
#include "buffer_layout.h"
#include "core/vulkan/swap_chain.h"

#include "graphics_pipeline/vulkan/shader.h"
#include "graphics_pipeline/vulkan/render_subpass.h"


namespace undicht {

	namespace graphics {

		class GraphicsDevice;

		class Renderer {
		
		private:

			// pipeline layout
			BufferLayout m_vertex_layout;
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
			vk::CommandBuffer* m_cmd_buffer = 0;


			vk::Device* m_device_handle = 0;
			vk::Queue* m_graphics_queue_handle = 0;
			Shader* m_shader_handle = 0;
			
			friend GraphicsDevice;

			Renderer(const GraphicsDevice* device);
		
		public:

			~Renderer();

			void linkPipeline();

		public:
			// pipeline settings 

			void setVertexLayout(const BufferLayout& layout);
			void setShader(Shader* shader);
			void setRenderTarget(SwapChain* swap_chain);
			// void setRenderTarget(const FrameBuffer& frame_buffer);
			
			// drawing
			void draw();

				
		private:

			void getTextureAttachments(std::vector<vk::AttachmentDescription>* attachments, std::vector<vk::AttachmentReference>* refs); 	


		};

	} // graphics

} // undicht

#endif // RENDERER_H
