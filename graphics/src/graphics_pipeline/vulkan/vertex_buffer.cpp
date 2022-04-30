#include "graphics_pipeline/vulkan/vertex_buffer.h"

namespace undicht {

    namespace graphics {

        VertexBuffer::VertexBuffer(const GraphicsDevice* device) : VramBuffer(device) {

            m_per_vertex_input = new vk::VertexInputBindingDescription(0, 0, vk::VertexInputRate::eVertex);
            m_per_instance_input = new vk::VertexInputBindingDescription(1, 0, vk::VertexInputRate::eInstance);

            // the vertex buffer is only going to be used by the graphics queue
            setUsage(vk::BufferUsageFlagBits::eVertexBuffer, {device->m_queue_family_ids.graphics_queue});
        }

        VertexBuffer::~VertexBuffer() {

            cleanUp();

            delete m_per_vertex_input;
            delete m_per_instance_input;
        }

        void VertexBuffer::cleanUp() {

            if(!m_device_handle)
                return;

        }

        ////////////////////////////////////// specifying the vertex layout ///////////////////////////////////////

        void VertexBuffer::setVertexAttributes(const BufferLayout& layout) {

            m_vertex_attributes = layout;
            m_per_vertex_input->setStride(m_vertex_attributes.getTotalSize());
        }

        void VertexBuffer::setVertexAttribute(uint32_t index, const FixedType& type) {

            m_vertex_attributes.setType(index, type);
            m_per_vertex_input->setStride(m_vertex_attributes.getTotalSize());

        }

        const BufferLayout& VertexBuffer::getVertexAttributes() const {

            return m_vertex_attributes;
        }

        std::vector<vk::VertexInputAttributeDescription> VertexBuffer::getAttributeDescriptions() const {
            // translating the attributes to vk::VertexInputAttributeDescriptions

            std::vector<vk::VertexInputAttributeDescription> descriptions;

            // offset (in bytes) into a vertex for each attribute
            uint32_t offset = 0;

            for(int i = 0; i < m_vertex_attributes.m_types.size(); i++) {

                const FixedType& t = m_vertex_attributes.getType(i);
                descriptions.emplace_back(vk::VertexInputAttributeDescription());
                descriptions.back().binding = 0;
                descriptions.back().location = i;
                descriptions.back().format = translateVulkanFormat(t);
                descriptions.back().offset = offset;

                offset += t.getSize();
            }

            return descriptions;
        }

        ////////////////////////////////////////// setting data ////////////////////////////////////////

        void VertexBuffer::setData(const std::vector<float>& data) {

            // deallocating old memory
            deallocate();

            // allocating new memory
            allocate(data.size() * sizeof(float));

            // storing the data
            VramBuffer::setData(data.data(), data.size() * sizeof(float), 0);

        }

        uint32_t VertexBuffer::getVertexCount() const {

            return m_byte_size / m_vertex_attributes.getTotalSize();
        }

    } // graphics

} // undicht