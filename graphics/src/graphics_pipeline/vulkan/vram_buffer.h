#ifndef VRAM_BUFFER_H
#define VRAM_BUFFER_H

#include "core/vulkan/vulkan_declaration.h"
#include "set"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;

        class VramBuffer {

        protected:

            vk::Buffer* m_buffer = 0;
            vk::DeviceMemory *m_memory = 0;
            vk::BufferUsageFlagBits* m_usage = 0;
            std::vector<uint32_t> m_queue_ids; // ids of the queue families that can use this buffer

            uint32_t m_byte_size = 0;

            friend GraphicsDevice;

            vk::Device* m_device_handle = 0;
            vk::PhysicalDevice* m_physical_device_handle = 0;

            VramBuffer() = default;
            VramBuffer(const GraphicsDevice* device);

            virtual void cleanUp();

        public:

            virtual ~VramBuffer();

        protected:
            // specifying usage

            void setUsage(const vk::BufferUsageFlagBits& usage_bits, std::vector<uint32_t> queue_ids);

        protected:
            // allocating memory

            void allocate(uint32_t byte_size);
            void deallocate(); // free the memory belonging to this buffer

        private:

            // finds the right memory type for your needs
            uint32_t findMemory(const vk::MemoryType& type) const;

        protected:
            // storing data

            void setData(const void* data, uint32_t byte_size, uint32_t offset);

        };

    } // graphics

} // undicht

#endif // VRAM_BUFFER_H