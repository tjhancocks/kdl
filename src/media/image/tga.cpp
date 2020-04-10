// Copyright (c) 2019-2020 Tom Hancocks
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
#include "tga.hpp"

// MARK: - Constructors

kdl::media::image::tga::tga(const std::string path)
{
    // TARGA images are stored in little endian, so set the byte order accordingly
    graphite::data::reader reader(path);
    reader.get()->set_byte_order(graphite::data::data::lsb);

    // TODO: Possibly handle any errors that occur?
    decode(reader);
}

// MARK: - Decoding

auto kdl::media::image::tga::decode(graphite::data::reader &reader) -> bool
{
    // Read the TGA header from the image
    tga::header header;
    header.id_length = reader.read_byte();
    header.color_map_type = reader.read_byte();
    header.data_type_code = reader.read_byte();
    header.color_map_origin = reader.read_short();
    header.color_map_length = reader.read_short();
    header.color_map_depth = reader.read_byte();
    header.x_origin = reader.read_short();
    header.y_origin = reader.read_short();
    header.width = reader.read_short();
    header.height = reader.read_short();
    header.bits_per_pixel = reader.read_byte();
    header.image_descriptor = reader.read_byte();

    // Setup a QuickDraw surface for the image to be read into. The buffer should be completely
    // black by default. This will be the "default" image in the event we fail to read.
    m_surface = std::make_shared<graphite::qd::surface>(header.width, header.height);

    // Make sure this is a TGA image that we can handle.
    if (header.data_type_code != 2 && header.data_type_code != 10) {
        std::cerr << "TGA Decoder: Can only handle image type 2 and 10" << std::endl;
        return false;
    }

    if (header.bits_per_pixel != 16 && header.bits_per_pixel != 24 && header.bits_per_pixel != 32) {
        std::cerr << "TGA Decoder: Can only handle image depths of 16, 24 or 32-bits" << std::endl;
        return false;
    }

    if (header.color_map_type != 0 && header.color_map_type != 1) {
        std::cerr << "TGA Decoder: Can only handle color map types of 0 and 1" << std::endl;
        return false;
    }

    // Ignore unrequired information.
    reader.move(header.id_length);
    reader.move(header.color_map_type * header.color_map_length);

    // Start reading the image itself.
    int bytes_to_read = header.bits_per_pixel >> 3;
    int n = 0;
    while (n < header.width * header.height) {
        switch (header.data_type_code) {
            // Uncompressed
            case 2: {
                auto v = reader.read_bytes(bytes_to_read);
                merge_bytes(n, v, 0, bytes_to_read);
                ++n;
                break;
            }
            // Compressed
            case 10: {
                auto v = reader.read_bytes(bytes_to_read + 1);
                int j = v[0] & 0x7F;
                merge_bytes(n, v, 1, bytes_to_read);
                ++n;
                if (v[0] & 0x80) { // RLE Chunk?
                    for (auto i = 0; i < j; ++i) {
                        merge_bytes(n, v, 1, bytes_to_read);
                        ++n;
                    }
                }
                else { // Normal Chunk?
                    for (auto i = 0; i < j; ++i) {
                        auto v2 = reader.read_bytes(bytes_to_read);
                        merge_bytes(n, v2, 0, bytes_to_read);
                        ++n;
                    }
                }
                break;
            }
            default: {
                throw std::logic_error("Illegal data_type_code encountered: " + std::to_string(header.data_type_code));
            }
        }

        // Finished
        return true;
    }
}

auto kdl::media::image::tga::merge_bytes(const int position, const std::vector<char> bytes, const int offset, const int size) -> void
{
    if (size == 4) {
        m_surface->set(position, graphite::qd::color(bytes[2], bytes[1], bytes[0], bytes[3]));
    }
    else if (size == 3) {
        m_surface->set(position, graphite::qd::color(bytes[2], bytes[1], bytes[0], 255));
    }
    else if (size == 2) {
        m_surface->set(position, graphite::qd::color((bytes[1] & 0x7c) << 1,
                                                     ((bytes[1] & 0x03) << 6) | ((bytes[0] & 0xe0) >> 2),
                                                     (bytes[0] & 0x1f) << 3,
                                                     bytes[1] & 0x80));
    }
}
