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
			m_device_handle = device;

            // settings
            m_vertex_bindings = new std::vector<vk::VertexInputBindingDescription>;
            m_vertex_attributes = new std::vector<vk::VertexInputAttributeDescription>;
            m_shader_layout = new vk::DescriptorSetLayout;
            m_shader_input_descriptor_pool = new vk::DescriptorPool;
            m_shader_descriptors = new std::vector<vk::DescriptorSet>;

			// creating the command buffers
			m_cmd_buffer = new std::vector<vk::CommandBuffer>;

			// creating other member objects
			m_layout = new vk::PipelineLayout;
			m_render_pass = new vk::RenderPass;
			m_pipeline = new vk::Pipeline;
			m_swap_frame_buffers = new std::vector<vk::Framebuffer>;

		}

		Renderer::~Renderer() {

            cleanUp();

            // settings
            delete m_vertex_bindings;
            delete m_vertex_attributes;
            delete m_shader_layout;
            delete m_shader_input_descriptor_pool;
            delete m_shader_descriptors;

            // actual pipeline objects
            delete m_swap_frame_buffers;
			delete m_cmd_buffer;
			delete m_layout;
			delete m_render_pass;
			delete m_pipeline;
		}

        void Renderer::cleanUp() {
            // destroys all vulkan objects, but keeps the settings
            // you then can change some of the settings, and link the pipeline once more
            // to use it again

            m_device_handle->m_device->waitIdle();

            destroyStaticPipelineObjects();
            destroyDynamicPipelineObjects();

        }

        ////////////////////////////////////////////// creating the pipeline ///////////////////////////////////////////

		void Renderer::linkPipeline() {

			if(!m_shader_handle) {
				UND_ERROR << "failed to create graphics pipeline: no shader was submitted\n";
				return;
			}

            initStaticPipelineObjects();
            initDynamicPipelineObjects();

		}

		///////////////////////////////// private functions for creating the pipeline ///////////////////////////////////

        void Renderer::initStaticPipelineObjects() {

            createCommandBuffers();
            createShaderInputLayout();
            createShaderInputDescriptorPool();
            createShaderInputDescriptors();
        }

        void Renderer::initDynamicPipelineObjects() {

            createRenderPass(); // depends on swapchain images
            createSwapChainFrameBuffers();

            // info about the fixed pipeline stages
            vk::PipelineVertexInputStateCreateInfo vertex_input = getVertexInputState();
            vk::PipelineInputAssemblyStateCreateInfo input_assembly({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
            vk::Viewport viewport = getViewport();
            vk::Rect2D scissor = getScissor();
            vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
            vk::PipelineRasterizationStateCreateInfo rasterizer({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
            vk::PipelineMultisampleStateCreateInfo multisample({}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
            vk::PipelineColorBlendAttachmentState color_blend_attachment({}, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);
            vk::PipelineColorBlendStateCreateInfo color_blending({}, VK_FALSE, vk::LogicOp::eCopy, 1, &color_blend_attachment, {0.0f, 0.0f, 0.0f, 0.0f});

            // settings that can be changed later
            std::vector<vk::DynamicState> dynamic_states({vk::DynamicState::eViewport, vk::DynamicState::eLineWidth});
            vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states);

            // creating the pipeline layout (shader uniforms)
            vk::PipelineLayoutCreateInfo pipeline_layout = getShaderInputLayout();
            *m_layout = m_device_handle->m_device->createPipelineLayout(pipeline_layout);

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
            std::tie(result, *m_pipeline) = m_device_handle->m_device->createGraphicsPipeline(nullptr, pipeline_info);

            if(result != vk::Result::eSuccess)
                UND_ERROR << "failed to create graphics pipeline\n";

        }

        void Renderer::destroyStaticPipelineObjects() {

            m_device_handle->m_device->destroyDescriptorPool(*m_shader_input_descriptor_pool);
            m_device_handle->m_device->destroyDescriptorSetLayout(*m_shader_layout);

        }

        void Renderer::destroyDynamicPipelineObjects() {

            for(vk::Framebuffer fbo : (*m_swap_frame_buffers))
                m_device_handle->m_device->destroyFramebuffer(fbo);

            m_device_handle->m_device->destroyPipelineLayout(*m_layout);
            m_device_handle->m_device->destroyRenderPass(*m_render_pass);
            m_device_handle->m_device->destroyPipeline(*m_pipeline);

        }


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
			*m_render_pass = m_device_handle->m_device->createRenderPass(render_pass_info);

		}	

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
			vk::CommandBufferAllocateInfo allocate_info(*m_device_handle->m_graphics_command_pool, vk::CommandBufferLevel::ePrimary, m_max_frames_in_flight);
			*m_cmd_buffer = m_device_handle->m_device->allocateCommandBuffers(allocate_info);

		}

        void Renderer::createShaderInputLayout() {

            uint32_t ubo_count = m_ubos.size();
            uint32_t texture_count = m_textures.size();

            vk::DescriptorSetLayoutBinding uniform_layout_binding;
            uniform_layout_binding.descriptorCount = 1;
            uniform_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
            uniform_layout_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;

            vk::DescriptorSetLayoutBinding sampler_layout_binding;
            sampler_layout_binding.descriptorCount = 1;
            sampler_layout_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            sampler_layout_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;
            sampler_layout_binding.pImmutableSamplers = nullptr;

            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            for(int i = 0; i < ubo_count; i++) {
                uniform_layout_binding.binding = i;
                bindings.push_back(uniform_layout_binding);
            }

            for(int i = 0; i < texture_count; i++) {
                sampler_layout_binding.binding = i + ubo_count;
                bindings.push_back(sampler_layout_binding);
            }

            vk::DescriptorSetLayoutCreateInfo layout_info({}, bindings);
            *m_shader_layout = m_device_handle->m_device->createDescriptorSetLayout(layout_info);

        }

        void Renderer::createShaderInputDescriptorPool() {

            vk::DescriptorPoolSize ubo_pool_size(vk::DescriptorType::eUniformBuffer, m_max_frames_in_flight * m_ubos.size());
            vk::DescriptorPoolSize tex_pool_size(vk::DescriptorType::eCombinedImageSampler, m_max_frames_in_flight * m_textures.size());

            std::vector<vk::DescriptorPoolSize> pool_sizes;

            if(m_ubos.size())
                pool_sizes.push_back(ubo_pool_size);

            if(m_textures.size())
                pool_sizes.push_back(tex_pool_size);

            if(pool_sizes.size()) {
                vk::DescriptorPoolCreateInfo info({}, m_max_frames_in_flight, pool_sizes, nullptr);
                *m_shader_input_descriptor_pool = m_device_handle->m_device->createDescriptorPool(info);
            }

        }

        void Renderer::createShaderInputDescriptors() {

            if(!(m_ubos.size() || m_textures.size()))
                return;

            std::vector<vk::DescriptorSetLayout> layouts(m_max_frames_in_flight, *m_shader_layout);
            vk::DescriptorSetAllocateInfo info(*m_shader_input_descriptor_pool, layouts);
            info.setDescriptorSetCount(m_max_frames_in_flight);

            // allocate descriptor sets (destroyed when the descriptor pool is destroyed)
            *m_shader_descriptors = m_device_handle->m_device->allocateDescriptorSets(info);

        }

		//////////////////////////////// pipeline settings /////////////////////////////////////

        void Renderer::setMaxFramesInFlight(uint32_t num) {

            m_max_frames_in_flight = num;

            for(std::vector<bool> v : m_ubos_updated_for_frame)
                v.resize(num);

            for(std::vector<bool> v : m_text_updated_for_frame)
                v.resize(num);

        }

        void Renderer::setCurrentFrameID(uint32_t frame) {

            m_current_frame = frame;
        }


		void Renderer::setVertexBufferLayout(const VertexBuffer& vbo_prototype) {

            m_vertex_bindings->push_back(*vbo_prototype.m_per_vertex_input);

            if(vbo_prototype.usesInstancing())
                m_vertex_bindings->push_back(*vbo_prototype.m_per_instance_input);

            *m_vertex_attributes = vbo_prototype.getAttributeDescriptions();

		}

        void Renderer::setShaderInput(uint32_t ubo_count, uint32_t tex_count) {

            m_ubos.resize(ubo_count);
            m_textures.resize(tex_count);

            m_ubos_updated_for_frame.resize(ubo_count, std::vector<bool>(m_max_frames_in_flight));
            m_text_updated_for_frame.resize(tex_count, std::vector<bool>(m_max_frames_in_flight));
        }

		void Renderer::setShader(Shader* shader) {
			
			m_shader_handle = shader;
		}

		void Renderer::setRenderTarget(SwapChain* swap_chain) {
			
			m_swap_chain_handle = swap_chain;
		}

        void Renderer::updateRenderTarget(SwapChain* swap_chain) {

            if(m_swap_chain_handle != swap_chain)
                UND_WARNING << "submitted a new swap chain to renderer: may cause undefined behaviour (not recommended)\n";

            m_device_handle->m_device->waitIdle();

            destroyDynamicPipelineObjects();
            initDynamicPipelineObjects();

        }

        //////////////////////////// functions for creating settings objects ////////////////////////////

        vk::PipelineVertexInputStateCreateInfo Renderer::getVertexInputState() const {

            vk::PipelineVertexInputStateCreateInfo vertex_input;
            vertex_input.setVertexBindingDescriptions(*m_vertex_bindings);
            vertex_input.setVertexAttributeDescriptions(*m_vertex_attributes);

            return vertex_input;
        }

        vk::Viewport Renderer::getViewport() const {

            vk::Viewport viewport;
            viewport.setX(0);
            viewport.setY(0);
            viewport.setWidth(m_swap_chain_handle->getWidth());
            viewport.setHeight(m_swap_chain_handle->getHeight());
            viewport.setMinDepth(0.0f);
            viewport.setMaxDepth(1.0f);

            return viewport;
        }

        vk::Rect2D Renderer::getScissor() const {
            // the part of the viewport that gets displayed

            vk::Rect2D scissor;
            scissor.setOffset(vk::Offset2D(0,0));
            scissor.setExtent(vk::Extent2D(m_swap_chain_handle->getWidth(), m_swap_chain_handle->getHeight()));

            return scissor;
        }

        vk::PipelineLayoutCreateInfo Renderer::getShaderInputLayout() const {

            vk::PipelineLayoutCreateInfo pipeline_layout;

            pipeline_layout.pSetLayouts = m_shader_layout;
            pipeline_layout.setLayoutCount = 1;

            return pipeline_layout;
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

            if(!m_ubos_updated_for_frame.at(index).at(m_current_frame)) {

                ubo->writeDescriptorSets(m_shader_descriptors, index, m_current_frame);
                m_ubos_updated_for_frame.at(index).at(m_current_frame) = true;
            }

            ubo->updateBuffer(m_current_frame);

        }

        void Renderer::submit(const Texture* tex, uint32_t index) {

            index -= m_ubos.size();

            if(m_textures.size() <= index) {
                UND_ERROR << "failed to submit texture: the index was is to big for this renderer\n";
                return;
            }

            if(m_textures.at(index) != tex) {

                std::fill(m_text_updated_for_frame.at(index).begin(), m_text_updated_for_frame.at(index).end(), false);
                m_textures.at(index) = tex;
            }

            if(!m_text_updated_for_frame.at(index).at(m_current_frame)) {

                tex->writeDescriptorSets(m_shader_descriptors, index + m_ubos.size(), m_current_frame);
                m_text_updated_for_frame.at(index).at(m_current_frame) = true;
            }

        }

		void Renderer::draw() {

            if(!m_vbo) {
                UND_ERROR << "failed to draw: no vbo submitted\n";
                return;
            }

            // getting the objects that belong to this frame
            vk::CommandBuffer* cmd = &m_cmd_buffer->at(m_current_frame);
            vk::Semaphore* image_ready = &m_swap_chain_handle->m_image_available->at(m_current_frame);
            vk::Semaphore* render_finished = &m_swap_chain_handle->m_render_finished->at(m_current_frame);
            vk::Fence* render_finished_fence = &m_swap_chain_handle->m_frame_in_flight->at(m_current_frame);

            // the semaphores to wait on before drawing to the render target
            std::vector<vk::Semaphore> wait_on({*image_ready});

            // record the command buffer
            recordCommandBuffer(cmd, m_vbo);

			// submit the command buffer
			submitCommandBuffer(cmd, &wait_on, render_finished, render_finished_fence);
		}

		/////////////////////////////////////// private draw functions ////////////////////////////

        void Renderer::bindVertexBuffer(vk::CommandBuffer* cmd, const VertexBuffer* vbo) {

            // binding the per vertex data
            cmd->bindVertexBuffers(0, *vbo->m_vertex_data.m_buffer, {0});

            // binding the per instance data
            if(vbo->usesInstancing())
                cmd->bindVertexBuffers(1, *vbo->m_instance_data.m_buffer, {0});
        }

        void Renderer::bindDescriptorSets(vk::CommandBuffer* cmd) {

            cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_layout, 0, m_shader_descriptors->at(m_current_frame), nullptr);

        }


        void Renderer::recordCommandBuffer(vk::CommandBuffer* cmd_buffer, const VertexBuffer* vbo) {

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
            cmd_buffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
            bindVertexBuffer(cmd_buffer, m_vbo);
            bindDescriptorSets(cmd_buffer);

            if(vbo->usesIndices()) {
                cmd_buffer->bindIndexBuffer(*m_vbo->m_index_data.m_buffer, 0, vk::IndexType::eUint32);
                cmd_buffer->drawIndexed(vbo->getVertexCount(), vbo->getInstanceCount(), 0, 0, 0);
            } else {
                cmd_buffer->draw(vbo->getVertexCount(), vbo->getInstanceCount(), 0, 0);
            }

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
			m_device_handle->m_graphics_queue->submit(1, &submit_info, *render_finished_fence);

		}


	} // graphics

} // undicht
