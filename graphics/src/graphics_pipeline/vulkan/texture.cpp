#include "texture.h"

namespace undicht {

    namespace graphics {

        Texture::Texture(const GraphicsDevice *device) : m_staging_buffer(device) {

            m_device_handle = device;

            m_sampler = new vk::Sampler;
            m_image = new vk::Image;
            m_image_view = new vk::ImageView;
            m_memory = new vk::DeviceMemory;
            m_format = new vk::Format(vk::Format::eR8G8B8A8Srgb);
            m_current_layout = new vk::ImageLayout(vk::ImageLayout::eUndefined);
            m_image_ready = new vk::Semaphore;

            if(!m_device_handle) return;

            initStagingBuffer();

            // init semaphore
            vk::SemaphoreCreateInfo semaphore_info;
            *m_image_ready = m_device_handle->m_device->createSemaphore(semaphore_info);
        }

        Texture::Texture(const Texture& tex) : Texture(0) {

            // copy content
            m_device_handle = tex.m_device_handle;
            *m_sampler = *tex.m_sampler;
            *m_image = *tex.m_image;
            *m_image_view = *tex.m_image_view;
            *m_memory = *tex.m_memory;
            *m_format = *tex.m_format;
            *m_current_layout = *tex.m_current_layout;
            *m_image_ready = *tex.m_image_ready;

        }

        Texture::~Texture() {

            cleanUp();

            delete m_sampler;
            delete m_image_view;
            delete m_image;
            delete m_memory;
            delete m_format;
            delete m_current_layout;
            delete m_image_ready;
        }

        const Texture& Texture::operator=(const Texture& tex) {

            // protection against self assignment
            if(this == &tex) return *this;

            // destroy previous content
            cleanUp();

            // copy content
            *m_sampler = *tex.m_sampler;
            *m_image = *tex.m_image;
            *m_image_view = *tex.m_image_view;
            *m_memory = *tex.m_memory;
            *m_format = *tex.m_format;
            *m_current_layout = *tex.m_current_layout;
            *m_image_ready = *tex.m_image_ready;

            return *this;
        }

        void Texture::cleanUp() {

            if(!m_device_handle) return;

            m_device_handle->m_device->destroySampler(*m_sampler);
            m_device_handle->m_device->destroyImageView(*m_image_view);
            if(m_own_image) m_device_handle->m_device->destroyImage(*m_image);
            m_device_handle->m_device->freeMemory(*m_memory);
            m_device_handle->m_device->destroySemaphore(*m_image_ready);
        }

        ///////////////////////// specifying the textures layout ///////////////////////////////

        void Texture::setSize(uint32_t width, uint32_t height, uint32_t layers) {

            m_width = width;
            m_height = height;
            m_layers = layers;

        }

        void Texture::setFormat(const FixedType& format) {

            *m_format = translateVulkanFormat(format);
            m_pixel_type = format;
        }

        void Texture::finalizeLayout() {

            if(m_own_image) {

                vk::ImageCreateInfo info;
                info.extent = vk::Extent3D(m_width, m_height, 1);
                info.imageType = vk::ImageType::e2D;
                info.arrayLayers = m_layers;
                info.mipLevels = 1;
                info.format = *m_format;
                info.tiling = vk::ImageTiling::eOptimal;
                info.initialLayout = *m_current_layout;
                info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
                info.sharingMode = vk::SharingMode::eExclusive; // used exclusively by the graphics queue
                info.samples = vk::SampleCountFlagBits::e1; // used for multisampling

                *m_image = m_device_handle->m_device->createImage(info);

                allocate();

            }

            initImageView();
            initSampler();

        }

        /////////////////////////////  initializing the texture ////////////////////////////////

        void Texture::initStagingBuffer() {

            // queue families this buffer is going to be accessed from
            std::vector<uint32_t> queue_ids;
            queue_ids.push_back(m_device_handle->m_graphics_queue_id);

            // memory properties
            vk::MemoryPropertyFlags mem_properties; // needs to be directly accessible by the cpu
            mem_properties |= vk::MemoryPropertyFlagBits::eHostCoherent;
            mem_properties |= vk::MemoryPropertyFlagBits::eHostVisible;

            // usage
            vk::BufferUsageFlags usage_flags = {};
            usage_flags |= vk::BufferUsageFlagBits::eTransferSrc;

            m_staging_buffer.setUsage(usage_flags, mem_properties,  queue_ids);

        }

        void Texture::initImageView() {

            vk::ImageViewCreateInfo info;
            info.image = *m_image;
            info.viewType = vk::ImageViewType::e2D;
            info.format = *m_format;
            info.components = vk::ComponentSwizzle::eIdentity;
            info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            *m_image_view = m_device_handle->m_device->createImageView(info);

        }

        void Texture::initSampler() {

            vk::SamplerCreateInfo info;
            info.magFilter = vk::Filter::eNearest;
            info.minFilter = vk::Filter::eLinear;
            info.addressModeU = vk::SamplerAddressMode::eRepeat;
            info.addressModeV = vk::SamplerAddressMode::eRepeat;
            info.addressModeW = vk::SamplerAddressMode::eRepeat;
            info.anisotropyEnable = VK_TRUE;
            info.maxAnisotropy = m_device_handle->getAnisotropyLimit();
            info.borderColor = vk::BorderColor::eIntOpaqueBlack;
            info.unnormalizedCoordinates = VK_FALSE; // use uv range of [0,1] instead of the actual pixel size
            info.compareEnable = VK_FALSE;
            info.compareOp = vk::CompareOp::eAlways;
            info.mipmapMode = vk::SamplerMipmapMode::eLinear;
            info.mipLodBias = 0.0f;
            info.minLod = 0.0f;
            info.maxLod = 0.0f;

            *m_sampler = m_device_handle->m_device->createSampler(info);
        }

        void Texture::writeDescriptorSets(const std::vector<vk::DescriptorSet>* shader_descriptors, uint32_t index, uint32_t frame_id) const {

            vk::DescriptorImageInfo image_info;
            image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            image_info.imageView = *m_image_view;
            image_info.sampler = *m_sampler;

            vk::WriteDescriptorSet descriptor_write;
            descriptor_write.dstBinding = index;
            descriptor_write.pImageInfo = &image_info;
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pTexelBufferView = nullptr;
            descriptor_write.dstSet = shader_descriptors->at(frame_id);

            m_device_handle->m_device->updateDescriptorSets(descriptor_write, nullptr);

        }

        void Texture::allocate() {

            // getting the requirements needed for the texture
            vk::MemoryRequirements requirements;
            requirements = m_device_handle->m_device->getImageMemoryRequirements(*m_image);

            // specifying the type of memory we want for the texture
            vk::MemoryType memory_type;
            memory_type.heapIndex = requirements.memoryTypeBits; // bitfield specifying which memory types can be used
            memory_type.propertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal; // on the actual graphics hardware

            // filling out the allocate info
            vk::MemoryAllocateInfo allocate_info;
            allocate_info.allocationSize = requirements.size;
            allocate_info.memoryTypeIndex = m_device_handle->findMemory(memory_type);

            // allocating the memory
            *m_memory = m_device_handle->m_device->allocateMemory(allocate_info);

            // binding the memory to the image
            m_device_handle->m_device->bindImageMemory(*m_image, *m_memory, 0);

        }

        void Texture::transitionToLayout(vk::ImageLayout new_layout) {

            // allocating a command buffer
            vk::Queue* queue = m_device_handle->m_graphics_queue;
            vk::CommandPool* cmd_pool = m_device_handle->m_graphics_command_pool;
            vk::CommandBuffer cmd_buffer = m_device_handle->beginSingleTimeCommand(*cmd_pool);

            vk::ImageMemoryBarrier mem_barrier = genMemBarrier(*m_format, *m_current_layout, new_layout);
            vk::PipelineStageFlagBits start_stage = choosePreBarrierStage(*m_current_layout, new_layout);
            vk::PipelineStageFlagBits wait_stage = chooseWaitStage(*m_current_layout, new_layout);
            cmd_buffer.pipelineBarrier(start_stage, wait_stage, {},{}, {}, mem_barrier);

            // ending & submitting the command buffer
            m_device_handle->endSingleTimeCommand(cmd_buffer, *cmd_pool, *queue);

            *m_current_layout = new_layout;
        }

        vk::ImageMemoryBarrier Texture::genMemBarrier(vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout) {

            vk::ImageMemoryBarrier memory_barrier;
            memory_barrier.oldLayout = old_layout;
            memory_barrier.newLayout = new_layout;
            memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.image = *m_image;
            memory_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            memory_barrier.subresourceRange.baseMipLevel = 0;
            memory_barrier.subresourceRange.levelCount = 1;
            memory_barrier.subresourceRange.baseArrayLayer = 0;
            memory_barrier.subresourceRange.layerCount = 1;

            // choosing the right access masks
            if(old_layout == vk::ImageLayout::eUndefined)
                memory_barrier.srcAccessMask = {};

            if(old_layout == vk::ImageLayout::eTransferDstOptimal)
                memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;

            if(new_layout == vk::ImageLayout::eTransferDstOptimal)
                memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            if(new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
                memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            return memory_barrier;
        }

        vk::PipelineStageFlagBits Texture::choosePreBarrierStage(vk::ImageLayout old_layout, vk::ImageLayout new_layout) const {
            // the stages that must happen before the layout transition

            if(old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
                return vk::PipelineStageFlagBits::eTopOfPipe; // transition can start instantly

            if(old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
                return vk::PipelineStageFlagBits::eTransfer; // do this after the transfer stage is finished

            UND_WARNING << "failed to find correct begin stage for texture operation\n"
                        << "    , now starting on top of the graphics pipeline (may cause unwanted/undefined behaviour)\n";

            return vk::PipelineStageFlagBits::eTopOfPipe;
        }


        vk::PipelineStageFlagBits Texture::chooseWaitStage(vk::ImageLayout old_layout, vk::ImageLayout new_layout) const {
            // at this stage the layout transition will be waited on to finish

            if(old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
                return vk::PipelineStageFlagBits::eTransfer; // finish transition before transfer stage

            if(old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
                return vk::PipelineStageFlagBits::eFragmentShader; // finish transition before access by fragment shaders

            UND_WARNING << "failed to find correct wait stage for texture operation\n"
                        << "    , now waiting on top of the graphics pipeline (may slow performance)\n"
                        << "    , or cause unwanted / undefined behaviour";

            return vk::PipelineStageFlagBits::eTopOfPipe;
        }

        ///////////////////////////////////// setting data /////////////////////////////////////

        void Texture::setData(const char* data, uint32_t byte_size) {

            transitionToLayout(vk::ImageLayout::eTransferDstOptimal);

            // storing the data in the staging buffer
            m_staging_buffer.setData(data, byte_size, 0);

            // copying the data into the texture using a command buffer
            vk::Queue* queue = m_device_handle->m_graphics_queue;
            vk::CommandPool* cmd_pool = m_device_handle->m_graphics_command_pool;
            vk::CommandBuffer cmd_buffer = m_device_handle->beginSingleTimeCommand(*cmd_pool);

            // copying the buffer to the image
            vk::BufferImageCopy region = genCopyRegion();
            cmd_buffer.copyBufferToImage(*m_staging_buffer.m_buffer, *m_image, *m_current_layout, region);

            // ending & submitting the command buffer
            m_device_handle->endSingleTimeCommand(cmd_buffer, *cmd_pool, *queue);

            transitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        }

        ///////////////////////////////// private functions for setting data /////////////////////////////////

        vk::BufferImageCopy Texture::genCopyRegion() const {

            vk::BufferImageCopy region;
            region.bufferOffset = 0; // layout of the data in the buffer
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = vk::Offset3D{0, 0, 0};
            region.imageExtent = vk::Extent3D{m_width, m_height, 1};

            return region;
        }

    } // graphics

} // undicht