// Copyright (c) 2019-2022 Tom Hancocks
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
#include "media/image/tga.hpp"

// MARK: - Constructors

kdl::media::image::tga::tga(const std::string& path)
    : m_path(path)
{
    // TARGA images are stored in little endian, so set the byte order accordingly
    graphite::data::block data(m_path, graphite::data::byte_order::lsb);
    graphite::data::reader reader(&data);

    // TODO: Possibly handle any errors that occur?
    decode(reader);
}

kdl::media::image::tga::tga(const graphite::data::block& data)
{
    // Ensure that the reader is going to be interpreting values as Little Endian
    graphite::data::reader reader(&data);
    reader.change_byte_order(graphite::data::byte_order::lsb);
    decode(reader);
}

kdl::media::image::tga::tga(graphite::quickdraw::surface& surface)
    : m_surface(surface)
{
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
    m_surface = graphite::quickdraw::surface(header.width, header.height);

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
    }

    // Finished
    return true;
}

auto kdl::media::image::tga::merge_bytes(std::int32_t position, const std::vector<char>& bytes, std::int32_t offset, std::size_t size) -> void
{
    if (size == 4) {
        m_surface.set(position, graphite::quickdraw::rgb(
            bytes[offset + 2], bytes[offset + 1], bytes[offset], bytes[offset + 3]
        ));
    }
    else if (size == 3) {
        m_surface.set(position, graphite::quickdraw::rgb(
            bytes[offset + 2], bytes[offset + 1], bytes[offset]
        ));
    }
    else if (size == 2) {
        m_surface.set(position, graphite::quickdraw::rgb(
            (bytes[offset + 1] & 0x7c) << 1,
            ((bytes[offset + 1] & 0x03) << 6) | ((bytes[offset + 0] & 0xe0) >> 2),
            (bytes[offset + 0] & 0x1f) << 3,
            bytes[offset + 1] & 0x80
        ));
    }
}

// MARK: - Encoding

auto kdl::media::image::tga::encode(graphite::data::writer &writer) const -> void
{
    // Formulate a TGA header
    tga::header header;
    header.id_length = 0;
    header.color_map_type = 0;
    header.data_type_code = 10;
    header.color_map_origin = 0;
    header.color_map_length = 0;
    header.color_map_depth = 0;
    header.x_origin = 0;
    header.y_origin = 0;
    header.width = m_surface.size().width;
    header.height = m_surface.size().height;
    header.bits_per_pixel = 32;
    header.image_descriptor = 0;

    writer.write_byte(header.id_length);
    writer.write_byte(header.color_map_type);
    writer.write_byte(header.data_type_code);
    writer.write_short(header.color_map_origin);
    writer.write_short(header.color_map_length);
    writer.write_byte(header.color_map_depth);
    writer.write_short(header.x_origin);
    writer.write_short(header.y_origin);
    writer.write_short(header.width);
    writer.write_short(header.height);
    writer.write_byte(header.bits_per_pixel);
    writer.write_byte(header.image_descriptor);

    // Start compressing and writing the image data.
    std::int32_t run = 0;
    std::vector<graphite::quickdraw::color> buffer;

    for (auto y = 0; y < header.height; ++y) {
        for (auto x = 0; x < header.width; ++x) {
            auto picker = m_surface.at(x, header.height - 1 - y);

            if (buffer.size() == 128 || (buffer.size() > 1 && buffer.back() == picker)) {
                auto n = buffer.size() - 1;
                if (buffer.back() == picker) {
                    n--;
                }

                writer.write_byte(n);
                for (auto i = 0; i < n; ++i) {
                    writer.write_byte(buffer[i].components.blue);
                    writer.write_byte(buffer[i].components.green);
                    writer.write_byte(buffer[i].components.red);
                    writer.write_byte(buffer[i].components.alpha);
                }

                if (buffer.back() == picker) {
                    buffer.clear();
                    buffer.emplace_back(picker);
                    run = 2;
                }
            }
            else if (!buffer.empty() && buffer.back() == picker) {
                ++run;

                if (run == 128) {
                    writer.write_byte(0x80 | (run - 1));
                    writer.write_byte(buffer.back().components.blue);
                    writer.write_byte(buffer.back().components.green);
                    writer.write_byte(buffer.back().components.red);
                    writer.write_byte(buffer.back().components.alpha);
                    buffer.clear();
                    run = 0;
                }
            }
            else {
                if (run > 0) {
                    writer.write_byte(0x80 | (run - 1));
                    writer.write_byte(buffer.back().components.blue);
                    writer.write_byte(buffer.back().components.green);
                    writer.write_byte(buffer.back().components.red);
                    writer.write_byte(buffer.back().components.alpha);
                    buffer.clear();
                    run = 0;
                }
                buffer.emplace_back(picker);
                run = 1;
            }

        }
    }
}

// MARK: - Accessors

auto kdl::media::image::tga::surface() -> graphite::quickdraw::surface&
{
    return m_surface;
}

auto kdl::media::image::tga::data() const -> graphite::data::block
{
    graphite::data::writer writer(graphite::data::byte_order::lsb);
    encode(writer);
    return std::move(*const_cast<graphite::data::block *>(writer.data()));
}
