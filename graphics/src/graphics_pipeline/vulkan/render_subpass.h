#ifndef RENDER_SUBPASS_H
#define RENDER_SUBPASS_H

#include "core/vulkan/vulkan_declaration.h"

#include "vector"


namespace undicht {

	namespace graphics {

		class Renderer;

		class RenderSubpass {

		private:

			vk::SubpassDescription* m_description = 0;

			friend Renderer;

			RenderSubpass();

			// (texture) attachments used by the subpass
			void setAttachments(std::vector<vk::AttachmentReference>* attachments);

		public:

			~RenderSubpass();

		};

	} // graphics

} // undicht


#endif // RENDER_SUBPASS_H
