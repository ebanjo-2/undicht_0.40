#include "graphics_types.h"
#include "debug.h"

namespace undicht {
	
	namespace graphics {

		const int UND_VERTEX_SHADER = 100;
		const int UND_FRAGMENT_SHADER = 101;

        vk::Format translateVulkanFormat(const FixedType& type) {

            if(type.m_num_components == 1) {
                // scalar types

                if(type.m_type == Type::FLOAT) {
                    // different types of float
                    if(type.m_size == 4) return vk::Format::eR32Sfloat;
                    if(type.m_size == 8) return vk::Format::eR64Sfloat;
                }

                if(type.m_type == Type::INT) {
                    // different types of int
                    if(type.m_size == 1) return vk::Format::eR8Sint;
                    if(type.m_size == 2) return vk::Format::eR16Sint;
                    if(type.m_size == 4) return vk::Format::eR32Sint;
                    if(type.m_size == 8) return vk::Format::eR64Sint;
                }
            }

            if(type.m_num_components == 2) {
                // 2D vectors

                if(type.m_type == Type::FLOAT) {
                    // different types of float
                    if(type.m_size == 4) return vk::Format::eR32G32Sfloat;
                    if(type.m_size == 8) return vk::Format::eR64G64Sfloat;
                }

                if(type.m_type == Type::INT) {
                    // different types of int
                    if(type.m_size == 1) return vk::Format::eR8G8Sint;
                    if(type.m_size == 2) return vk::Format::eR16G16Sint;
                    if(type.m_size == 4) return vk::Format::eR32G32Sint;
                    if(type.m_size == 8) return vk::Format::eR64G64Sint;
                }
            }

            if(type.m_num_components == 3) {
                // 3D vectors

                if(type.m_type == Type::FLOAT) {
                    // different types of float
                    if(type.m_size == 4) return vk::Format::eR32G32B32Sfloat;
                    if(type.m_size == 8) return vk::Format::eR64G64B64Sfloat;
                }

                if(type.m_type == Type::INT) {
                    // different types of int
                    if(type.m_size == 1) return vk::Format::eR8G8B8Sint;
                    if(type.m_size == 2) return vk::Format::eR16G16B16Sint;
                    if(type.m_size == 4) return vk::Format::eR32G32B32Sint;
                    if(type.m_size == 8) return vk::Format::eR64G64B64Sint;
                }
            }

            if(type.m_num_components == 4) {
                // 4D vectors

                if(type.m_type == Type::FLOAT) {
                    // different types of float
                    if(type.m_size == 4) return vk::Format::eR32G32B32A32Sfloat;
                    if(type.m_size == 8) return vk::Format::eR64G64B64A64Sfloat;
                }

                if(type.m_type == Type::INT) {
                    // different types of int
                    if(type.m_size == 1) return vk::Format::eR8G8B8A8Sint;
                    if(type.m_size == 2) return vk::Format::eR16G16B16A16Sint;
                    if(type.m_size == 4) return vk::Format::eR32G32B32A32Sint;
                    if(type.m_size == 8) return vk::Format::eR64G64B64A64Sint;
                }
            }

            UND_ERROR << "failed to translate format\n";
            return vk::Format::eUndefined;
        }

	} // graphics

} // undicht
