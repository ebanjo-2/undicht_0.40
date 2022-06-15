//
// Created by eike on 14.06.22.
//

#include "pipeline.h"
#include "debug.h"
#include "core/vulkan/graphics_device.h"


namespace undicht {

    namespace graphics {

        Pipeline::Pipeline(const GraphicsDevice* device) {

            m_device_handle = device;

            m_vertex_bindings = new std::vector<vk::VertexInputBindingDescription>;
            m_vertex_attributes = new std::vector<vk::VertexInputAttributeDescription>;
            m_shader_layout = new vk::DescriptorSetLayout;
            m_shader_input_descriptor_pool = new vk::DescriptorPool;
            m_shader_descriptors = new std::vector<vk::DescriptorSet>;

            m_layout = new vk::PipelineLayout;
            m_render_pass = new vk::RenderPass;
            m_subpass_description = new vk::SubpassDescription;

            m_pipeline = new vk::Pipeline;


        }

        Pipeline::~Pipeline() {

            cleanUp();

            delete m_vertex_bindings;
            delete m_vertex_attributes;

            delete m_shader_layout;
            delete m_shader_input_descriptor_pool;
            delete m_shader_descriptors;

            delete m_layout;
            delete m_subpass_description;
            delete m_render_pass;

            delete m_pipeline;
        }

        void Pipeline::cleanUp() {

            m_device_handle->m_device->waitIdle();

            destroyStaticPipelineObjects();
            destroyDynamicPipelineObjects();
        }

        //////////////////////////////////////////////// settings ////////////////////////////////////////////

        void Pipeline::setVertexBufferLayout(const VertexBuffer& vbo_prototype) {

            m_vertex_bindings->push_back(*vbo_prototype.m_per_vertex_input);

            if(vbo_prototype.usesInstancing())
                m_vertex_bindings->push_back(*vbo_prototype.m_per_instance_input);

            *m_vertex_attributes = vbo_prototype.getAttributeDescriptions();

        }

        void Pipeline::setShaderInput(uint32_t ubo_count, uint32_t tex_count) {

            createShaderInputLayout(ubo_count, tex_count);
            createShaderInputDescriptorPool(ubo_count, tex_count);
            createShaderInputDescriptors(ubo_count, tex_count);
        }

        void Pipeline::setShader(Shader* shader) {

            m_shader_handle = shader;
        }

        void Pipeline::setRenderTarget(SwapChain* swap_chain) {

            m_swap_chain_handle = swap_chain;
        }

        void Pipeline::updateRenderTarget(SwapChain* swap_chain) {

            if(m_swap_chain_handle != swap_chain)
                UND_WARNING << "submitted a new swap chain to renderer: may cause undefined behaviour (not recommended)\n";

            m_device_handle->m_device->waitIdle();

            destroyDynamicPipelineObjects();
            initDynamicPipelineObjects();

        }

        //////////////////////////////////////// initializing the pipeline (public) ///////////////////////////////////

        void Pipeline::linkPipeline() {

            if(!checkLinkDependencies(true))
                return;

            initStaticPipelineObjects();
            initDynamicPipelineObjects();

        }

        bool Pipeline::checkLinkDependencies(bool debug_output) {

            if(!m_shader_handle) {
                if(debug_output) UND_ERROR << "failed to create graphics pipeline: no shader was submitted\n";
                return false;
            }

            return true;
        }

        //////////////////////////////////////// initializing the pipeline (private) //////////////////////////////////

        void Pipeline::initStaticPipelineObjects() {

            // shader layout objects are initialized by setShader()

        }

        void Pipeline::initDynamicPipelineObjects() {

            createRenderPass();

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

        void Pipeline::createShaderInputLayout(unsigned ubo_count, unsigned tex_count) {

            // describes a ubo binding
            vk::DescriptorSetLayoutBinding uniform_layout_binding;
            uniform_layout_binding.descriptorCount = 1;
            uniform_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
            uniform_layout_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;

            // describes a sampler binding
            vk::DescriptorSetLayoutBinding sampler_layout_binding;
            sampler_layout_binding.descriptorCount = 1;
            sampler_layout_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            sampler_layout_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;
            sampler_layout_binding.pImmutableSamplers = nullptr;

            // list of all bindings used in the shader
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            for(int i = 0; i < ubo_count; i++) {
                uniform_layout_binding.binding = i;
                bindings.push_back(uniform_layout_binding);
            }

            for(int i = 0; i < tex_count; i++) {
                sampler_layout_binding.binding = i + ubo_count;
                bindings.push_back(sampler_layout_binding);
            }

            // the shader layout combines all bindings
            vk::DescriptorSetLayoutCreateInfo layout_info({}, bindings);
            *m_shader_layout = m_device_handle->m_device->createDescriptorSetLayout(layout_info);

        }

        void Pipeline::createShaderInputDescriptorPool(unsigned ubo_count, unsigned tex_count) {

            uint32_t max_frames_in_flight = m_device_handle->getMaxFramesInFlight();

            // determining the size of the descriptor pool
            vk::DescriptorPoolSize ubo_pool_size(vk::DescriptorType::eUniformBuffer, max_frames_in_flight * ubo_count);
            vk::DescriptorPoolSize tex_pool_size(vk::DescriptorType::eCombinedImageSampler, max_frames_in_flight * tex_count);

            std::vector<vk::DescriptorPoolSize> pool_sizes;

            if(ubo_count)
                pool_sizes.push_back(ubo_pool_size);

            if(tex_count)
                pool_sizes.push_back(tex_pool_size);

            if(pool_sizes.size()) {
                vk::DescriptorPoolCreateInfo info({}, max_frames_in_flight, pool_sizes, nullptr);
                *m_shader_input_descriptor_pool = m_device_handle->m_device->createDescriptorPool(info);
            }

        }

        void Pipeline::createShaderInputDescriptors(unsigned ubo_count, unsigned tex_count) {

            if(!(ubo_count || tex_count)) // no input
                return;

            std::vector<vk::DescriptorSetLayout> layouts(m_device_handle->getMaxFramesInFlight(), *m_shader_layout);
            vk::DescriptorSetAllocateInfo info(*m_shader_input_descriptor_pool, layouts);
            info.setDescriptorSetCount(m_device_handle->getMaxFramesInFlight());

            // allocate descriptor sets (destroyed when the descriptor pool is destroyed)
            *m_shader_descriptors = m_device_handle->m_device->allocateDescriptorSets(info);
        }

        std::vector<vk::Format> Pipeline::getAttFormats(const SwapChain* swap_chain) const {
            // get formats used by the swap chains attachments (images, depth buffer, ...)

            if(!swap_chain) {
                UND_ERROR << "Failed to determine attachment formats: no swap chain submitted\n";
                return {};
            }

            // to do: determine the attachments used by the swap chain
            // is there a depth buffer ?

            return {swap_chain->m_format->format};
        }

        std::vector<vk::AttachmentDescription> Pipeline::createAttachmentDescriptions(const std::vector<vk::Format>& att_formats) const{
            // create descriptions of the textures that are going to be drawn to

            std::vector<vk::AttachmentDescription> attachments;

            for(int i = 0; i < att_formats.size(); i++) {

                vk::AttachmentDescription attachment({}, att_formats[i], vk::SampleCountFlagBits::e1);
                attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
                attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
                attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
                attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
                attachment.setInitialLayout(vk::ImageLayout::eUndefined);
                attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

                attachments.push_back(attachment);
            }

            return attachments;
        }

        std::vector<vk::AttachmentReference> Pipeline::createAttachmentReferences(const std::vector<vk::AttachmentDescription>& attachments) const{
            // create references for the attachments that describe the attachments layout

            std::vector<vk::AttachmentReference> refs;

            for(const vk::AttachmentDescription& description : attachments) {
                // assuming that all attachments are color attachments for now

                vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
                refs.push_back(color_ref);
            }

            return refs;
        }

        void Pipeline::createRenderPass() {

            // declaring which images are used during the rendering
            std::vector<vk::Format> att_formats = getAttFormats(m_swap_chain_handle);
            std::vector<vk::AttachmentDescription> attachments = createAttachmentDescriptions(att_formats);
            std::vector<vk::AttachmentReference> attachment_refs = createAttachmentReferences(attachments);

            // creating subpasses
            m_subpass_description->setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
            m_subpass_description->setColorAttachments(attachment_refs);
            std::vector<vk::SubpassDescription> subpasses({*m_subpass_description});

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

        /////////////////////////////////// getting pipeline setting objects //////////////////////////////////////

        vk::PipelineVertexInputStateCreateInfo Pipeline::getVertexInputState() const {

            vk::PipelineVertexInputStateCreateInfo vertex_input;
            vertex_input.setVertexBindingDescriptions(*m_vertex_bindings);
            vertex_input.setVertexAttributeDescriptions(*m_vertex_attributes);

            return vertex_input;
        }

        vk::Viewport Pipeline::getViewport() const {

            vk::Viewport viewport;
            viewport.setX(0);
            viewport.setY(0);
            viewport.setMinDepth(0.0f);
            viewport.setMaxDepth(1.0f);

            if(m_swap_chain_handle) {
                viewport.setWidth(m_swap_chain_handle->getWidth());
                viewport.setHeight(m_swap_chain_handle->getHeight());
            } else {
                UND_WARNING << "no swap chain submitted\n";
            }

            return viewport;
        }

        vk::Rect2D Pipeline::getScissor() const {
            // the part of the viewport that gets displayed

            vk::Rect2D scissor;
            scissor.setOffset(vk::Offset2D(0,0));

            if(m_swap_chain_handle) {
                scissor.setExtent(vk::Extent2D(m_swap_chain_handle->getWidth(), m_swap_chain_handle->getHeight()));
            } else {
                UND_WARNING << "no swap chain submitted\n";
            }

            return scissor;
        }

        vk::PipelineLayoutCreateInfo Pipeline::getShaderInputLayout() const {

            vk::PipelineLayoutCreateInfo pipeline_layout;

            pipeline_layout.pSetLayouts = m_shader_layout;
            pipeline_layout.setLayoutCount = 1;

            return pipeline_layout;
        }

        //////////////////////////////////////// destroying the pipeline //////////////////////////////////////////

        void Pipeline::destroyStaticPipelineObjects() {

            m_device_handle->m_device->destroyDescriptorPool(*m_shader_input_descriptor_pool);
            m_device_handle->m_device->destroyDescriptorSetLayout(*m_shader_layout);

        }

        void Pipeline::destroyDynamicPipelineObjects() {

            m_device_handle->m_device->destroyPipelineLayout(*m_layout);
            m_device_handle->m_device->destroyRenderPass(*m_render_pass);
            m_device_handle->m_device->destroyPipeline(*m_pipeline);
        }


    } // graphics

} // undicht