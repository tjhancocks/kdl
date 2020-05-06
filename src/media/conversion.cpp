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

#include "media/conversion.hpp"
#include "media/image/tga.hpp"
#include "media/image/png.hpp"
#include "media/sound/wav.hpp"
#include "libGraphite/quickdraw/pict.hpp"
#include "libGraphite/quickdraw/cicn.hpp"
#include "libGraphite/quickdraw/rle.hpp"
#include "libGraphite/resources/sound.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Constructors

kdl::media::conversion::conversion(const std::string m_input_file_contents, const kdl::lexeme input,
                                          const kdl::lexeme output)
    : m_input_file_format(input), m_output_file_format(output),
      m_input_file_contents({ std::make_shared<std::vector<char>>(m_input_file_contents.begin(), m_input_file_contents.end()) })
{
}

kdl::media::conversion::conversion(const kdl::lexeme input, const kdl::lexeme output)
    : m_input_file_format(input), m_output_file_format(output)
{
}

// MARK: - Adding input data

auto kdl::media::conversion::add_input_data(std::vector<char> data) -> void
{
    m_input_file_contents.emplace_back(std::make_shared<std::vector<char>>(data));
}

// MARK: - Adding input files

auto kdl::media::conversion::add_input_file(const std::string contents) -> void
{
    m_input_file_contents.emplace_back(std::make_shared<std::vector<char>>(contents.begin(), contents.end()));
}

// MARK: - Conversion

static inline auto is_image_type(const kdl::lexeme& type) -> bool
{
    return type.is("PICT") || type.is("cicn") || type.is("PNG") || type.is("TGA");
}

auto kdl::media::conversion::perform_conversion() const -> std::vector<char>
{
    if (is_image_type(m_input_file_format) && is_image_type(m_output_file_format)) {
        if (m_input_file_contents.size() != 1) {
            log::fatal_error(m_output_file_format, 1, "Unable to process more than one input file for format '" + m_output_file_format.text() + "'");
        }

        // Handle image-to-image conversions
        std::shared_ptr<graphite::qd::surface> surface;
        if (m_input_file_format.is("TGA")) {
            image::tga tga(m_input_file_contents[0]);
            surface = tga.surface().lock();
        }
        else if (m_input_file_format.is("PNG")) {
            image::png png(m_input_file_contents[0]);
            surface = png.surface().lock();
        }
        else if (m_input_file_format.is("PICT")) {
            auto data = m_input_file_contents[0];
            auto data_ptr = std::make_shared<graphite::data::data>(data, data->size());
            graphite::qd::pict pict(data_ptr);
            surface = pict.image_surface().lock();
        }
        else if (m_input_file_format.is("cicn")) {
            auto data = m_input_file_contents[0];
            auto data_ptr = std::make_shared<graphite::data::data>(data, data->size());
            graphite::qd::cicn cicn(data_ptr);
            surface = cicn.surface().lock();
        }
        else {
            log::fatal_error(m_input_file_format, 1, "Unable to handle input format '" + m_input_file_format.text() + "'");
        }

        if (m_output_file_format.is("PICT")) {
            graphite::qd::pict pict(surface);
            auto pict_data = pict.data();

            return std::vector<char>(pict_data->get()->begin(), pict_data->get()->end());
        }
        else if (m_output_file_format.is("cicn")) {
            graphite::qd::cicn cicn(surface);
            auto cicn_data = cicn.data();

            return std::vector<char>(cicn_data->get()->begin(), cicn_data->get()->end());
        }
        else if (m_output_file_format.is("PNG")) {
            image::png png(surface);
            auto png_data = png.data();
            return std::vector<char>(png_data->get()->begin(), png_data->get()->end());
        }
        else {
            log::fatal_error(m_output_file_format, 1, "Unable to handle output format '" + m_output_file_format.text() + "'");
        }
    }
    else if (m_input_file_format.is("WAV") && m_output_file_format.is("snd")) {
        if (m_input_file_contents.size() != 1) {
            log::fatal_error(m_output_file_format, 1, "Unable to process more than one input file for format '" + m_output_file_format.text() + "'");
        }

        sound::wav wav(m_input_file_contents[0]);
        graphite::resources::sound snd(wav.sample_rate(), wav.sample_bits(), wav.samples());
        auto snd_data = snd.data();

        return std::vector<char>(snd_data->get()->begin(), snd_data->get()->end());
    }
    else if (is_image_type(m_input_file_format) && m_output_file_format.is("rleD")) {
        if (m_input_file_contents.size() == 0) {
            log::fatal_error(m_output_file_format, 1, "Must have at least one input file for format '" + m_output_file_format.text() + "'");
        }

        std::shared_ptr<graphite::qd::surface> surface;

        // Load the first image to determine the frame size
        if (m_input_file_format.is("TGA")) {
            image::tga tga(m_input_file_contents[0]);
            surface = tga.surface().lock();
        }
        else if (m_input_file_format.is("PNG")) {
            image::png png(m_input_file_contents[0]);
            surface = png.surface().lock();
        }
        auto frame_size = surface->size();
        graphite::qd::rle rle(frame_size, m_input_file_contents.size());
        rle.write_frame(0, surface);

        // Load subsequent frames and make sure they're the same size as the first
        for (auto i = 1; i <  m_input_file_contents.size(); i++) {
            if (m_input_file_format.is("TGA")) {
                image::tga tga(m_input_file_contents[i]);
                surface = tga.surface().lock();
            }
            else if (m_input_file_format.is("PNG")) {
                image::png png(m_input_file_contents[i]);
                surface = png.surface().lock();
            }

            if (surface->size().width() != frame_size.width() ||
                    surface->size().height() != frame_size.height()) {
                log::fatal_error(m_output_file_format, 1, "Frame " + std::to_string(i) + " has incorrect size");
            }

            rle.write_frame(i, surface);
        }

        // Encode the rleD resource
        auto rled_data = rle.data();
        return std::vector<char>(rled_data->get()->begin(), rled_data->get()->end());
    }
    else if (m_input_file_format.is("rleD") && is_image_type(m_output_file_format)) {
        auto data = m_input_file_contents[0];
        auto data_ptr = std::make_shared<graphite::data::data>(data, data->size());
        graphite::qd::rle rle(data_ptr);
        if (auto surface = rle.surface().lock()) {

            if (m_output_file_format.is("PNG")) {
                image::png png(surface);
                auto png_data = png.data();
                return std::vector<char>(png_data->get()->begin(), png_data->get()->end());
            }

            return {};
        }
    }
    else {
        return {};
        log::fatal_error(m_output_file_format, 1, "Unable to convert from '" + m_input_file_format.text() +
                         "' to '" + m_output_file_format.text() + "'");
    }
}
