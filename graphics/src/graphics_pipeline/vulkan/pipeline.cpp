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
            createShaderInputDescriptorPool(ubo_count, tex_count, 400);
            createShaderInputDescriptors(ubo_count, tex_count, 400);
        }

        void Pipeline::setShader(Shader* shader) {

            m_shader_handle = shader;
        }

        void Pipeline::setViewport(unsigned width, unsigned height) {

            m_view_width = width;
            m_view_height = height;

            if((*m_pipeline) != (vk::Pipeline)VK_NULL_HANDLE) {
                // reinitializing the pipeline to update on the changes
                m_device_handle->m_device->waitIdle();
                destroyDynamicPipelineObjects();
                initDynamicPipelineObjects();
            }

        }

        void Pipeline::setFramebufferLayout(const Framebuffer& fbo) {

            m_render_pass = fbo.m_render_pass;
        }

        void Pipeline::setDepthTest(bool test, bool write) {

            m_enable_depth_test = test;
            m_write_depth_values = write;
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
            vk::PipelineDepthStencilStateCreateInfo depth_stencil = getDepthStencilInfo();

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
            pipeline_info.setPDepthStencilState(&depth_stencil);
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

        void Pipeline::createShaderInputDescriptorPool(unsigned ubo_count, unsigned tex_count, unsigned num_draw_calls) {

            uint32_t max_frames_in_flight = m_device_handle->getMaxFramesInFlight();

            // determining the size of the descriptor pool
            vk::DescriptorPoolSize ubo_pool_size(vk::DescriptorType::eUniformBuffer, max_frames_in_flight * ubo_count * num_draw_calls);
            vk::DescriptorPoolSize tex_pool_size(vk::DescriptorType::eCombinedImageSampler, max_frames_in_flight * tex_count * num_draw_calls);

            std::vector<vk::DescriptorPoolSize> pool_sizes;

            if(ubo_count)
                pool_sizes.push_back(ubo_pool_size);

            if(tex_count)
                pool_sizes.push_back(tex_pool_size);

            if(pool_sizes.size()) {
                vk::DescriptorPoolCreateInfo info({}, max_frames_in_flight * num_draw_calls, pool_sizes, nullptr);
                *m_shader_input_descriptor_pool = m_device_handle->m_device->createDescriptorPool(info);
            }

        }

        void Pipeline::createShaderInputDescriptors(unsigned ubo_count, unsigned tex_count, unsigned num_draw_calls) {

            if(!(ubo_count || tex_count)) // no input
                return;

            std::vector<vk::DescriptorSetLayout> layouts(m_device_handle->getMaxFramesInFlight() * num_draw_calls, *m_shader_layout);
            vk::DescriptorSetAllocateInfo info(*m_shader_input_descriptor_pool, layouts);
            info.setDescriptorSetCount(m_device_handle->getMaxFramesInFlight() * num_draw_calls);

            // allocate descriptor sets (destroyed when the descriptor pool is destroyed)
            *m_shader_descriptors = m_device_handle->m_device->allocateDescriptorSets(info);
        }

        vk::DescriptorSet* Pipeline::getShaderInputDescriptor(unsigned frame, unsigned draw_call) const {

            return &m_shader_descriptors->at(frame + m_device_handle->getMaxFramesInFlight() * draw_call);
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

            viewport.setWidth(m_view_width);
            viewport.setHeight(m_view_height);


            return viewport;
        }

        vk::Rect2D Pipeline::getScissor() const {
            // the part of the viewport that gets displayed

            vk::Rect2D scissor;
            scissor.setOffset(vk::Offset2D(0,0));
            scissor.setExtent(vk::Extent2D(m_view_width, m_view_height));

            return scissor;
        }

        vk::PipelineLayoutCreateInfo Pipeline::getShaderInputLayout() const {

            vk::PipelineLayoutCreateInfo pipeline_layout;

            pipeline_layout.pSetLayouts = m_shader_layout;
            pipeline_layout.setLayoutCount = 1;

            return pipeline_layout;
        }

        vk::PipelineDepthStencilStateCreateInfo Pipeline::getDepthStencilInfo() const {

            vk::PipelineDepthStencilStateCreateInfo depth_stencil;

            depth_stencil.depthTestEnable = m_enable_depth_test;
            depth_stencil.depthWriteEnable = m_write_depth_values;
            depth_stencil.depthCompareOp = vk::CompareOp::eLess;
            depth_stencil.depthBoundsTestEnable = VK_FALSE;
            depth_stencil.stencilTestEnable = VK_FALSE;

            return depth_stencil;
        }

        //////////////////////////////////////// destroying the pipeline //////////////////////////////////////////

        void Pipeline::destroyStaticPipelineObjects() {

            m_device_handle->m_device->destroyDescriptorPool(*m_shader_input_descriptor_pool);
            m_device_handle->m_device->destroyDescriptorSetLayout(*m_shader_layout);

        }

        void Pipeline::destroyDynamicPipelineObjects() {

            m_device_handle->m_device->destroyPipelineLayout(*m_layout);
            m_device_handle->m_device->destroyPipeline(*m_pipeline);
        }


    } // graphics

} // undicht