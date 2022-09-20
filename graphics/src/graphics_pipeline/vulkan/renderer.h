#ifndef RENDERER_H
#define RENDERER_H

#include "core/vulkan/vulkan_declaration.h"
#include "buffer_layout.h"
#include "core/vulkan/swap_chain.h"

#include "graphics_pipeline/vulkan/shader.h"
#include "graphics_pipeline/vulkan/vertex_buffer.h"
#include "graphics_pipeline/vulkan/uniform_buffer.h"
#include "graphics_pipeline/vulkan/texture.h"
#include "graphics_pipeline/vulkan/pipeline.h"
#include "graphics_pipeline/vulkan/render_pass.h"


namespace undicht {

	namespace graphics {

		class GraphicsDevice;
        class SwapChain;

		class Renderer {

        protected:

            const GraphicsDevice* m_device_handle = 0;

            std::vector<bool> m_render_started;
            std::vector<vk::Fence>* m_render_finished = 0;

            Pipeline m_pipeline;
            RenderPass m_render_pass;

            // currently submitted objects
            Framebuffer* m_fbo;
            const VertexBuffer* m_vbo = 0;
            std::vector<const UniformBuffer*> m_ubos;
            std::vector<const Texture*> m_textures;

            std::vector<std::vector<bool>> m_text_updated_for_frame;
            std::vector<std::vector<bool>> m_ubos_updated_for_frame; // ubo_index -> frame_index

            // objects used in the current render pass
            unsigned m_current_draw_call = 0;

            friend GraphicsDevice;
            friend SwapChain;

        public:

            Renderer(const GraphicsDevice* device);
            virtual ~Renderer();
            void cleanUp();

		public:

            void setFramebufferLayout(const Framebuffer& fbo);
            void setVertexBufferLayout(const VertexBuffer& vbo_prototype);
            void setShader(Shader* shader);
            void setShaderInput(uint32_t ubo_count, uint32_t tex_count);
            void setViewport(unsigned width, unsigned height);
            void setDepthTest(bool test = true, bool write = true);

            void linkPipeline();

        public:
            // managed by the swap chain

            void beginNewFrame(uint32_t frame_id);
            bool renderStarted(uint32_t frame_id);

        public:
			// drawing

            // commands that can be executed during a render pass
            void submit(UniformBuffer* ubo, uint32_t index);
            void submit(const Texture* tex, uint32_t index); // the texture index starts after the last ubo index
			void draw(const VertexBuffer* vbo);

            void beginRenderPass(Framebuffer* fbo);
            void endRenderPass(); // the renderpass will be executed by the gpu


		};

	} // graphics

} // undicht

#endif // RENDERER_H
