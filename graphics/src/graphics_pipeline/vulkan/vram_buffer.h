#ifndef VRAM_BUFFER_H
#define VRAM_BUFFER_H

#include "core/vulkan/vulkan_declaration.h"
#include "set"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;
        class VertexBuffer;
        class Renderer;

        class VramBuffer {

        protected:

            vk::Buffer* m_buffer = 0;
            vk::DeviceMemory *m_memory = 0;
            vk::BufferUsageFlags* m_usage = 0;
            vk::MemoryPropertyFlags* m_mem_properties = 0;
            std::vector<uint32_t> m_queue_ids; // ids of the queue families that can use this buffer

            vk::CommandPool* m_transfer_cmd_pool = 0; // used to copy data between buffers

            uint32_t m_byte_size = 0;

            friend GraphicsDevice;
            friend VertexBuffer;
            friend Renderer;

            const GraphicsDevice* m_device_handle = 0;

            VramBuffer(const GraphicsDevice* device);

            virtual void cleanUp();

        public:

            virtual ~VramBuffer();

        protected:
            // specifying usage

            void setUsage(const vk::BufferUsageFlags& usage_bits, vk::MemoryPropertyFlags mem_properties, std::vector<uint32_t> queue_ids);

        protected:
            // allocating memory

            void allocate(uint32_t byte_size);
            void deallocate(); // free the memory belonging to this buffer

            void createCommandPool();
        private:

            // finds the right memory type for your needs
            uint32_t findMemory(const vk::MemoryType& type) const;

        protected:
            // storing data

            void setData(const void* data, uint32_t byte_size, uint32_t offset);
            void setData(const VramBuffer& data, uint32_t offset); // copy from buffer

            uint32_t getSize() const; // size in bytes

        };

    } // graphics

} // undicht

#endif // VRAM_BUFFER_H