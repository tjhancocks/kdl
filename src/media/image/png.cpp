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
#include "media/image/png.hpp"
#include "media/image/lodepng.hpp"

// MARK: - Constructors

kdl::media::image::png::png(const std::string& path)
    : m_path(path)
{
    // TODO: Double check that this actually wants to be MSB?
    graphite::data::block data(m_path, graphite::data::byte_order::msb);
    graphite::data::reader reader(&data);

    // TODO: Possibly handle any errors that occur?
    decode(reader);
}

kdl::media::image::png::png(const graphite::data::block& data)
{
    graphite::data::reader reader(&data);
    reader.change_byte_order(graphite::data::byte_order::msb);
    decode(reader);
}

kdl::media::image::png::png(graphite::quickdraw::surface& surface)
    : m_surface(surface)
{
}

// MARK: - Decoding

auto kdl::media::image::png::decode(graphite::data::reader &reader) -> bool
{
    std::vector<unsigned char> image;
    unsigned int width, height;

    auto bytes = reader.read_bytes(reader.size());
    auto error = lodepng::decode(image, width, height, std::vector<unsigned char>(bytes.begin(), bytes.end()));

    if (error) {
        std::cerr << "PNG Decoder: Decode failed with error " << error << std::endl;
        return false;
    }

    // Setup a QuickDraw surface for the image to be read into. The buffer should be completely
    // black by default. This will be the "default" image in the event we fail to read.
    m_surface = graphite::quickdraw::surface(width, height);

    for (auto i = 0, offset = 0; i < width * height; i++, offset += 4) {
        m_surface.set(i, graphite::quickdraw::rgb(
            image[offset], image[offset + 1], image[offset + 2], image[offset + 3]
        ));
    }

    // Finished
    return true;
}

// MARK: - Encoding

auto kdl::media::image::png::encode(graphite::data::writer &writer) const -> void
{
    auto size = m_surface.size();
    std::vector<unsigned char> png;
    std::vector<unsigned char> image(size.width, size.height * 4);

    for (auto y = 0; y < size.height; y++) {
        for (auto x = 0; x < size.width; x++) {
            image[size.width * y * 4 + x * 4 + 0] = m_surface.at(x, y).components.red;
            image[size.width * y * 4 + x * 4 + 1] = m_surface.at(x, y).components.green;
            image[size.width * y * 4 + x * 4 + 2] = m_surface.at(x, y).components.blue;
            image[size.width * y * 4 + x * 4 + 3] = m_surface.at(x, y).components.alpha;
        }
    }

    auto error = lodepng::encode(png, image, size.width, size.height);
    writer.write_bytes(png);

    if (error) {
        std::cerr << "PNG Encoder: Encode failed with error " << error << std::endl;
    }
}

// MARK: - Accessors

auto kdl::media::image::png::surface() -> graphite::quickdraw::surface&
{
    return m_surface;
}

auto kdl::media::image::png::data() const -> graphite::data::block
{
    graphite::data::writer writer(graphite::data::byte_order::msb);
    encode(writer);
    return std::move(*const_cast<graphite::data::block *>(writer.data()));
}
