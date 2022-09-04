#include "graphics_types.h"
#include "debug.h"

#include "vector"


namespace undicht {
	
	namespace graphics {

		const int UND_VERTEX_SHADER = 100;
		const int UND_FRAGMENT_SHADER = 101;

        const std::vector<std::pair<FixedType, vk::Format>> FORMAT_DICTIONARY {
                {UND_UNDEFINED_TYPE, vk::Format::eUndefined},

                // types with 1 component
                {UND_FLOAT32, vk::Format::eR32Sfloat},
                {UND_FLOAT64, vk::Format::eR64Sfloat},
                {UND_INT8, vk::Format::eR8Sint},
                {UND_INT16, vk::Format::eR16Sint},
                {UND_INT32, vk::Format::eR32Sint},
                {UND_INT64, vk::Format::eR64Sint},
                {UND_R8, vk::Format::eR8Srgb},

                // types with 2 components
                {UND_VEC2F, vk::Format::eR32G32Sfloat},
                {FixedType(Type::FLOAT, 8, 2), vk::Format::eR64G64Sfloat},
                {FixedType(Type::INT, 1, 2), vk::Format::eR8G8Sint},
                {FixedType(Type::INT, 2, 2), vk::Format::eR16G16Sint},
                {FixedType(Type::INT, 4, 2), vk::Format::eR32G32Sint},
                {FixedType(Type::INT, 8, 2), vk::Format::eR64G64Sint},
                {UND_R8G8, vk::Format::eR8G8Srgb},

                // types with 3 components
                {UND_VEC3F, vk::Format::eR32G32B32Sfloat},
                {FixedType(Type::FLOAT, 8, 3), vk::Format::eR64G64B64Sfloat},
                {FixedType(Type::INT, 1, 3), vk::Format::eR8G8B8Sint},
                {FixedType(Type::INT, 2, 3), vk::Format::eR16G16B16Sint},
                {FixedType(Type::INT, 4, 3), vk::Format::eR32G32B32Sint},
                {FixedType(Type::INT, 8, 3), vk::Format::eR64G64B64Sint},
                {UND_R8G8B8, vk::Format::eR8G8B8Srgb},
                {UND_B8G8R8, vk::Format::eB8G8R8Srgb},

                // types with 4 components
                {UND_VEC4F, vk::Format::eR32G32B32A32Sfloat},
                {FixedType(Type::FLOAT, 8, 4), vk::Format::eR64G64B64A64Sfloat},
                {FixedType(Type::INT, 1, 4), vk::Format::eR8G8B8A8Sint},
                {FixedType(Type::INT, 2, 4), vk::Format::eR16G16B16A16Sint},
                {FixedType(Type::INT, 4, 4), vk::Format::eR32G32B32A32Sint},
                {FixedType(Type::INT, 8, 4), vk::Format::eR64G64B64A64Sint},
                {UND_R8G8B8A8, vk::Format::eR8G8B8A8Srgb},
                {UND_B8G8R8A8, vk::Format::eB8G8R8A8Srgb},

                // depth buffer formats
                {UND_DEPTH32F, vk::Format::eD32Sfloat},
                {UND_DEPTH32f_STENCIL8, vk::Format::eD32SfloatS8Uint},
                {UND_DEPTH24_STENCIL8, vk::Format::eD24UnormS8Uint},

        };

        vk::Format translateVulkanFormat(const FixedType& type) {

            for(const std::pair<FixedType, vk::Format>& p : FORMAT_DICTIONARY) {

                if(p.first == type)
                    return p.second;
            }

            UND_ERROR << "failed to translate format\n";
            return vk::Format::eUndefined;
        }

        FixedType translateVulkanFormat(const vk::Format& format) {

            for(const std::pair<FixedType, vk::Format>& p : FORMAT_DICTIONARY) {

                if(p.second == format)
                    return p.first;
            }

            UND_ERROR << "failed to translate format\n";
            return UND_UNDEFINED_TYPE;
        }

	} // graphics

} // undicht
