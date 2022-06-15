#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "core/vulkan/vulkan_declaration.h"
#include "vram_buffer.h"
#include "buffer_layout.h"

#include "vector"

namespace undicht {

    namespace graphics {

        class Pipeline;
        class GraphicsDevice;
        class Renderer;

        class VertexBuffer {

        private:

            // transfer ("staging") buffer
            VramBuffer m_transfer_data;

            // per vertex data
            VramBuffer m_vertex_data;
            BufferLayout m_vertex_attributes;
            vk::VertexInputBindingDescription* m_per_vertex_input = 0;

            // index data
            VramBuffer m_index_data;

            // per instance data
            VramBuffer m_instance_data;
            BufferLayout m_instance_attributes;
            vk::VertexInputBindingDescription* m_per_instance_input = 0;

            friend GraphicsDevice;
            friend Renderer;
            friend Pipeline;

            const GraphicsDevice* m_device_handle = 0;

            VertexBuffer(const GraphicsDevice* device);

            void cleanUp();

        public:

            virtual ~VertexBuffer();

        private:
            // initializing the buffers

            void initTransferBuffer();
            void initVertexDataBuffer();
            void initIndexDataBuffer();
            void initInstanceDataBuffer();

        public:
            // specifying the vertex layout

            void setVertexAttributes(const BufferLayout& layout);
            void setVertexAttribute(uint32_t index, const FixedType& type);
            const BufferLayout& getVertexAttributes() const;

            // layout of the data that changes every instance (instead of every vertex)
            void setInstanceAttributes(const BufferLayout& layout);
            void setInstanceAttribute(uint32_t index, const FixedType& type);
            const BufferLayout& getInstanceAttributes() const;


        private:
            // private functions for specifying vertex layout

            // translating the attributes to vk::VertexInputAttributeDescriptions
            std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() const;

        public:
            // setting data

            void setVertexData(const std::vector<float>& data, uint32_t offset = 0);
            void setIndexData(const std::vector<uint32_t>& data, uint32_t offset = 0);
            void setInstanceData(const std::vector<float>& data, uint32_t offset = 0);

            void setVertexData(const void* data, uint32_t byte_size , uint32_t offset);
            void setIndexData(const void* data, uint32_t byte_size, uint32_t offset);
            void setInstanceData(const void* data, uint32_t byte_size, uint32_t offset);

            bool usesIndices() const;
            bool usesInstancing() const;
            uint32_t getVertexCount() const;
            uint32_t getInstanceCount() const;

        };

    }

} // undicht

#endif // VERTEX_BUFFER_H