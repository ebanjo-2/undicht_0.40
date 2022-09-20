#include <graphics_pipeline/vulkan/texture.h>
#include "image_file.h"
#include "stb_image.h"
#include "debug.h"

namespace undicht {

    namespace tools {

        ImageFile::ImageFile(const std::string &file_name, ImageData &data) {

            loadImage(file_name, data);
        }

        ImageFile::ImageFile(const std::string& file_name, graphics::Texture& texture) {

            loadImage(file_name, texture);
        }


        bool ImageFile::loadImage(const std::string &file_name, ImageData &data) {

            stbi_set_flip_vertically_on_load(true);
            unsigned char* tmp = stbi_load(file_name.data(), (int*)&data._width, (int*)&data._height, (int*)&data._nr_channels, STBI_rgb_alpha);

            if(!tmp) {
                UND_ERROR << "failed to read image file: " << file_name << "\n";
                return false;
            }

            uint32_t image_size = data._width * data._height * 4;
            data._pixels.insert(data._pixels.begin(), tmp, tmp + image_size);
            data._nr_channels = 4;

            stbi_image_free(tmp);

            return true;
        }

        bool ImageFile::loadImage(const std::string &file_name, graphics::Texture& texture) {

            ImageData data;

            if(!loadImage(file_name, data))
                return false;

            // storing the data in the texture
            texture.setSize(data._width, data._height, 1);
            texture.finalizeLayout();
            texture.setData(data._pixels.data(), data._pixels.size());

            return true;
        }

    } // tools

} // undicht