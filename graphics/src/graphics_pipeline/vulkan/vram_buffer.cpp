#include "graphics_pipeline/vulkan/vram_buffer.h"

namespace undicht {

    namespace graphics {

        VramBuffer::VramBuffer(const GraphicsDevice* device) {

            m_device_handle = device;

            m_usage = new vk::BufferUsageFlags;
            m_mem_properties = new vk::MemoryPropertyFlags;
            m_buffer = new vk::Buffer;
            m_memory = new vk::DeviceMemory;
        }

        VramBuffer::~VramBuffer() {

            if(!m_device_handle)
                return;

            cleanUp();

            delete m_usage;
            delete m_mem_properties;
            delete m_buffer;
            delete m_memory;
        }

        void VramBuffer::cleanUp() {

            m_device_handle->m_device->freeMemory(*m_memory);
            m_device_handle->m_device->destroyBuffer(*m_buffer);
        }

        ////////////////////////////////////////// specifying usage //////////////////////////////////////////

        void VramBuffer::setUsage(const vk::BufferUsageFlags& usage_bits, vk::MemoryPropertyFlags mem_properties, std::vector<uint32_t> queue_ids) {

            // properties defined by the user
            *m_usage = usage_bits;
            *m_mem_properties = mem_properties;
            m_queue_ids = queue_ids;

            // additional properties needed by this class
            *m_usage |= vk::BufferUsageFlagBits::eTransferDst; // to be able to copy data between buffers
            *m_usage |= vk::BufferUsageFlagBits::eTransferSrc; // also needed when the buffer is resized

            if(std::find(m_queue_ids.begin(), m_queue_ids.end(), m_device_handle->m_transfer_queue_id) == m_queue_ids.end())
                m_queue_ids.push_back(m_device_handle->m_transfer_queue_id); // needed to copy data between buffers

        }


        ////////////////////////////////////////// allocating memory /////////////////////////////////////////

        void VramBuffer::reserve(uint32_t byte_size) {
            // makes sure that the buffer has at least a size of byte_size

            // checking if a new memory allocation is necessary
            if(byte_size <= getSize())
                return; // nothing to be done

            // if the buffer has no allocated memory
            if(!m_byte_size) {
                allocate(byte_size);
                return;
            }

            // creating a new Buffer
            VramBuffer new_buffer(m_device_handle);
            new_buffer.setUsage(*m_usage, *m_mem_properties, m_queue_ids);
            new_buffer.allocate(byte_size);

            // copying the old data
            new_buffer.setData(*this, m_byte_size, 0, 0);

            // swapping *this and new_buffer
            // while swapping no vulkan objects should be destroyed
            // this is why the device handles are set to 0 while swapping
            // not quite as sophisticated as "the rule of three"
            // but faster
            // besides, no objects with vulkan handles should ever be copied
            // this is literary the only exception (at least for the moment)
            // edit: there is one more exception now: in the uniform buffer class, when creating a vector of VramBuffers
            // it just works (and im going to regret it at some point)
            const GraphicsDevice* tmp = m_device_handle;
            m_device_handle = 0;
            new_buffer.m_device_handle = 0;
            std::swap(*this, new_buffer);
            new_buffer.m_device_handle = tmp;
            m_device_handle = tmp;

        }

        void VramBuffer::allocate(uint32_t byte_size) {

            if(byte_size <= m_byte_size)
                return;

            m_byte_size = byte_size;

            // creating the buffer
            vk::SharingMode sharing_mode = (m_queue_ids.size() > 1)? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
            vk::BufferCreateInfo info({}, byte_size, *m_usage, sharing_mode, m_queue_ids);
            *m_buffer = m_device_handle->m_device->createBuffer(info);

            // getting memory requirements
            vk::MemoryRequirements requirements;
            requirements = m_device_handle->m_device->getBufferMemoryRequirements(*m_buffer);

            // specifying the required properties of the  memory type
            vk::MemoryType memory_type;
            memory_type.heapIndex = requirements.memoryTypeBits; // bitfield specifying which memory types can be used
            memory_type.propertyFlags = *m_mem_properties;

            // filling out the allocation info
            vk::MemoryAllocateInfo allocate_info;
            allocate_info.allocationSize = requirements.size; // comes from the buffer
            allocate_info.memoryTypeIndex = m_device_handle->findMemory(memory_type);

            // allocating the memory
            *m_memory = m_device_handle->m_device->allocateMemory(allocate_info);

            // binding the memory to the buffer
            m_device_handle->m_device->bindBufferMemory(*m_buffer, *m_memory, 0);

        }

        ///////////////////////////////////////////////// storing data /////////////////////////////////////////////////

        void VramBuffer::setData(const void* data, uint32_t byte_size, uint32_t offset) {
            // directly storing the data from the cpu

            reserve(byte_size + offset);

            // mapping the memory
            void* buffer = m_device_handle->m_device->mapMemory(*m_memory, 0, m_byte_size);

            // copying the data into the mapped buffer
            std::memcpy(buffer, data, byte_size);

            // unmapping the memory (vulkan can now transfer it to the gpu)
            m_device_handle->m_device->unmapMemory(*m_memory);

        }

        void VramBuffer::setData(const VramBuffer& data, uint32_t byte_size, uint32_t src_offset, uint32_t dst_offset) {
            // copy from buffer

            reserve(byte_size + dst_offset);

            // create a command buffer
            vk::Queue* queue = m_device_handle->m_transfer_queue;
            vk::CommandPool* cmd_pool = m_device_handle->m_transfer_command_pool;
            vk::CommandBuffer transfer_cmd = m_device_handle->beginSingleTimeCommand(*cmd_pool);

            // transfer commands
            vk::BufferCopy copy_info(src_offset, dst_offset, byte_size);
            transfer_cmd.copyBuffer(*data.m_buffer, *m_buffer, copy_info);

            // submitting the transfer command
            m_device_handle->endSingleTimeCommand(transfer_cmd, *cmd_pool, *queue);
        }

        uint32_t VramBuffer::getSize() const {
            // size in bytes
            return m_byte_size;
        }

    } // graphics

} // undicht
