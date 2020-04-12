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
#include "parser/sema/declarations/unnamed_reference_value_parser.hpp"

// MARK: - Constructor

kdl::sema::unnamed_reference_value_parser::unnamed_reference_value_parser(kdl::sema::parser &parser,
                                                                          build_target::type_field& field,
                                                                          build_target::type_field_value& field_value,
                                                                          build_target::type_template::binary_field binary_field,
                                                                          kdl::build_target::kdl_type &type)
    : m_parser(parser), m_explicit_type(type), m_field(field), m_binary_field(binary_field), m_field_value(field_value)
{

}

// MARK: - Parser

auto kdl::sema::unnamed_reference_value_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    if (!m_parser.expect({ expectation(lexeme::res_id).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "The field '" + m_field.name().text() + "' expects as a resource id.");
    }
    auto ref = m_parser.read();

    // Ensure that the underlying type is correct for a reference.
    switch (m_binary_field.type & ~0xFFF) {
        case build_target::DWRD: {
            instance.write_signed_short(m_field, m_field_value, ref.value<int16_t>());
            break;
        }
        case build_target::DLNG: {
            instance.write_signed_long(m_field, m_field_value, ref.value<int32_t>());
            break;
        }
        case build_target::DQAD: {
            instance.write_signed_quad(m_field, m_field_value, ref.value<int64_t>());
            break;
        }
        default: {
            log::fatal_error(m_field.name(), 1, "Resource reference value should be backed by either a DWRD, DLNG or DQAD");
        }
    }
}


