#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H

#include "types.h"
#include "core/vulkan/vulkan_declaration.h"

namespace undicht {

	namespace graphics {

		const extern int UND_VERTEX_SHADER;
		const extern int UND_FRAGMENT_SHADER;

        vk::Format translateVulkanFormat(const FixedType& t);
        FixedType translateVulkanFormat(const vk::Format& format);

	} // graphics

} // undicht

#endif // GRAPHICS_TYPES_H
