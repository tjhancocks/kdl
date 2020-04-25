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

// MARK: - Constructor

kdl::media::conversion::conversion(const std::string m_input_file_contents, const kdl::lexeme input,
                                          const kdl::lexeme output)
    : m_input_file_format(input), m_output_file_format(output),
      m_input_file_contents(std::make_shared<std::vector<char>>(m_input_file_contents.begin(), m_input_file_contents.end()))
{
}

// MARK: - Conversion

auto kdl::media::conversion::perform_conversion() const -> std::vector<char>
{
    if ((m_input_file_format.is("TGA") || m_input_file_format.is("PNG")) &&
            (m_output_file_format.is("PICT") || m_output_file_format.is("cicn"))) {
        // Handle image-to-image conversions
        std::shared_ptr<graphite::qd::surface> surface;
        if (m_input_file_format.is("TGA")) {
            image::tga tga(m_input_file_contents);
            surface = tga.surface().lock();
        }
        else if (m_input_file_format.is("PNG")) {
            image::png png(m_input_file_contents);
            surface = png.surface().lock();
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
        else {
            log::fatal_error(m_output_file_format, 1, "Unable to handle output format '" + m_output_file_format.text() + "'");
        }
    }
    else if (m_input_file_format.is("WAV") && m_output_file_format.is("snd")) {
        sound::wav wav(m_input_file_contents);
        graphite::resources::sound snd(wav.sample_rate(), wav.sample_bits(), wav.samples());
        auto snd_data = snd.data();

        return std::vector<char>(snd_data->get()->begin(), snd_data->get()->end());
    }
    else {
        log::fatal_error(m_output_file_format, 1, "Unable to convert from '" + m_input_file_format.text() +
                         "' to '" + m_output_file_format.text() + "'");
    }
}
