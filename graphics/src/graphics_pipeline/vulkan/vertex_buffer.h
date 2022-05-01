#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "core/vulkan/vulkan_declaration.h"
#include "vram_buffer.h"
#include "buffer_layout.h"

#include "vector"

namespace undicht {

    namespace graphics {

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

            // per instance data
            vk::VertexInputBindingDescription* m_per_instance_input = 0;

            friend GraphicsDevice;
            friend Renderer;

            const GraphicsDevice* m_device_handle = 0;

            VertexBuffer(const GraphicsDevice* device);

            void cleanUp();

        public:

            virtual ~VertexBuffer();

        private:
            // initializing the buffers

            void initTransferBuffer();
            void initVertexDataBuffer();

        public:
            // specifying the vertex layout

            void setVertexAttributes(const BufferLayout& layout);
            void setVertexAttribute(uint32_t index, const FixedType& type);
            const BufferLayout& getVertexAttributes() const;

        private:
            // private functions for specifying vertex layout

            // translating the attributes to vk::VertexInputAttributeDescriptions
            std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() const;

        public:
            // setting data

            void setVertexData(const std::vector<float>& data);

            uint32_t getVertexCount() const;

        };

    }

} // undicht

#endif // VERTEX_BUFFER_H