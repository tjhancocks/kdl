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
#include "libGraphite/quickdraw/pict.hpp"
#include "libGraphite/quickdraw/cicn.hpp"
#include "libGraphite/quickdraw/rle.hpp"
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
    // TGA -> PICT
    if (m_input_file_format.is("TGA") && m_output_file_format.is("PICT")) {
        return __tga_to_pict();
    }
    // TGA -> cicn
    else if (m_input_file_format.is("TGA") && m_output_file_format.is("cicn")) {
        return __tga_to_cicn();
    }

    log::fatal_error(m_input_file_format, 1, "Unable to perform conversion between '" + m_input_file_format.text() + "' and '" + m_output_file_format.text() + "'");
}

// MARK: - Private Converters.

auto kdl::media::conversion::__tga_to_pict() const -> std::vector<char>
{
    image::tga tga(m_input_file_contents);
    auto tga_surface = tga.surface().lock();

    graphite::qd::pict pict(tga_surface);
    auto pict_data = pict.data();

    return std::vector<char>(pict_data->get()->begin(), pict_data->get()->end());
}

auto kdl::media::conversion::__tga_to_cicn() const -> std::vector<char>
{
    image::tga tga(m_input_file_contents);
    auto tga_surface = tga.surface().lock();

    graphite::qd::cicn cicn(tga_surface);
    auto cicn_data = cicn.data();

    return std::vector<char>(cicn_data->get()->begin(), cicn_data->get()->end());
}
