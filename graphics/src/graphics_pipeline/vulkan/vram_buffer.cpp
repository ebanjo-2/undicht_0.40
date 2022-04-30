#include "graphics_pipeline/vulkan/vram_buffer.h"

namespace undicht {

    namespace graphics {

        VramBuffer::VramBuffer(const GraphicsDevice* device) {

            m_device_handle = device->m_device;
            m_physical_device_handle = device->m_physical_device;

            m_usage = new vk::BufferUsageFlagBits;
            m_buffer = new vk::Buffer;
            m_memory = new vk::DeviceMemory;

        }

        VramBuffer::~VramBuffer() {

            cleanUp();

            delete m_usage;
            delete m_buffer;
            delete m_memory;

        }

        void VramBuffer::cleanUp() {

            if(!m_device_handle)
                return;

            m_device_handle->destroyBuffer(*m_buffer);
            m_device_handle->freeMemory(*m_memory);

        }

        ////////////////////////////////////////// specifying usage //////////////////////////////////////////

        void VramBuffer::setUsage(const vk::BufferUsageFlagBits& usage_bits, std::vector<uint32_t> queue_ids) {

            *m_usage = usage_bits;
            m_queue_ids = queue_ids;

        }


        ////////////////////////////////////////// allocating memory /////////////////////////////////////////

        void VramBuffer::allocate(uint32_t byte_size) {

            m_byte_size = byte_size;

            // creating the buffer
            vk::SharingMode sharing_mode = (m_queue_ids.size() > 1)? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
            vk::BufferCreateInfo info({}, byte_size, *m_usage, sharing_mode, m_queue_ids);
            *m_buffer = m_device_handle->createBuffer(info);

            // getting memory requirements
            vk::MemoryRequirements requirements;
            requirements = m_device_handle->getBufferMemoryRequirements(*m_buffer);

            // specifying the required properties of the  memory type
            vk::MemoryType memory_type;
            memory_type.heapIndex = requirements.memoryTypeBits; // bitfield specifying which memory types can be used
            memory_type.propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

            // filling out the allocation info
            vk::MemoryAllocateInfo allocate_info;
            allocate_info.allocationSize = requirements.size; // comes from the buffer
            allocate_info.memoryTypeIndex = findMemory(memory_type);

            // allocating the memory
            *m_memory = m_device_handle->allocateMemory(allocate_info);

            // binding the memory to the buffer
            m_device_handle->bindBufferMemory(*m_buffer, *m_memory, 0);

        }

        void VramBuffer::deallocate() {
            // free the memory belonging to this buffer

            m_device_handle->freeMemory(*m_memory);
        }

        uint32_t VramBuffer::findMemory(const vk::MemoryType& type) const {
            // finds the right memory type for your needs

            // getting the physical devices memory properties
            vk::PhysicalDeviceMemoryProperties properties;
            properties = m_physical_device_handle->getMemoryProperties();

            // searching for the right memory
            for(int i = 0; i < properties.memoryTypeCount; i++) {

                if(!((1 << i) & type.heapIndex))
                    continue; // heapIndex is used as a bitfield which specifies the types that can be used

                if(properties.memoryTypes.at(i).propertyFlags == type.propertyFlags)
                    return i;

            }

            UND_ERROR << "failed to find the right type of vram\n";
            return 0;
        }

        ///////////////////////////////////////////////// storing data /////////////////////////////////////////////////

        void VramBuffer::setData(const void* data, uint32_t byte_size, uint32_t offset) {

            if(byte_size + offset > m_byte_size) {
                UND_ERROR << "failed to fill buffer: buffer to small\n";
                return;
            }

            // mapping the memory
            void* buffer = m_device_handle->mapMemory(*m_memory, 0, m_byte_size);

            // copying the data into the mapped buffer
            std::memcpy(buffer, data, byte_size);

            // unmapping the memory (vulkan can now transfer it to the gpu)
            m_device_handle->unmapMemory(*m_memory);

        }

    } // graphics

} // undicht
