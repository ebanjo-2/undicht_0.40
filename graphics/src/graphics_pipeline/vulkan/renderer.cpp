#include "renderer.h"
#include "debug.h"


#include "vulkan/vulkan.hpp"

#include "vector"
#include "tuple"

#include "core/vulkan/graphics_device.h"

namespace undicht {

	namespace graphics {
		
		Renderer::Renderer(const GraphicsDevice* device) {
			
			m_device_handle = device->m_device;
			m_graphics_queue_handle = device->m_queues.graphics_queue;
			
			m_layout = new vk::PipelineLayout;
			m_render_pass = new vk::RenderPass;
			m_pipeline = new vk::Pipeline;

			m_swap_frame_buffers = new std::vector<vk::Framebuffer>;

			// creating the command pool
			m_graphics_cmds = new vk::CommandPool;
			m_cmd_buffer = new vk::CommandBuffer;

			vk::CommandPoolCreateInfo cmd_pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, device->m_queue_family_ids.graphics_queue);
			*m_graphics_cmds = m_device_handle->createCommandPool(cmd_pool_info);
		}

		Renderer::~Renderer() {

			if(m_graphics_cmds) {
				m_device_handle->destroyCommandPool(*m_graphics_cmds);
				delete m_graphics_cmds;
			}

			if(m_cmd_buffer)
				delete m_cmd_buffer;

			if(m_swap_frame_buffers) {
				for(vk::Framebuffer fbo : (*m_swap_frame_buffers))
					m_device_handle->destroyFramebuffer(fbo);

				delete m_swap_frame_buffers;
			}

			if(m_layout) {
				m_device_handle->destroyPipelineLayout(*m_layout);
				delete m_layout;
			}

			if(m_render_pass) {
				m_device_handle->destroyRenderPass(*m_render_pass);		
				delete m_render_pass;
			}

			if(m_pipeline) {
				m_device_handle->destroyPipeline(*m_pipeline);
				delete m_pipeline;
			}
		}


		void Renderer::linkPipeline() {

			if(!m_shader_handle) {
				UND_ERROR << "failed to create graphics pipeline: no shader was submitted\n";
				return;
			}

			// info about the fixed pipeline stages
			vk::PipelineVertexInputStateCreateInfo vertex_input({}, 0, nullptr, 0, nullptr);
			vk::PipelineInputAssemblyStateCreateInfo input_assembly({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
			vk::Viewport viewport(0, 0, 800, 600, 0.0f, 1.0f);
			vk::Rect2D scissor(vk::Offset2D(0,0), vk::Extent2D(800, 600));
			vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
			vk::PipelineRasterizationStateCreateInfo rasterizer({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);	
			vk::PipelineMultisampleStateCreateInfo multisample({}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
		   	vk::PipelineColorBlendAttachmentState color_blend_attachment({}, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);
			vk::PipelineColorBlendStateCreateInfo color_blending({}, VK_FALSE, vk::LogicOp::eCopy, 1, &color_blend_attachment, {0.0f, 0.0f, 0.0f, 0.0f});
			
			// settings that can be changed later
			std::vector<vk::DynamicState> dynamic_states({vk::DynamicState::eViewport, vk::DynamicState::eLineWidth});
			vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states); 
			
			// creating the pipeline info
			vk::PipelineLayoutCreateInfo pipeline_layout({}, 0, nullptr, 0, nullptr);
			*m_layout = m_device_handle->createPipelineLayout(pipeline_layout);
			
			// determines how images are used during the rendering
			std::vector<vk::AttachmentDescription> attachments;
			std::vector<vk::AttachmentReference> attachment_refs;
			getTextureAttachments(&attachments, &attachment_refs);

			// creating subpasses
			m_subpass.setAttachments(&attachment_refs);
			std::vector<vk::SubpassDescription> subpasses({*m_subpass.m_description});

			// creating the render pass
			vk::SubpassDependency subpass_dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);
			std::vector<vk::SubpassDependency> subpass_dependencies({subpass_dependency});
			vk::RenderPassCreateInfo render_pass_info({}, attachments, subpasses, subpass_dependencies);
			*m_render_pass = m_device_handle->createRenderPass(render_pass_info);

			// creating the pipeline
			vk::GraphicsPipelineCreateInfo pipeline_info({}, *m_shader_handle->m_stages);
			pipeline_info.setPVertexInputState(&vertex_input);
			pipeline_info.setPInputAssemblyState(&input_assembly);
			pipeline_info.setPViewportState(&viewport_state);
			pipeline_info.setPRasterizationState(&rasterizer);
			pipeline_info.setPMultisampleState(&multisample);
			pipeline_info.setPDepthStencilState(nullptr);
			pipeline_info.setPColorBlendState(&color_blending);
			pipeline_info.setPDynamicState(nullptr); // optional

			pipeline_info.setLayout(*m_layout);
			pipeline_info.setRenderPass(*m_render_pass);
			pipeline_info.setSubpass(0); // index of the subpass
				
			vk::Result result;
			std::tie(result, *m_pipeline) = m_device_handle->createGraphicsPipeline(nullptr, pipeline_info);

			if(result != vk::Result::eSuccess)
				UND_ERROR << "failed to create graphics pipeline\n";

			// creating swap chain framebuffers
			for(int i = 0; i < m_swap_chain_handle->m_image_count; i++) {
				vk::FramebufferCreateInfo fbo_info({}, *m_render_pass, 1, &m_swap_chain_handle->m_image_views->at(i), m_swap_chain_handle->getWidth(), m_swap_chain_handle->getHeight(), 1);
				m_swap_frame_buffers->push_back(m_device_handle->createFramebuffer(fbo_info));
			}

			// creating the command buffer
			vk::CommandBufferAllocateInfo allocate_info(*m_graphics_cmds, vk::CommandBufferLevel::ePrimary, 1);
			*m_cmd_buffer = m_device_handle->allocateCommandBuffers(allocate_info)[0];

		}


		//////////////////////////////// pipeline settings /////////////////////////////////////

		void Renderer::setVertexLayout(const BufferLayout& layout) {

			m_vertex_layout = layout;
		}

		void Renderer::setShader(Shader* shader) {
			
			m_shader_handle = shader;
		}

		void Renderer::setRenderTarget(SwapChain* swap_chain) {
			
			m_swap_chain_handle = swap_chain;
		}

		/////////////////////////////////////// drawing /////////////////////////////////////


		void Renderer::draw() {

			// getting the framebuffer that gets drawn to
			int image_index = m_swap_chain_handle->acquireNextImage();
			vk::Framebuffer* fbo = &m_swap_frame_buffers->at(image_index);

			// viewport size
			vk::Rect2D render_area({0,0}, {m_swap_chain_handle->getWidth(), m_swap_chain_handle->getHeight()});

			// clear value
			std::vector<vk::ClearValue> clear_values = {vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}))};

			// recording the draw buffer
			m_cmd_buffer->reset();
			
			vk::CommandBufferBeginInfo begin_info({}, nullptr);
			m_cmd_buffer->begin(begin_info);

			// beginning the render pass
			vk::RenderPassBeginInfo render_pass_info(*m_render_pass, *fbo, render_area, clear_values);
			m_cmd_buffer->beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

			// draw commands
			m_cmd_buffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
			m_cmd_buffer->draw(3, 1, 0, 0);

			// ending the render pass
			m_cmd_buffer->endRenderPass();

			// finishing the draw buffer
			m_cmd_buffer->end();


			// submit the command buffer
			std::vector<vk::Semaphore> wait_before_draw({*m_swap_chain_handle->m_image_available});
			std::vector<vk::PipelineStageFlags> wait_stage({vk::PipelineStageFlagBits::eColorAttachmentOutput}); // the stage at which to wait
			std::vector<vk::CommandBuffer> cmd_buffers({*m_cmd_buffer});
			std::vector<vk::Semaphore> signal_once_finished({*m_swap_chain_handle->m_render_finished});
			vk::SubmitInfo submit_info(wait_before_draw, wait_stage, cmd_buffers, signal_once_finished);
			m_graphics_queue_handle->submit(1, &submit_info, *m_swap_chain_handle->m_frame_in_flight);	
		}


		///////////////////////////////// private functions ///////////////////////////////////

		void Renderer::getTextureAttachments(std::vector<vk::AttachmentDescription>* attachments, std::vector<vk::AttachmentReference>* refs) {

			if(m_swap_chain_handle) {
				
				// creating the color attachment
				vk::AttachmentDescription color_attachment({}, m_swap_chain_handle->m_format->format, vk::SampleCountFlagBits::e1);
				color_attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
				color_attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
				color_attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				color_attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
				color_attachment.setInitialLayout(vk::ImageLayout::eUndefined);
				color_attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
				
				// storing the attachments
				attachments->push_back(color_attachment);

				// creating reference objects
				vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
				refs->push_back(color_ref);

			} else {

				UND_ERROR << "no swap chain submitted to renderer\n";
			}

		}	

	} // graphics

} // undicht
