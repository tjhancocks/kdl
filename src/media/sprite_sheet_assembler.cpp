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
#include "media/image/tga.hpp"
#include "media/image/png.hpp"
#include "sprite_sheet_assembler.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Construction

kdl::media::sprite_sheet_assembler::sprite_sheet_assembler(std::vector<std::string> input_file_contents,
                                                           const kdl::lexeme input)
    : m_input_file_format(input)
{
    for (auto f : input_file_contents) {
        m_input_file_contents.emplace_back(std::make_shared<std::vector<char>>(f.begin(), f.end()));
    }
}

// MARK: -

auto kdl::media::sprite_sheet_assembler::assemble() const -> std::vector<char>
{
    std::vector<std::shared_ptr<graphite::qd::surface>> sprites;

    // Ensure consistent size
    int16_t width = 0;
    int16_t height = 0;

    // Import the file data as appropriate images, and ensure consistent sizes.
    if (m_input_file_format.is("TGA")) {
        for (auto i = 0; i < m_input_file_contents.size(); ++i) {
            auto tga = std::make_unique<image::tga>(m_input_file_contents[i]);
            if (auto surface = tga->surface().lock()) {
                sprites.emplace_back(surface);
            }
            else {
                log::fatal_error(m_input_file_format, 1, "One of the input files could not be parsed.");
            }
        }
    }
    else if (m_input_file_format.is("PNG")) {
        for (auto i = 0; i < m_input_file_contents.size(); ++i) {
            auto png = std::make_unique<image::png>(m_input_file_contents[i]);
            if (auto surface = png->surface().lock()) {
                sprites.emplace_back(surface);
            }
            else {
                log::fatal_error(m_input_file_format, 1, "One of the input files could not be parsed.");
            }
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
    int16_t required_width = 0;
    int16_t required_height = 0;
    std::vector<graphite::qd::rect> sprite_rects;
    std::vector<graphite::qd::rect> sprite_sheet_rects;
    int16_t y = 0;

    for (auto sprite : sprites) {

        // Calculate the bounding box of the sprite
        graphite::qd::rect edges(0, 0, sprite->size().width(), sprite->size().height());

        // left edge
        for (auto x = 0; x < edges.width() / 2; ++x) {
            for (auto y = 0; y < edges.height(); ++y) {
                if (sprite->at(x, y).alpha_component() != 0) {
                    edges.set_x(x);
                    goto LEFT_EDGE_FOUND;
                }
            }
        }
        LEFT_EDGE_FOUND:

        // right edge
        for (auto x = edges.width() - 1; x > edges.width() / 2; --x) {
            for (auto y = 0; y < edges.height(); ++y) {
                if (sprite->at(x, y).alpha_component() != 0) {
                    edges.set_width(x - edges.x());
                    goto RIGHT_EDGE_FOUND;
                }
            }
        }
        RIGHT_EDGE_FOUND:

        // top edge
        for (auto y = 0; y < edges.height() / 2; ++y) {
            for (auto x = 0; x < edges.width(); ++x) {
                if (sprite->at(x + edges.x(), y).alpha_component() != 0) {
                    edges.set_y(y);
                    goto TOP_EDGE_FOUND;
                }
            }
        }
        TOP_EDGE_FOUND:

        // bottom edge
        for (auto y = edges.height() - 1; y > edges.height() / 2; --y) {
            for (auto x = 0; x < edges.width(); ++x) {
                if (sprite->at(x + edges.x(), y).alpha_component() != 0) {
                    edges.set_height(y - edges.y());
                    goto BOTTOM_EDGE_FOUND;
                }
            }
        }
        BOTTOM_EDGE_FOUND:

        sprite_rects.emplace_back(edges);

        if (sprite_sheet_rects.empty()) {
            sprite_sheet_rects.emplace_back(graphite::qd::rect(0, 0, edges.width(), edges.height()));
        }
        else {
            auto x = sprite_sheet_rects.back().x() + sprite_sheet_rects.back().width();
            auto rx = x + edges.width();

            if (rx > sprites.front()->size().width() * (sprites.size() / 10)) {
                x = 0;
                y = required_height;
            }

            sprite_sheet_rects.emplace_back(graphite::qd::rect(x, y, edges.width(), edges.height()));
        }

        required_width = std::max(required_width, static_cast<int16_t>(sprite_sheet_rects.back().x() + sprite_sheet_rects.back().width()));
        required_height = std::max(required_height, static_cast<int16_t>(sprite_sheet_rects.back().y() + sprite_sheet_rects.back().height()));
    }

    // Produce the output surface, with each of the sprites merged into a sprite sheet.
    // Calculate the geometry of the surface to some optimal.
    auto sheet = std::make_shared<graphite::qd::surface>(required_width, required_height);

    for (auto sprite_index = 0; sprite_index < sprites.size(); ++sprite_index) {
        auto sprite_frame = sprite_rects[sprite_index];
        auto sheet_frame = sprite_sheet_rects[sprite_index];

        auto sprite_x = sheet_frame.x();
        auto sprite_y = sheet_frame.y();
        auto sprite = sprites[sprite_index];

        for (auto y = 0; y < sprite_frame.height(); ++y) {
            for (auto x = 0; x < sprite_frame.width(); ++x) {
                sheet->set(sprite_x + x, sprite_y + y, sprite->at(x + sprite_frame.x(), y + sprite_frame.y()));
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
    auto sprite_sheet_data = std::make_shared<graphite::data::data>();
    graphite::data::writer sprite_sheet_writer(sprite_sheet_data);

    sprite_sheet_writer.write_short(1);
    sprite_sheet_writer.write_short(sprite_sheet_rects.size());
    for (auto rect : sprite_sheet_rects) {
        sprite_sheet_writer.write_short(rect.x());
        sprite_sheet_writer.write_short(rect.y());
        sprite_sheet_writer.write_short(rect.width());
        sprite_sheet_writer.write_short(rect.height());
    }

    auto tga_data = tga.data();
    sprite_sheet_writer.write_long(tga_data.size());
    sprite_sheet_writer.write_bytes(tga.data());

    return *sprite_sheet_data->get();
}
