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

#include "diagnostic/fatal.hpp"
#include "parser/sema/declarations/named_types/color_parser.hpp"

// MARK: - Constructor

kdl::sema::color_parser::color_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                      kdl::build_target::type_field_value &field_value,
                                      kdl::build_target::type_template::binary_field binary_field,
                                      kdl::build_target::kdl_type &type)
        : m_parser(parser),
          m_field(field),
          m_field_value(field_value),
          m_binary_field(binary_field),
          m_explicit_type(type)
{

}

// MARK: - Parser

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
static auto __validate_component(const kdl::lexeme value) -> T
{
    auto __v = value.value<T>();
    auto __lV = 0;
    auto __uV = 255;
    if ((__v >= __lV) && (__v <= __uV)) {
        return __v;
    }
    else {
        kdl::log::fatal_error(value, 1, "Specified value '" + value.text() + "' is outside of allowed range '0 ... 255'");
    }
}

auto kdl::sema::color_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    if (m_parser.expect({ expectation(lexeme::integer).be_true() })) {
        // Raw color code
        instance.write_long(m_field, m_field_value, m_parser.read().value<uint32_t>());
    }
    else if (m_parser.expect({
        expectation(lexeme::identifier, "rgb").be_true(), expectation(lexeme::l_paren).be_true(),
        expectation(lexeme::integer).be_true(), expectation(lexeme::comma).be_true(),
        expectation(lexeme::integer).be_true(), expectation(lexeme::comma).be_true(),
        expectation(lexeme::integer).be_true(), expectation(lexeme::r_paren).be_true()
    })) {
        // RGB Color Function
        m_parser.advance(2);
        auto r = m_parser.read().value<uint8_t>();
        m_parser.advance();
        auto g = m_parser.read().value<uint8_t>();
        m_parser.advance();
        auto b = m_parser.read().value<uint8_t>();
        m_parser.advance();

        uint32_t color = (r << 16) | (g << 8) | b;
        instance.write_long(m_field, m_field_value, color);
    }
    else {
        kdl::log::fatal_error(m_parser.peek(), 1, "Unexpected lexeme encountered: '" + m_parser.peek().text() + "'");
    }
}