#ifndef TRUE_TYPE_H
#define TRUE_TYPE_H

#include "user_interface/font.h"
#include "cstdint"

namespace undicht {

    namespace tools {

        class TrueType {
            // font file format
        public:

            TrueType();
            TrueType(const std::string& file_name, graphics::Font& font);

            bool loadFont(const std::string& file_name, graphics::Font& font) const;

        private:
            // processing font data

            void genFontMap(std::vector<unsigned char>& data, graphics::Font& font) const;
            uint32_t calcFontMapHeight(uint32_t font_height, uint32_t char_count, uint32_t font_map_width) const;

        };

    } // tools

} // undicht

#endif // TRUE_TYPE_H