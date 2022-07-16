// Copyright (c) 2020 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <complex>
#include <limits>
#include "media/image/tga.hpp"
#include "media/image/png.hpp"
#include "sprite_sheet_assembler.hpp"
#include "diagnostic/fatal.hpp"
#include <libGraphite/quickdraw/type/rect.hpp>

// MARK: - Construction

kdl::media::sprite_sheet_assembler::sprite_sheet_assembler(const std::vector<graphite::data::block>& input_file_contents, kdl::lexeme input)
    : m_input_file_format(std::move(input))
{
    for (auto file : input_file_contents) {
        m_input_file_contents.emplace_back(file);
    }
}

// MARK: -

auto kdl::media::sprite_sheet_assembler::assemble() const -> graphite::data::block
{
    std::vector<graphite::quickdraw::surface> sprites;

    // Ensure consistent size
    std::int16_t width = 0;
    std::int16_t height = 0;

    // Import the file data as appropriate images, and ensure consistent sizes.
    if (m_input_file_format.is("TGA")) {
        for (auto i = 0; i < m_input_file_contents.size(); ++i) {
            image::tga tga(m_input_file_contents[i]);
            sprites.emplace_back(std::move(tga.surface()));
        }
    }
    else if (m_input_file_format.is("PNG")) {
        for (auto i = 0; i < m_input_file_contents.size(); ++i) {
            image::png png(m_input_file_contents[i]);
            sprites.emplace_back(std::move(png.surface()));
        }
    }
    else {
        log::fatal_error(m_input_file_format, 1, "Unable to handle input format '" + m_input_file_format.text() + "'");
    }

    // "Trim" the sprites, and determine where to actually place them to reduce the amount of dead space in the output
    // sprite sheet.
    // This code is not pretty, or efficient but it works.
    //  Future improvements:
    //      - Remove use of goto and extract into smaller functions.
    //      - The runtime complexity of all of this is pretty dire, it is a naive
    //        bruteforce algorithm.
    std::int16_t required_width = 0;
    std::int16_t required_height = 0;
    std::vector<graphite::quickdraw::rect<std::int16_t>> sprite_rects;
    std::vector<graphite::quickdraw::rect<std::int16_t>> sprite_sheet_rects;
    std::int16_t y = 0;

    std::int16_t left_edge = std::numeric_limits<std::int16_t>::max();
    std::int16_t right_edge = 0;
    std::int16_t top_edge = std::numeric_limits<std::int16_t>::max();
    std::int16_t bottom_edge = 0;

    for (const auto& sprite : sprites) {
        auto w = sprite.size().width;
        auto h = sprite.size().height;
        auto hw = static_cast<std::int16_t>(w >> 1);
        auto hh = static_cast<std::int16_t>(h >> 1);

        for (std::int16_t y = 0; y < hh; ++y) {
            for (std::int16_t x = 0; x < hw; ++x) {
                const auto& c1 = sprite.at(x, y);
                const auto& c2 = sprite.at(hw + x, y);
                if (c1.components.alpha != 0 || c2.components.alpha != 0) {
                    top_edge = std::min(y, top_edge);
                    goto TOP_FOUND;
                }
            }
        }
        TOP_FOUND:

        for (std::int16_t y = h - 1; y >= hh; --y) {
            for (std::int16_t x = 0; x < hw; ++x) {
                const auto& c1 = sprite.at(x, y);
                const auto& c2 = sprite.at(hw + x, y);
                if (c1.components.alpha != 0 || c2.components.alpha != 0) {
                    bottom_edge = std::max(y, bottom_edge);
                    goto BOTTOM_FOUND;
                }
            }
        }
        BOTTOM_FOUND:

        for (std::int16_t x = 0; x < hw; ++x) {
            for (std::int16_t y = 0; y < hh; ++y) {
                const auto& c1 = sprite.at(x, y);
                const auto& c2 = sprite.at(x, hh + y);
                if (c1.components.alpha != 0 || c2.components.alpha != 0) {
                    left_edge = std::min(x, left_edge);
                    goto LEFT_FOUND;
                }
            }
        }
        LEFT_FOUND:

        for (std::int16_t x = w - 1; x >= hw; --x) {
            for (std::int16_t y = 0; y < hh; ++y) {
                const auto& c1 = sprite.at(x, y);
                const auto& c2 = sprite.at(x, hh + y);
                if (c1.components.alpha != 0 || c2.components.alpha != 0) {
                    right_edge = std::max(x, right_edge);
                    goto RIGHT_FOUND;
                }
            }
        }
        RIGHT_FOUND:
        continue;

    }

    graphite::quickdraw::rect<std::int16_t> edges(left_edge, top_edge,right_edge - left_edge, bottom_edge - top_edge);

    for (const auto& sprite : sprites) {
        sprite_rects.emplace_back(edges);

        if (sprite_sheet_rects.empty()) {
            sprite_sheet_rects.emplace_back(0, 0, edges.size.width, edges.size.height);
        }
        else {
            std::int16_t x = sprite_sheet_rects.back().origin.x + sprite_sheet_rects.back().size.width;
            std::int16_t rx = x + edges.size.width;

            if (rx > sprites.front().size().width * (sprites.size() / 10)) {
                x = 0;
                y = required_height;
            }

            sprite_sheet_rects.emplace_back(x, y, edges.size.width, edges.size.height);
        }

        required_width = std::max(required_width, static_cast<std::int16_t>(sprite_sheet_rects.back().origin.x + sprite_sheet_rects.back().size.width));
        required_height = std::max(required_height, static_cast<std::int16_t>(sprite_sheet_rects.back().origin.y + sprite_sheet_rects.back().size.height));
    }

    // Produce the output surface, with each of the sprites merged into a sprite sheet.
    // Calculate the geometry of the surface to some optimal.
    graphite::quickdraw::surface sheet(required_width, required_height);

    for (auto sprite_index = 0; sprite_index < sprites.size(); ++sprite_index) {
        auto sprite_frame = sprite_rects[sprite_index];
        auto sheet_frame = sprite_sheet_rects[sprite_index];

        std::int16_t sprite_x = sheet_frame.origin.x;
        std::int16_t sprite_y = sheet_frame.origin.y;
        auto sprite = sprites[sprite_index];

        for (int16_t yy = 0; yy < sprite_frame.size.height; ++yy) {
            for (int16_t x = 0; x < sprite_frame.size.width; ++x) {
                sheet.set(sprite_x + x, sprite_y + yy, sprite.at(x + sprite_frame.origin.x, yy + sprite_frame.origin.y));
            }
        }
    }

    // Return the data for the sprite sheet that was assembled.
    image::tga tga(sheet);

    // Assemble the information for the sprite sheet itself. This includes each of the sprite frames in order.
    // The format is as follows:
    //
    //  uint16_t    format_version      = 1
    //  uint16_t    frames - 1
    //      uint16_t    x
    //      uint16_t    y
    //      uint16_t    width
    //      uint16_t    height
    //  uint32_t    tga_size
    //  uint8_t...  tga
    graphite::data::writer sprite_sheet_writer(graphite::data::byte_order::lsb);

    sprite_sheet_writer.write_short(1);
    sprite_sheet_writer.write_short(sprite_sheet_rects.size());
    for (auto rect : sprite_sheet_rects) {
        sprite_sheet_writer.write_short(rect.origin.x);
        sprite_sheet_writer.write_short(rect.origin.y);
        sprite_sheet_writer.write_short(rect.size.width);
        sprite_sheet_writer.write_short(rect.size.height);
    }

    auto tga_data = tga.data();
    sprite_sheet_writer.write_long(tga_data.size());
    sprite_sheet_writer.write_data(&tga_data);

    return std::move(*const_cast<graphite::data::block *>(sprite_sheet_writer.data()));
}
