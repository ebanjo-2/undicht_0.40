//
// Created by eike on 14.06.22.
//

#ifndef UNDICHT_PIPELINE_H
#define UNDICHT_PIPELINE_H

#include "core/vulkan/vulkan_declaration.h"
#include "buffer_layout.h"
#include "shader.h"
#include "vertex_buffer.h"
#include "core/vulkan/swap_chain.h"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;
        class DrawCall;

        class Pipeline {

            friend GraphicsDevice;
            friend DrawCall;

        protected:
            // pipeline settings

            unsigned m_view_width = 0; // viewport size
            unsigned m_view_height = 0;

            std::vector<FixedType> m_attachment_formats;

        protected:
            // general info about the pipeline (vulkan objects)

            // describes buffers that can be used for vertex input
            std::vector<vk::VertexInputBindingDescription>* m_vertex_bindings = 0;
            // describes the structure of a vertex (includes data that is changed per vertex and per instance)
            std::vector<vk::VertexInputAttributeDescription>* m_vertex_attributes = 0;

            // describes the bindings for uniform buffers (which ids are used for what)
            vk::DescriptorSetLayout* m_shader_layout = 0;
            // the pool from which the shader descriptor sets are allocated
            vk::DescriptorPool* m_shader_input_descriptor_pool = 0;
            // specifies what descriptors (such as uniform buffers or samplers) are bound to shaders
            std::vector<vk::DescriptorSet>* m_shader_descriptors = 0;

            vk::RenderPass* m_render_pass = 0;

            // the object in which the pipeline layout is combined
            vk::PipelineLayout* m_layout = 0;

        protected:
            // handles to objects the pipeline depends on

            const GraphicsDevice* m_device_handle = 0;
            const Shader* m_shader_handle = 0;

        protected:

            vk::Pipeline* m_pipeline = 0;

        protected:

            Pipeline(const GraphicsDevice* device);
            virtual ~Pipeline();
            void cleanUp();

        public:
            // settings

            virtual void setVertexBufferLayout(const VertexBuffer& vbo_prototype);
            virtual void setShaderInput(uint32_t ubo_count, uint32_t tex_count);
            virtual void setShader(Shader* shader);
            virtual void setViewport(unsigned width, unsigned height);
            virtual void setFramebufferLayout(const Framebuffer& fbo); // dont destroy the fbo before the pipeline

        public:
            // initializing the pipeline

            virtual void linkPipeline();
            bool checkLinkDependencies(bool debug_output = false);

        protected:
            // initializing the pipeline

            void initStaticPipelineObjects();
            void initDynamicPipelineObjects(); // objects that may change after the pipeline was linked

            void createShaderInputLayout(unsigned ubo_count, unsigned tex_count);
            void createShaderInputDescriptorPool(unsigned ubo_count, unsigned tex_count);
            void createShaderInputDescriptors(unsigned ubo_count, unsigned tex_count);

        protected:
            // getting pipeline setting objects

            vk::PipelineVertexInputStateCreateInfo getVertexInputState() const;
            vk::Viewport getViewport() const;
            vk::Rect2D getScissor() const; // the part of the viewport that gets displayed
            vk::PipelineLayoutCreateInfo getShaderInputLayout() const;

        protected:
            // destroying the pipeline

            void destroyStaticPipelineObjects();
            void destroyDynamicPipelineObjects();


        };

    }
}




#endif //UNDICHT_PIPELINE_H
