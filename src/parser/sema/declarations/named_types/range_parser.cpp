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

#include <stdexcept>
#include <utility>
#include "diagnostic/fatal.hpp"
#include "parser/sema/declarations/named_types/range_parser.hpp"

// MARK: - Constructor

kdl::sema::range_parser::range_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                      kdl::build_target::type_field_value &field_value,
                                      kdl::build_target::type_template::binary_field binary_field,
                                      kdl::build_target::kdl_type &type)
    : m_parser(parser),
      m_field(field),
      m_field_value(field_value),
      m_binary_field(std::move(binary_field)),
      m_explicit_type(type)
{

}

// MARK: - Parser

template<typename T>
static auto validate_range(const kdl::lexeme& value, const kdl::lexeme& lower, const kdl::lexeme& upper) -> T
{
    auto v = value.value<T>();
    auto lV = lower.value<T>();
    auto uV = upper.value<T>();
    if ((v >= lV) && (v <= uV)) {
        return v;
    }
    else {
        kdl::log::fatal_error(value, 1, "Specified value '" + value.text() + "' is outside of allowed range '" + lower.text() + " ... " + upper.text() + "'");
    }
}

auto kdl::sema::range_parser::parse(kdl::build_target::resource_constructor &instance) -> void
{
    // A range requires two values to be provided.
    if (m_explicit_type.type_hints().size() != 2) {
        log::fatal_error(m_field_value.base_name(), 1, "The 'Range' type requires an upper and a lower bound to be provided.");
    }

    auto lower = m_explicit_type.type_hints()[0];
    auto upper = m_explicit_type.type_hints()[1];

    if (!(lower.is(lexeme::integer) || lower.is(lexeme::percentage) || lower.is(lexeme::res_id))) {
        log::fatal_error(lower, 1, "Lower bound of 'Range' type must be a numeric value.");
    }
    if (!(upper.is(lexeme::integer) || upper.is(lexeme::percentage) || upper.is(lexeme::res_id))) {
        log::fatal_error(lower, 1, "Upper bound of 'Range' type must be a numeric value.");
    }
    if (lower.type() != upper.type()) {
        log::fatal_error(lower, 1, "Lower and Upper bound of 'Range' type must be of the same type.");
    }

    // Get the value - we need to know the backing store so that we know what type of integer to work with.
    if (!m_parser.expect({ expectation(lower.type()).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Incorrect value type provided for field '" + m_field.name().text() + "'");
    }

    switch (m_binary_field.type & ~0xFFFUL) {
        case build_target::DBYT: {
            instance.write_signed_byte(m_field, m_field_value, validate_range<int8_t>(m_parser.read(), lower, upper));
            break;
        }
        case build_target::DWRD: {
            instance.write_signed_short(m_field, m_field_value, validate_range<int16_t>(m_parser.read(), lower, upper));
            break;
        }
        case build_target::DLNG: {
            instance.write_signed_long(m_field, m_field_value, validate_range<int32_t>(m_parser.read(), lower, upper));
            break;
        }
        case build_target::DQAD: {
            instance.write_signed_quad(m_field, m_field_value, validate_range<int64_t>(m_parser.read(), lower, upper));
            break;
        }
        case build_target::HBYT: {
            instance.write_byte(m_field, m_field_value, validate_range<uint8_t>(m_parser.read(), lower, upper));
            break;
        }
        case build_target::HWRD: {
            instance.write_short(m_field, m_field_value, validate_range<uint16_t>(m_parser.read(), lower, upper));
            break;
        }
        case build_target::HLNG: {
            instance.write_long(m_field, m_field_value, validate_range<uint32_t>(m_parser.read(), lower, upper));
            break;
        }
        case build_target::HQAD: {
            instance.write_quad(m_field, m_field_value, validate_range<uint64_t>(m_parser.read(), lower, upper));
            break;
        }
        default: {
            throw std::logic_error("Attempting to handle 'Range' type with a none numeric backing type.");
        }
    }
}