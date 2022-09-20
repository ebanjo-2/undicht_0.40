#ifndef IMAGE_FILE_H
#define IMAGE_FILE_H

#include "string"
#include "vector"
#include "undicht_graphics.h"

namespace undicht {

    namespace tools {

        struct ImageData {
            std::vector<char> _pixels;
            uint32_t _width = 0;
            uint32_t _height = 0;
            uint32_t _nr_channels = 0;
        };

        class ImageFile {
        public:

            ImageFile() = default;
            ImageFile(const std::string& file_name, ImageData& data);
            ImageFile(const std::string& file_name, graphics::Texture& texture);

            bool loadImage(const std::string& file_name, ImageData& data);
            bool loadImage(const std::string& file_name, graphics::Texture& texture);

        };

    } // tools

} // undicht

#endif // IMAGE_FILE_H