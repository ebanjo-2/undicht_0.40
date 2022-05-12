#include "true_type.h"
#include "stb_truetype.h"
#include "fstream"

#include "debug.h"

namespace undicht {

    namespace tools {

        TrueType::TrueType() {

        }

        TrueType::TrueType(const std::string &file_name, graphics::Font &font) {

            loadFont(file_name, font);
        }

        bool TrueType::loadFont(const std::string &file_name, graphics::Font &font) const {

            // open the file (begin at the end)
            std::ifstream file(file_name, std::ios_base::ate);

            // check if file was opened
            if(!file.is_open()) {
                UND_LOG << "failed to open font file: " << file_name << "\n";
                return false;
            }

            // load entire file
            uint32_t file_end = file.tellg();
            file.seekg(file.beg);
            std::vector<unsigned char> data(file_end - file.tellg());
            file.read((char*)data.data(), file_end - file.tellg());

            // generate font map
            genFontMap(data, font);

            return true;
        }


        /////////////////////////////////// processing font data ///////////////////////////////////

        void TrueType::genFontMap(std::vector<unsigned char>& data, graphics::Font& font) const {

            // determining which chars should be placed on the font map
            const uint32_t first_char = 30;
            const uint32_t last_char = 255;
            const uint32_t char_count = last_char - first_char;

            // determining the size of the font map
            int font_height = 64; // 64 pixels in height
            float font_map_width = 512; // fixed
            float font_map_height = calcFontMapHeight(font_height, char_count, font_map_width);

            // asking stb to generate a font map
            std::vector<unsigned char> font_map_data(font_map_width * font_map_height);
            stbtt_bakedchar* char_data = new stbtt_bakedchar[char_count]; // contains info about the chars such as size and position
            int result = stbtt_BakeFontBitmap(data.data(), 0, font_height, font_map_data.data()
                                              , font_map_width, font_map_height, first_char, char_count, char_data);

            // checking the result of the font map generation
            if(result > (-1 * char_count)) UND_ERROR << "failed to place all chars on fontmap\n";
            if(!result) UND_ERROR << "failed to place any chars on fontmap\n";

            // the font map might have been to large (stb calculated the actual size)
            font_map_height = result;
            font_map_data.resize(font_map_width * font_map_height);

            // storing the data in the font map
            font.m_font_map.setSize(font_map_width, font_map_height);
            font.m_font_map.setFormat(UND_R8);
            font.m_font_map.finalizeLayout();
            font.m_font_map.setData((char*)font_map_data.data(), font_map_data.size());

            // storing the char data
            for(int i = 0; i < char_count; i++) {

                stbtt_bakedchar* stbchar = char_data + i;
                graphics::FontChar mychar;

                mychar._c = i + first_char;
                mychar._font_map_x = stbchar->x0 / font_map_width; // relative coordinates ranging from 0 to 1
                mychar._font_map_y = stbchar->y0 / font_map_height; // relative coordinates ranging from 0 to 1
                mychar._font_map_width = (stbchar->x1 - stbchar->x0) / font_map_width;
                mychar._font_map_height = (stbchar->y1 - stbchar->y0) / font_map_height;
                mychar._advance = stbchar->xadvance / font_map_width;
                mychar._offset_x = stbchar->xoff / font_map_width;
                mychar._offset_y = stbchar->yoff / font_map_height;

                font.m_chars.push_back(mychar);
            }

            delete[] char_data;
        }

        uint32_t TrueType::calcFontMapHeight(uint32_t font_height, uint32_t char_count, uint32_t font_map_width) const{

            const int chars_per_line = font_map_width / font_height;
            const int required_lines = char_count / (chars_per_line) + 1;
            const int font_map_height = font_height * required_lines;

            return font_map_height;
        }


    } // graphics

} // undicht