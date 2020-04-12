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
#include "parser/sema/declarations/implicit_value_parser.hpp"

// MARK: - Constructor

kdl::sema::implicit_value_parser::implicit_value_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                                        kdl::build_target::type_field_value &field_value,
                                                        kdl::build_target::type_template::binary_field binary_field)
    : m_parser(parser), m_field(field), m_field_value(field_value), m_binary_field(binary_field)
{

}

// MARK: - Parser

auto kdl::sema::implicit_value_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    // Read the next value and write it to the resource.
    switch (m_binary_field.type & ~0xFFF) {
        case build_target::DBYT: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_signed_byte(m_field, m_field_value, m_parser.read().value<int8_t>());
            break;
        }
        case build_target::DWRD: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_signed_short(m_field, m_field_value, m_parser.read().value<int16_t>());
            break;
        }
        case build_target::DLNG: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_signed_long(m_field, m_field_value, m_parser.read().value<int32_t>());
            break;
        }
        case build_target::DQAD: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_signed_quad(m_field, m_field_value, m_parser.read().value<int64_t>());
            break;
        }
        case build_target::HBYT: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_byte(m_field, m_field_value, m_parser.read().value<uint8_t>());
            break;
        }
        case build_target::HWRD: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_short(m_field, m_field_value, m_parser.read().value<uint16_t>());
            break;
        }
        case build_target::HLNG: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_long(m_field, m_field_value, m_parser.read().value<uint32_t>());
            break;
        }
        case build_target::HQAD: {
            if (!m_parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_quad(m_field, m_field_value, m_parser.read().value<uint64_t>());
            break;
        }

        case build_target::PSTR: {
            if (!m_parser.expect({ expectation(lexeme::string).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an string literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_pstr(m_field, m_field_value, m_parser.read().text());
            break;
        }
        case build_target::CSTR: {
            if (!m_parser.expect({ expectation(lexeme::string).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an string literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_cstr(m_field, m_field_value, m_parser.read().text());
            break;
        }

        case build_target::Cnnn: {
            if (!m_parser.expect({ expectation(lexeme::string).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an string literal for field '" + m_field.name().text() + "'.");
            }
            instance.write_cstr(m_field, m_field_value, m_parser.read().text(), m_binary_field.type & 0xFFF);
            break;
        }

        case build_target::RECT: {
            if (!m_parser.expect({
                                       expectation(lexeme::integer).be_true(),
                                       expectation(lexeme::integer).be_true(),
                                       expectation(lexeme::integer).be_true(),
                                       expectation(lexeme::integer).be_true()
                               })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected 4 integer literals for field '" + m_field.name().text() + "'.");
            }
            instance.write_rect(m_field, m_field_value,
                                m_parser.read().value<int16_t>(),
                                m_parser.read().value<int16_t>(),
                                m_parser.read().value<int16_t>(),
                                m_parser.read().value<int16_t>());
            break;
        }

        case build_target::HEXD: {
            log::fatal_error(m_parser.peek(), 1, "The 'HEXD' type is not directly supported '" + m_field.name().text() + "'.");
        }

        case build_target::INVALID: {
            log::fatal_error(m_parser.peek(), 1, "Unknown type encountered in field '" + m_field.name().text() + "'.");
        }
    }
}