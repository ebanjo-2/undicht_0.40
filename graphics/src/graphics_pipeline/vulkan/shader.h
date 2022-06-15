#ifndef SHADER_H
#define SHADER_H

#include "core/vulkan/vulkan_declaration.h"

#include "vector"
#include "string"


namespace undicht {

	namespace graphics {

		class GraphicsDevice;
		class Renderer;
        class Pipeline;

		class Shader {

		private:

			// binary shader sources
			std::vector<char> m_vert_shader_bin;
			std::vector<char> m_frag_shader_bin;

			// shader modules
			vk::ShaderModule* m_vert_shader = 0;
			vk::ShaderModule* m_frag_shader = 0;

			// shader stages
			std::vector<vk::PipelineShaderStageCreateInfo>* m_stages = 0;

            const GraphicsDevice* m_device_handle = 0;

			friend GraphicsDevice;
			friend Renderer;
            friend Pipeline;


		public:

            Shader(const GraphicsDevice* device);
            ~Shader();
            void cleanUp();

            // loading source code

			void loadBinaryFile(const std::string& file_name, int stage);
			void loadBinarySource(const std::vector<char>& bytes, int stage);
			void linkStages();


		};

	} // graphics

} // undicht

#endif // SHADER_H
