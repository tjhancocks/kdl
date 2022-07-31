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

#include <libGraphite/quickdraw/format/pict.hpp>
#include <libGraphite/quickdraw/format/cicn.hpp>
#include <libGraphite/quickdraw/format/ppat.hpp>
#include <libGraphite/spriteworld/rleD.hpp>
#include <libGraphite/spriteworld/rleX.hpp>
#include <libGraphite/sound/sound.hpp>

#include "diagnostic/fatal.hpp"

// MARK: - Constructors

kdl::media::conversion::conversion(const std::vector<char>& input_file_contents, kdl::lexeme input, kdl::lexeme output)
    : m_input_file_format(input), m_output_file_format(output)
{
    // Convert from std::vector<char> to graphite::data::block
    m_input_file_contents.emplace_back(
        graphite::data::block(input_file_contents, graphite::data::byte_order::lsb)
    );
}

kdl::media::conversion::conversion(kdl::lexeme input, kdl::lexeme output)
    : m_input_file_format(input), m_output_file_format(output)
{
}

kdl::media::conversion::conversion(const graphite::data::block &input_file_contents, lexeme input, lexeme output)
    : m_input_file_contents({ input_file_contents }), m_input_file_format(input), m_output_file_format(output)
{
}

// MARK: - Adding input data

auto kdl::media::conversion::add_input_data(const std::vector<char>& data) -> void
{
    m_input_file_contents.emplace_back(graphite::data::block(data, graphite::data::byte_order::lsb));
}

auto kdl::media::conversion::add_input_data(const graphite::data::block &data) -> void
{
    m_input_file_contents.emplace_back(data);
}

// MARK: - Adding input files

auto kdl::media::conversion::add_input_file(const std::string& contents) -> void
{
    std::vector<char> str(contents.begin(), contents.end());
    m_input_file_contents.emplace_back(graphite::data::block(str, graphite::data::byte_order::lsb));
}

// MARK: - Conversion

static inline auto is_image_type(const kdl::lexeme& type) -> bool
{
    return type.is("PICT")
        || type.is("cicn")
        || type.is("PNG")
        || type.is("TGA")
        || type.is("ppat");
}

auto kdl::media::conversion::perform_conversion() const -> graphite::data::block
{
    if (is_image_type(m_input_file_format) && is_image_type(m_output_file_format)) {
        if (m_input_file_contents.size() != 1) {
            log::fatal_error(m_output_file_format, 1, "Unable to process more than one input file for format '" + m_output_file_format.text() + "'");
        }

        // Handle image-to-image conversions
        graphite::quickdraw::surface surface;
        if (m_input_file_format.is("TGA")) {
            image::tga tga(m_input_file_contents[0]);
            surface = tga.surface();
        }
        else if (m_input_file_format.is("PNG")) {
            image::png png(m_input_file_contents[0]);
            surface = png.surface();
        }
        else if (m_input_file_format.is("PICT")) {
            graphite::quickdraw::pict pict(m_input_file_contents[0]);
            surface = pict.surface();
        }
        else if (m_input_file_format.is("cicn")) {
            graphite::quickdraw::cicn cicn(m_input_file_contents[0]);
            surface = cicn.surface();
        }
        else {
            log::fatal_error(m_input_file_format, 1, "Unable to handle input format '" + m_input_file_format.text() + "'");
        }

        if (m_output_file_format.is("PICT")) {
            graphite::quickdraw::pict pict(surface);
            return std::move(pict.data());
        }
        else if (m_output_file_format.is("cicn")) {
            graphite::quickdraw::cicn cicn(surface);
            return std::move(cicn.data());
        }
        else if (m_output_file_format.is("PNG")) {
            image::png png(surface);
            return std::move(png.data());
        }
        else if (m_output_file_format.is("TGA")) {
            image::tga tga(surface);
            return std::move(tga.data());
        }
        else if (m_output_file_format.is("ppat")) {
            graphite::quickdraw::ppat ppat(surface);
            return std::move(ppat.data());
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
        graphite::sound_manager::sound snd(wav.sample_rate(), wav.sample_bits(), wav.samples());
        return snd.samples();
    }
    else if (is_image_type(m_input_file_format) && m_output_file_format.is("rleD")) {
        if (m_input_file_contents.empty()) {
            log::fatal_error(m_output_file_format, 1, "Must have at least one input file for format '" + m_output_file_format.text() + "'");
        }

        graphite::quickdraw::surface surface;

        // Load the first image to determine the frame size
        if (m_input_file_format.is("TGA")) {
            image::tga tga(m_input_file_contents[0]);
            surface = tga.surface();
        }
        else if (m_input_file_format.is("PNG")) {
            image::png png(m_input_file_contents[0]);
            surface = png.surface();
        }

        auto frame_size = surface.size();
        graphite::spriteworld::rleD rle(surface.size(), m_input_file_contents.size());
        rle.write_frame(0, surface);

        // Load subsequent frames and make sure they're the same size as the first
        for (auto i = 1; i <  m_input_file_contents.size(); i++) {
            if (m_input_file_format.is("TGA")) {
                image::tga tga(m_input_file_contents[i]);
                surface = tga.surface();
            }
            else if (m_input_file_format.is("PNG")) {
                image::png png(m_input_file_contents[i]);
                surface = png.surface();
            }

            if (surface.size().width != frame_size.width || surface.size().height != frame_size.height) {
                log::fatal_error(m_output_file_format, 1, "Frame " + std::to_string(i) + " has incorrect size");
            }

            rle.write_frame(i, surface);
        }

        // Encode the rleD resource
        return std::move(rle.data());
    }
    else if (is_image_type(m_input_file_format) && m_output_file_format.is("rleX")) {
        if (m_input_file_contents.empty()) {
            log::fatal_error(m_output_file_format, 1, "Must have at least one input file for format '" + m_output_file_format.text() + "'");
        }

        graphite::quickdraw::surface surface;

        // Load the first image to determine the frame size
        if (m_input_file_format.is("TGA")) {
            image::tga tga(m_input_file_contents[0]);
            surface = tga.surface();
        }
        else if (m_input_file_format.is("PNG")) {
            image::png png(m_input_file_contents[0]);
            surface = png.surface();
        }

        auto frame_size = surface.size();
        graphite::spriteworld::rleX rle(surface.size(), m_input_file_contents.size());
        rle.write_frame(0, surface);

        // Load subsequent frames and make sure they're the same size as the first
        for (auto i = 1; i <  m_input_file_contents.size(); i++) {
            if (m_input_file_format.is("TGA")) {
                image::tga tga(m_input_file_contents[i]);
                surface = tga.surface();
            }
            else if (m_input_file_format.is("PNG")) {
                image::png png(m_input_file_contents[i]);
                surface = png.surface();
            }

            if (surface.size().width != frame_size.width || surface.size().height != frame_size.height) {
                log::fatal_error(m_output_file_format, 1, "Frame " + std::to_string(i) + " has incorrect size");
            }

            rle.write_frame(i, surface);
        }

        // Encode the rleX resource
        return std::move(rle.data());
    }
    else if (m_input_file_format.is("rleD") && is_image_type(m_output_file_format)) {
        graphite::spriteworld::rleD rle(m_input_file_contents[0]);
        auto surface = rle.surface();

        if (m_output_file_format.is("PNG")) {
            image::png png(surface);
            return std::move(png.data());
        }

        return {};
    }
    else if (m_input_file_format.is("rleX") && is_image_type(m_output_file_format)) {
        graphite::spriteworld::rleX rle(m_input_file_contents[0]);
        auto surface =rle.surface();

        if (m_output_file_format.is("PNG")) {
            image::png png(surface);
            return std::move(png.data());
        }

        return {};
    }
    else {
        log::fatal_error(m_output_file_format, 1, "Unable to convert from '" + m_input_file_format.text() +
                         "' to '" + m_output_file_format.text() + "'");
    }
}
