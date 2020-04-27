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
#include "png.hpp"
#include "lodepng.hpp"

// MARK: - Constructors

kdl::media::image::png::png(const std::string path)
{
    graphite::data::reader reader(path);

    // TODO: Possibly handle any errors that occur?
    decode(reader);
}

kdl::media::image::png::png(std::shared_ptr<std::vector<char>> data)
{
    auto ptr = std::make_shared<graphite::data::data>(data, data->size(), 0, graphite::data::data::lsb);
    graphite::data::reader reader(ptr, 0);
    decode(reader);
}

kdl::media::image::png::png(std::shared_ptr<graphite::qd::surface> surface)
        : m_surface(surface)
{

}

// MARK: - Decoding

auto kdl::media::image::png::decode(graphite::data::reader &reader) -> bool
{
    std::vector<unsigned char> image;
    unsigned int width, height;
    std::shared_ptr<std::vector<char>> data = reader.get().get()->get();

    auto error = lodepng::decode(image, width, height, std::vector<unsigned char>(data->begin(), data->end()));

    if (error) {
        std::cerr << "PNG Decoder: Decode failed with error " << error << std::endl;
        return false;
    }

    // Setup a QuickDraw surface for the image to be read into. The buffer should be completely
    // black by default. This will be the "default" image in the event we fail to read.
    m_surface = std::make_shared<graphite::qd::surface>(width, height);

    for (auto i = 0, offset = 0; i < width * height; i++, offset += 4) {
        m_surface->set(i, graphite::qd::color(image[offset], image[offset + 1], image[offset + 2], image[offset + 3]));
    }

    // Finished
    return true;
}

// MARK: - Encoding

auto kdl::media::image::png::encode(graphite::data::writer &writer) -> void
{
    auto width = m_surface->size().width(), height = m_surface->size().height();
    std::vector<unsigned char> png;
    std::vector<unsigned char> image(width * height * 4);

    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            image[width * y * 4 + x * 4 + 0] = m_surface->at(x, y).red_component();
            image[width * y * 4 + x * 4 + 1] = m_surface->at(x, y).green_component();
            image[width * y * 4 + x * 4 + 2] = m_surface->at(x, y).blue_component();
            image[width * y * 4 + x * 4 + 3] = m_surface->at(x, y).alpha_component();
        }
    }

    auto error = lodepng::encode(png, image, width, height);
    writer.write_bytes(png);

    if (error) {
        std::cerr << "PNG Encoder: Encode failed with error " << error << std::endl;
    }
}

// MARK: - Accessors

auto kdl::media::image::png::surface() -> std::weak_ptr<graphite::qd::surface>
{
    return m_surface;
}

auto kdl::media::image::png::data() -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer writer(data);
    encode(writer);
    return data;
}
