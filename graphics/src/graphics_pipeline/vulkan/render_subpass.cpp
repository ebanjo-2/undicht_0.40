#include "render_subpass.h"

#include "vulkan/vulkan.hpp"


namespace undicht {

	namespace graphics {

		RenderSubpass::RenderSubpass() {
			
			m_description = new vk::SubpassDescription;

		}

		RenderSubpass::~RenderSubpass() {

			if(m_description)
				delete m_description;

		}

		void RenderSubpass::setAttachments(std::vector<vk::AttachmentReference>* attachments) {
			
			m_description->setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
			m_description->setColorAttachments(*attachments);

		}

	} // graphics

} // undicht
