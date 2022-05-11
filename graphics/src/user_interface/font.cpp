#include "font.h"
#include "debug.h"

namespace undicht {

    namespace graphics {

        Font::Font(const GraphicsDevice *device) : m_font_map(device){

        }

        FontChar Font::getCharData(char c) const {

            for(const FontChar& char_data : m_chars) {

                if(char_data._c == c)
                    return char_data;
            }

            UND_ERROR << "failed to find char: " << c << "\n";

            return m_chars.back();
        }

        void Font::buildString(const std::string& text, VertexBuffer& loadTo) const {

            // specifying the vbo layout
            loadTo.setVertexAttribute(0, UND_VEC2F); // position of the corners of the base rectangle
            loadTo.setInstanceAttribute(0, UND_VEC2F); // font map uvs
            loadTo.setInstanceAttribute(1, UND_VEC2F); // font map size
            loadTo.setInstanceAttribute(2, UND_VEC2F); // line offset
            loadTo.setInstanceAttribute(3, UND_FLOAT32); // advance

            // simple rectangle data
            loadTo.setVertexData({0.0f, 0.0f, 1.0f, 0.0f, 1.0f,1.0f, 0.0f,1.0f});
            loadTo.setIndexData({0,1,2, 2,3,0});

            // generating the characters (each is an instance of the base rectangle)
            struct CharData{
                float _u = 0.0f;
                float _v = 0.0f;
                float _w = 0.0f; // fontmap width
                float _h = 0.0f; // fontmap height
                float _x = 0.0f;
                float _y = 0.0f;
                float _a = 0.0f; // advance
            };

            std::vector<CharData> data;
            float advance = 0.0f;
            for(char c : text) {
                FontChar d = getCharData(c);

                CharData char_data;
                char_data._u = d._font_map_x;
                char_data._v = d._font_map_y;
                char_data._w = d._font_map_width;
                char_data._h = d._font_map_height;
                char_data._x = d._offset_x;
                char_data._y = d._offset_y;
                char_data._a = advance;

                data.push_back(char_data);
                advance += d._advance;
            }

            // storing the char data
            loadTo.setInstanceData(data.data(), data.size() * sizeof(CharData), 0);

        }

    } // graphics

} // undicht