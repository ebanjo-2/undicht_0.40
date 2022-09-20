#ifndef VRAM_BUFFER_H
#define VRAM_BUFFER_H

#include "core/vulkan/vulkan_declaration.h"
#include "set"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;
        class VertexBuffer;
        class Renderer;
        class UniformBuffer;
        class Texture;
        class RenderPass;

        class VramBuffer {

        protected:

            vk::Buffer* m_buffer = 0;
            vk::DeviceMemory *m_memory = 0;
            vk::BufferUsageFlags* m_usage = 0;
            vk::MemoryPropertyFlags* m_mem_properties = 0;
            std::vector<uint32_t> m_queue_ids; // ids of the queue families that can use this buffer

            uint32_t m_byte_size = 0;

            friend GraphicsDevice;
            friend VertexBuffer;
            friend Renderer;
            friend UniformBuffer;
            friend Texture;
            friend RenderPass;

            const GraphicsDevice* m_device_handle = 0;

            VramBuffer() = default;
            VramBuffer(const GraphicsDevice* device);

            virtual void cleanUp();

        public:

            virtual ~VramBuffer();

        protected:
            // specifying usage

            void setUsage(const vk::BufferUsageFlags& usage_bits, vk::MemoryPropertyFlags mem_properties, std::vector<uint32_t> queue_ids);

        protected:
            // allocating memory

            // makes sure that the buffer has at least a size of byte_size
            void reserve(uint32_t byte_size);
            void allocate(uint32_t byte_size);

        protected:
            // storing data

            void setData(const void* data, uint32_t byte_size, uint32_t offset);
            void setData(const VramBuffer& data, uint32_t byte_size, uint32_t src_offset, uint32_t dst_offset); // copy from buffer

            uint32_t getSize() const; // size in bytes

        };

    } // graphics

} // undicht

#endif // VRAM_BUFFER_H