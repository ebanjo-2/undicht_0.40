#ifndef FONT_H
#define FONT_H

#include "vector"
#include "undicht_graphics.h"

namespace undicht {

    namespace graphics {

        class FontChar {
            // holds all the data for characters of a font
        public:

            char _c = 0;

            // position on font map
            float _font_map_x = 0.0f;
            float _font_map_y = 0.0f;
            float _font_map_width = 0.0f;
            float _font_map_height = 0.0f;

            // "behaviour" in a line of text
            float _offset_x = 0.0f; // position of the upper left corner
            float _offset_y = 0.0f;
            float _advance = 0.0f; // advance after the char

        };

        class Font {
            // holds all the data for a font

        public:

            Texture m_font_map;
            std::vector<FontChar> m_chars;

            Font(const GraphicsDevice* device);

        public:

            FontChar getCharData(char c) const;

            void buildString(const std::string& text, VertexBuffer& loadTo) const;

        };

    } // graphics

} // undicht

#endif