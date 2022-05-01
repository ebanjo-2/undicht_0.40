#include "graphics_pipeline/vulkan/vertex_buffer.h"

namespace undicht {

    namespace graphics {

        VertexBuffer::VertexBuffer(const GraphicsDevice* device) : m_vertex_data(device), m_transfer_data(device) {

            m_device_handle = device;

            // vertex bindings
            m_per_vertex_input = new vk::VertexInputBindingDescription(0, 0, vk::VertexInputRate::eVertex);
            m_per_instance_input = new vk::VertexInputBindingDescription(1, 0, vk::VertexInputRate::eInstance);

            initTransferBuffer();
            initVertexDataBuffer();
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

        ///////////////////////////////////// initializing the buffers /////////////////////////////////////

        void VertexBuffer::initTransferBuffer() {

            // queue families this buffer is going to be accessed from
            std::vector<uint32_t> queue_ids;
            queue_ids.push_back(m_device_handle->m_transfer_queue_id); // obviously

            // memory properties
            vk::MemoryPropertyFlags mem_properties; // needs to be directly accessible by the cpu
            mem_properties |= vk::MemoryPropertyFlagBits::eHostCoherent;
            mem_properties |= vk::MemoryPropertyFlagBits::eHostVisible;

            // usage
            vk::BufferUsageFlags usage_flags = {};
            usage_flags |= vk::BufferUsageFlagBits::eTransferSrc;

            m_transfer_data.setUsage(usage_flags, mem_properties,  queue_ids);

        }

        void VertexBuffer::initVertexDataBuffer() {

            // queue families this buffer is going to be accessed from
            std::vector<uint32_t> queue_ids;
            queue_ids.push_back(m_device_handle->m_graphics_queue_id); // these two should be different
            queue_ids.push_back(m_device_handle->m_transfer_queue_id);

            // memory properties
            vk::MemoryPropertyFlags mem_properties; // preferably actual device memory (fastest)
            mem_properties |= vk::MemoryPropertyFlagBits::eDeviceLocal;

            // usage
            vk::BufferUsageFlags usage_flags = {};
            usage_flags |= vk::BufferUsageFlagBits::eVertexBuffer;
            usage_flags |= vk::BufferUsageFlagBits::eTransferDst; // data needs to be able to be copied to it

            m_vertex_data.setUsage(usage_flags, mem_properties,  queue_ids);

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

        void VertexBuffer::setVertexData(const std::vector<float>& data) {

            // storing the data in the transfer buffer
            m_transfer_data.deallocate();
            m_transfer_data.allocate(data.size() * sizeof(float));
            m_transfer_data.setData(data.data(), data.size() * sizeof(float), 0);

            // copying the data to the vertex buffer (on cpu invisible but faster gpu memory)
            m_vertex_data.deallocate();
            m_vertex_data.allocate(data.size() * sizeof(float));
            m_vertex_data.setData(m_transfer_data, 0);

        }

        uint32_t VertexBuffer::getVertexCount() const {

            return m_vertex_data.getSize() / m_vertex_attributes.getTotalSize();
        }

    } // graphics

} // undicht