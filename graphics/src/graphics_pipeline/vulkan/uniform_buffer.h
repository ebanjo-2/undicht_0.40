#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "vram_buffer.h"
#include "buffer_layout.h"
#include "core/vulkan/vulkan_declaration.h"
#include "vector"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;
        class Renderer;

        class UniformBuffer {

        private:

            std::vector<VramBuffer> m_buffers;
            BufferLayout m_buffer_layout;
            std::vector<char> m_tmp_buffer; // temporarily store the data
            std::vector<bool> m_buffers_updated; // true if he buffer of a frame has been updated with the tmp data
            std::vector<uint32_t> m_offsets; // offsets into the buffer for correct alignment

            friend Renderer;
            friend GraphicsDevice;
            const GraphicsDevice* m_device_handle = 0;

            UniformBuffer(const GraphicsDevice* device);

            void cleanUp();

        public:

            ~UniformBuffer();

        private:
            // init the buffer

            void initBuffers(uint32_t count);
            void initMemoryOffsets();

            void writeDescriptorSets(const std::vector<vk::DescriptorSet>* shader_descriptors, uint32_t index, uint32_t frame_id) const;
            void updateBuffer(uint32_t current_frame);

        public:
            // specifying the memory layout (should not be changed once the buffer has been submitted to a renderer)

            void setAttribute(uint32_t index, const FixedType& type);
            void setAttributes(const BufferLayout& layout);

            // call this after the layout + max frames in flight have been set
            // and before storing any data in the buffer
            void finalizeLayout();

        public:
            // setting data

            void setData(uint32_t index, const void* data, uint32_t byte_size);

        };

    } // graphics

} // undicht

#endif // UNIFORM_BUFFER_H