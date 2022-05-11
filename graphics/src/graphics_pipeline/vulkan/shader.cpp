#include "shader.h"

#include "graphics_types.h"
#include "debug.h"

#include "vulkan/vulkan.hpp"

#include "fstream"


namespace undicht {

	namespace graphics {

		
		Shader::Shader(const GraphicsDevice* device) {

			m_device_handle = device;

			m_vert_shader = new vk::ShaderModule;
			m_frag_shader = new vk::ShaderModule;

			m_stages = new std::vector<vk::PipelineShaderStageCreateInfo>;

		}
	
		Shader::~Shader() {

            cleanUp();

	        delete m_vert_shader;
            delete m_frag_shader;
            delete m_stages;
		}

        void Shader::cleanUp() {

            m_device_handle->m_device->destroyShaderModule(*m_frag_shader);
            m_device_handle->m_device->destroyShaderModule(*m_vert_shader);

        }

        //////////////////////////////////////////// loading source code //////////////////////////////////////////

		void Shader::loadBinaryFile(const std::string& file_name, int stage) {

			// opening the file
			std::ifstream file(file_name, std::ios::ate | std::ios::binary);
			uint32_t file_size = file.tellg(); // file was opened at the end
			file.seekg(0, file.beg);

			if(!file.is_open()) {
				UND_ERROR << "failed to read shader binary: " << file_name << "\n";
				return;
			}

			// loading the contents of the file
			std::vector<char> bytes(file_size);
			file.read(bytes.data(), file_size);

			// closing the file
			file.close();
			
			// passing the binary data	
			loadBinarySource(bytes, stage);

		}	


		void Shader::loadBinarySource(const std::vector<char>& bytes, int stage) {
		
			if(stage == UND_VERTEX_SHADER)
				m_vert_shader_bin = bytes;

			if(stage == UND_FRAGMENT_SHADER)
				m_frag_shader_bin = bytes;
			
		}

		void Shader::linkStages() {

			bool add_vert_shader = m_vert_shader_bin.size();
			bool add_frag_shader = m_frag_shader_bin.size();

			// every shader pipeline needs a vertex shader
			if(!add_vert_shader) {
				UND_ERROR << "failed to link shader stages: need vertex stage\n";
				return;
			}

			vk::ShaderModuleCreateInfo vert_info({}, m_vert_shader_bin.size(), (uint32_t*) m_vert_shader_bin.data());
			vk::ShaderModuleCreateInfo frag_info({}, m_frag_shader_bin.size(), (uint32_t*) m_frag_shader_bin.data());

			*m_vert_shader = m_device_handle->m_device->createShaderModule(vert_info);
			*m_frag_shader = m_device_handle->m_device->createShaderModule(frag_info);

			vk::PipelineShaderStageCreateInfo vert_stage({}, vk::ShaderStageFlagBits::eVertex, *m_vert_shader, "main");
			vk::PipelineShaderStageCreateInfo frag_stage({}, vk::ShaderStageFlagBits::eFragment, *m_frag_shader, "main");

			// storing the stage infos
			if(add_vert_shader)
				m_stages->push_back(vert_stage);

			if(add_frag_shader)
				m_stages->push_back(frag_stage);

		}


	} // graphics

} // undicht
