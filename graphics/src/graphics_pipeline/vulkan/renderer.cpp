#include "renderer.h"
#include "debug.h"


#include "vulkan/vulkan.hpp"

#include "vector"
#include "tuple"

#include "core/vulkan/graphics_device.h"

namespace undicht {

	namespace graphics {
		
		Renderer::Renderer(const GraphicsDevice* device) {
			
			// storing handles
			m_device_handle = device->m_device;
            m_graphics_queue_index = device->m_queue_family_ids.graphics_queue;
			m_graphics_queue_handle = device->m_queues.graphics_queue;
			
			// creating the command pool
			m_graphics_cmds = new vk::CommandPool;
			m_cmd_buffer = new std::vector<vk::CommandBuffer>;

			// creating other member objects
			m_layout = new vk::PipelineLayout;
			m_render_pass = new vk::RenderPass;
			m_pipeline = new vk::Pipeline;
			m_swap_frame_buffers = new std::vector<vk::Framebuffer>;

		}

		Renderer::~Renderer() {

            cleanUp();

            delete m_swap_frame_buffers;
			delete m_graphics_cmds;
			delete m_cmd_buffer;
			delete m_layout;
			delete m_render_pass;
			delete m_pipeline;
		}

        void Renderer::cleanUp() {
            // destroys all vulkan objects, but keeps the settings
            // you then can change some of the settings, and link the pipeline once more
            // to use it again

            m_device_handle->waitIdle();

            for(vk::Framebuffer fbo : (*m_swap_frame_buffers))
                m_device_handle->destroyFramebuffer(fbo);

            m_device_handle->destroyCommandPool(*m_graphics_cmds);
            m_device_handle->destroyPipelineLayout(*m_layout);
            m_device_handle->destroyRenderPass(*m_render_pass);
            m_device_handle->destroyPipeline(*m_pipeline);

        }

        ////////////////////////////////////////////// creating the pipeline ///////////////////////////////////////////

		void Renderer::linkPipeline() {

			if(!m_shader_handle) {
				UND_ERROR << "failed to create graphics pipeline: no shader was submitted\n";
				return;
			}

			createRenderPass();
			createSwapChainFrameBuffers();
            createCommandPool();
			createCommandBuffers();

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

		}

		///////////////////////////////// private functions for creating the pipeline ///////////////////////////////////

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

		void Renderer::createRenderPass() {

			// declaring which images are used during the rendering
			std::vector<vk::AttachmentDescription> attachments;
			std::vector<vk::AttachmentReference> attachment_refs;
			getTextureAttachments(&attachments, &attachment_refs);

			// creating subpasses
			m_subpass.setAttachments(&attachment_refs);
			std::vector<vk::SubpassDescription> subpasses({*m_subpass.m_description});

			// declaring the stages the subpass depends on
			vk::SubpassDependency subpass_dependency(VK_SUBPASS_EXTERNAL, 0);
		   subpass_dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		   subpass_dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		   subpass_dependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
		   subpass_dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
			std::vector<vk::SubpassDependency> subpass_dependencies({subpass_dependency});

			// creating the render pass
			vk::RenderPassCreateInfo render_pass_info({}, attachments, subpasses, subpass_dependencies);
			*m_render_pass = m_device_handle->createRenderPass(render_pass_info);

		}	

		void Renderer::createSwapChainFrameBuffers() {
            // creating frame buffers to access the swap chain images

            m_swap_frame_buffers->resize(m_swap_chain_handle->m_image_count);

			for(int i = 0; i < m_swap_chain_handle->m_image_count; i++) {

				vk::FramebufferCreateInfo fbo_info({}, *m_render_pass, 1, &m_swap_chain_handle->m_image_views->at(i));
				fbo_info.setWidth(m_swap_chain_handle->getWidth());
				fbo_info.setHeight(m_swap_chain_handle->getHeight());
				fbo_info.setLayers( 1);

				m_swap_frame_buffers->at(i) = m_device_handle->createFramebuffer(fbo_info);
			}

		}

        void Renderer::createCommandPool() {

            vk::CommandPoolCreateInfo cmd_pool_info;
            cmd_pool_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
            cmd_pool_info.setQueueFamilyIndex(m_graphics_queue_index);
            *m_graphics_cmds = m_device_handle->createCommandPool(cmd_pool_info);

        }

		void Renderer::createCommandBuffers() {

            if(!m_swap_chain_handle) {
                UND_ERROR << "no swap chain was submitted to the renderer\n";
                return;
            }

            int max_frames_in_flight = m_swap_chain_handle->getMaxFramesInFlight();

            // creating a command buffer for each frame in flight
			vk::CommandBufferAllocateInfo allocate_info(*m_graphics_cmds, vk::CommandBufferLevel::ePrimary, max_frames_in_flight);
			*m_cmd_buffer = m_device_handle->allocateCommandBuffers(allocate_info);

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

        void Renderer::updateRenderTarget(SwapChain* swap_chain) {

            cleanUp();
            linkPipeline();
        }

		/////////////////////////////////////// drawing /////////////////////////////////////


		void Renderer::draw() {

            int current_frame = m_swap_chain_handle->getCurrentFrameID();

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

        void Renderer::recordCommandBuffer(vk::CommandBuffer* cmd_buffer) {

            // getting the framebuffer that gets drawn to
            int image_index = m_swap_chain_handle->getCurrentImageID();
            vk::Framebuffer* fbo = &m_swap_frame_buffers->at(image_index);

            // viewport size
            vk::Rect2D render_area({0,0}, {m_swap_chain_handle->getWidth(), m_swap_chain_handle->getHeight()});

            // clear value
            std::vector<vk::ClearValue> clear_values = {vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}))};

            // recording the draw buffer
            cmd_buffer->reset();

            vk::CommandBufferBeginInfo begin_info({}, nullptr);
            cmd_buffer->begin(begin_info);

            // beginning the render pass
            vk::RenderPassBeginInfo render_pass_info(*m_render_pass, *fbo, render_area, clear_values);
            cmd_buffer->beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

            // draw commands
            cmd_buffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
            cmd_buffer->draw(3, 1, 0, 0);

            // ending the render pass
            cmd_buffer->endRenderPass();

            // finishing the draw buffer
            cmd_buffer->end();
        }

		void Renderer::submitCommandBuffer(vk::CommandBuffer* cmd_buffer, std::vector<vk::Semaphore>* wait_on, vk::Semaphore* render_finished, vk::Fence* render_finished_fence) {

			// waiting on other processes to finish
			std::vector<vk::PipelineStageFlags> wait_stages(wait_on->size(), vk::PipelineStageFlagBits::eColorAttachmentOutput); // the stage at which to wait

			// sync object that gets signaled once drawing finishes
			std::vector<vk::Semaphore> signal_once_finished({*render_finished});

			// the command buffers to submit
			std::vector<vk::CommandBuffer> cmd_buffers({*cmd_buffer});

			vk::SubmitInfo submit_info(*wait_on, wait_stages, cmd_buffers, signal_once_finished);
			m_graphics_queue_handle->submit(1, &submit_info, *render_finished_fence);

		}

	} // graphics

} // undicht
