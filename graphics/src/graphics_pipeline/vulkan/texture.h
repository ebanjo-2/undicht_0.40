#ifndef TEXTURE_H
#define TEXTURE_H

#include "core/vulkan/vulkan_declaration.h"
#include "cstdint"
#include "graphics_pipeline/vulkan/vram_buffer.h"

namespace undicht {

    namespace graphics {

        class GraphicsDevice;
        class Renderer;

        class Texture {
        private:

            uint32_t m_width = 0;
            uint32_t m_height = 0;
            uint32_t m_layers = 1;

            vk::Image* m_image = 0;
            vk::ImageView* m_image_view = 0;
            vk::DeviceMemory* m_memory = 0;
            vk::Sampler* m_sampler = 0;

            vk::Format* m_format = 0;
            vk::ImageLayout* m_current_layout = 0;

            VramBuffer m_staging_buffer;

            friend GraphicsDevice;
            friend Renderer;
            const GraphicsDevice* m_device_handle = 0;

            Texture(const GraphicsDevice* device);

            void cleanUp();

        public:

            ~Texture();

        public:
            // specifying the textures layout

            void setSize(uint32_t width, uint32_t height, uint32_t layers = 1);

            void finalizeLayout();

        private:
            // initializing the texture

            void initStagingBuffer();
            void initImageView();
            void initSampler();

            // tells the descriptors that this texture will be accessed in the shader under the index
            void writeDescriptorSets(const std::vector<vk::DescriptorSet>* shader_descriptors, uint32_t index, uint32_t frame_id) const;

            void allocate(uint32_t byte_size);

            // the texture can be transitioned between different layouts
            // this way it can be in the optimal layout for transferring data, graphics, ..
            // which might differ from one another
            void transitionToLayout(vk::ImageLayout new_layout);
            vk::ImageMemoryBarrier genMemBarrier(vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
            vk::PipelineStageFlagBits choosePreBarrierStage(vk::ImageLayout old_layout, vk::ImageLayout new_layout) const; // the stages that must happen before the layout transition
            vk::PipelineStageFlagBits chooseWaitStage(vk::ImageLayout old_layout, vk::ImageLayout new_layout) const; // at this stage the layout transition will be waited on to finish


        public:
            // setting data

            void setData(const char* data, uint32_t byte_size);

        private:
            // private functions for setting data

            vk::BufferImageCopy genCopyRegion() const;

        };

    } // graphics

} // undicht

#endif // TEXTURE_H