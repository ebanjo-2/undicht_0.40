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

namespace undicht {

	namespace graphics {

		class GraphicsDevice;

		class Renderer : public Pipeline {

        protected:

            const SwapChain* m_swap_chain_handle = 0;
			std::vector<vk::Framebuffer> *m_swap_frame_buffers = 0;

			// command pool
			std::vector<vk::CommandBuffer>* m_cmd_buffer = 0;

            // currently submitted objects
            const VertexBuffer* m_vbo = 0;
            std::vector<const UniformBuffer*> m_ubos;
            std::vector<const Texture*> m_textures;

            std::vector<std::vector<bool>> m_text_updated_for_frame;
            std::vector<std::vector<bool>> m_ubos_updated_for_frame; // ubo_index -> frame_index


			friend GraphicsDevice;

        public:

            Renderer(const GraphicsDevice* device);
            virtual ~Renderer();
            void cleanUp();

		public:

            void setShaderInput(uint32_t ubo_count, uint32_t tex_count);
            void setRenderTarget(SwapChain* swap_chain);

			void linkPipeline();

		private:
            // functions for creating the renderer

			void createSwapChainFrameBuffers();
			void createCommandBuffers();


        public:
			// drawing

            void submit(const VertexBuffer* vbo);
            void submit(UniformBuffer* ubo, uint32_t index);
            void submit(const Texture* tex, uint32_t index); // the texture index starts after the last ubo index

			void draw();

		private:

            void bindVertexBuffer(vk::CommandBuffer* cmd);
            void bindDescriptorSets(vk::CommandBuffer* cmd);

            void recordCommandBuffer(vk::CommandBuffer* cmd_buffer);
            void recordDrawCommands(vk::CommandBuffer* cmd_buffer);
			void submitCommandBuffer(vk::CommandBuffer* cmd_buffer, std::vector<vk::Semaphore>* wait_on, vk::Semaphore* render_finished, vk::Fence* render_finished_fence);

		};

	} // graphics

} // undicht

#endif // RENDERER_H
