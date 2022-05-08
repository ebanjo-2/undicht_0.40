#include "uniform_buffer.h"

namespace undicht {

    namespace graphics {

        UniformBuffer::UniformBuffer(const GraphicsDevice* device, const std::vector<vk::DescriptorSet>* shader_descriptors, uint32_t index) {

            m_device_handle = device;
            m_descriptor_sets = shader_descriptors;

            m_shader_binding = index;

        }

        UniformBuffer::~UniformBuffer() {

            cleanUp();
        }

        void UniformBuffer::cleanUp() {

        }

        //////////////////////////////////////// init the buffer /////////////////////////////////////

        void UniformBuffer::initDescriptorSets(uint32_t count) {
            /// @param count max number of frames in flight

            if(!m_descriptor_sets) {
                UND_ERROR << "The Uniform was created for a renderer that doesnt expect one\n";
                return;
            }

            // tell the descriptor sets about the buffers
            vk::DescriptorBufferInfo buffer_info;
            buffer_info.offset = 0;
            buffer_info.range = m_tmp_buffer.size();

            vk::WriteDescriptorSet descriptor_write;
            descriptor_write.dstBinding = m_shader_binding;
            descriptor_write.pBufferInfo = &buffer_info;
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pImageInfo = nullptr;
            descriptor_write.pTexelBufferView = nullptr;

            for(int i = 0; i < count; i++) {

                buffer_info.buffer = *m_buffers.at(i).m_buffer;
                descriptor_write.dstSet = m_descriptor_sets->at(i);

                m_device_handle->m_device->updateDescriptorSets(descriptor_write, nullptr);

            }

        }

        void UniformBuffer::initBuffers(uint32_t count) {

            m_buffers.clear();

            // queue families this buffer is going to be accessed from
            std::vector<uint32_t> queue_ids;
            queue_ids.push_back(m_device_handle->m_graphics_queue_id);

            // memory properties
            vk::MemoryPropertyFlags mem_properties; // needs to be directly accessible by the cpu
            mem_properties |= vk::MemoryPropertyFlagBits::eHostCoherent;
            mem_properties |= vk::MemoryPropertyFlagBits::eHostVisible;

            // usage
            vk::BufferUsageFlags usage_flags = {};
            usage_flags |= vk::BufferUsageFlagBits::eUniformBuffer;

            // initialize the buffers
            for(int i = 0; i < count; i++) {
                VramBuffer new_buffer(m_device_handle);
                new_buffer.setUsage(usage_flags, mem_properties, queue_ids);
                new_buffer.setData(0, 0, m_tmp_buffer.size());
                new_buffer.m_device_handle = 0; // dont delete the vulkan objects for now
                m_buffers.push_back(new_buffer);
            }

            // giving them their device handle back
            for(int i = 0; i < count; i++)
                m_buffers.at(i).m_device_handle = m_device_handle;

        }

        void UniformBuffer::initMemoryOffsets() {
            // there are special needs for the alignment of types
            // for uniform buffers

            m_offsets.clear();

            uint32_t offset = 0;
            uint32_t last_size = 0;

            for(int i = 0; i < m_buffer_layout.m_types.size(); i++) {
                // moving past the last type
                offset += last_size;

                // calculating this correct types offset
                uint32_t current_size = m_buffer_layout.getType(i).getSize();
                uint32_t alignment = current_size;
                alignment = std::max(alignment, 4u); // nothing smaller than 4 bytes
                alignment = std::min(alignment, 16u); // types bigger than 16 bytes still have to be aligned as if they were 16 bytes

                if(offset % alignment)
                    offset += alignment - (offset % alignment);

                //UND_LOG << i << " offset: " << offset << "\n";
                m_offsets.push_back(offset);

                last_size = current_size;
            }

            // calculating the size of the tmp buffer
            m_tmp_buffer.resize(offset + m_buffer_layout.m_types.back().getSize());

        }

        //////////////////////////////////// using more than one frame in flight //////////////////////////////////

        void UniformBuffer::setMaxFramesInFlight(uint32_t count) {
            // the uniform data might need to be updated for every frame in flight
            // which shouldn't be done while the data is being used in rendering
            // so there is an internal buffer for each frame

            m_max_frames_in_flight = count;

        }

        void UniformBuffer::setCurrentFrame(uint32_t current_frame) {

            // updating the buffer belonging to the current frame
            if(!m_buffers_updated.at(current_frame)) {

                m_buffers.at(current_frame).setData(m_tmp_buffer.data(), m_tmp_buffer.size(), 0);
                m_buffers_updated.at(current_frame) = true;
            }

        }

        /////////////////////////////////// specifying memory layout ////////////////////////////////////////
        /// (should not be changed once the buffer has been submitted to a renderer)

        void UniformBuffer::setAttribute(uint32_t index, const FixedType& type) {

            m_buffer_layout.setType(index, type);

            initMemoryOffsets();
        }

        void UniformBuffer::setAttributes(const BufferLayout& layout) {

            m_buffer_layout = layout;

            initMemoryOffsets();
        }

        void UniformBuffer::finalizeLayout () {
            // call this after the layout + max frames in flight have been set
            // and before storing any data in the buffer

            initBuffers(m_max_frames_in_flight);
            initDescriptorSets(m_max_frames_in_flight);
            m_buffers_updated.resize(m_max_frames_in_flight);
        }

        /////////////////////////////////////// setting data ///////////////////////////////////////

        void UniformBuffer::setData(uint32_t index, const void* data, uint32_t byte_size) {
            // storing the data in the tmp buffer

            std::copy((const char*)data, (const char*)data + byte_size, m_tmp_buffer.begin() + m_offsets.at(index));

            std::fill(m_buffers_updated.begin(), m_buffers_updated.end(), false);
        }

    } // graphics

} // undicht