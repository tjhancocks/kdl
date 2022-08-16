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

#include <utility>
#include <stdexcept>
#include "diagnostic/fatal.hpp"
#include "parser/sema/declarations/implicit_value_parser.hpp"
#include "parser/sema/expression/expression_parser.hpp"

// MARK: - Constructor

kdl::sema::implicit_value_parser::implicit_value_parser(kdl::sema::parser &parser,
                                                        std::weak_ptr<target> target,
                                                        kdl::build_target::type_field &field,
                                                        kdl::build_target::type_field_value &field_value,
                                                        kdl::build_target::type_template::binary_field binary_field)
    : m_parser(parser), m_field(field), m_field_value(field_value), m_binary_field(std::move(binary_field))
{
    if (target.expired()) {
        throw std::logic_error("Target has expired. This is a bug.");
    }
    m_target = target.lock();
}

// MARK: - Parser

auto kdl::sema::implicit_value_parser::parse(kdl::build_target::resource_constructor &instance) -> void
{
    // TODO: This function may need evaluating to make expressions work correctly without hundreds of hacks.
    // Validate the type of the next value
    enum lexeme::type field_type;

    // Check if the value coming up in the lexical stream is an expression. If it is then preemptively parse it
    // and push the result into the parser.
    if (m_parser.expect({ expectation(lexeme::l_expr).be_true() })) {
        expression_parser expr(m_parser, m_target, {
            std::make_pair("_id", kdl::lexeme(std::to_string(instance.id()), lexeme::res_id)),
            std::make_pair("_name", kdl::lexeme(instance.name(), lexeme::string))
        });
        m_parser.push({ expr.parse() });
    }

    switch (m_binary_field.type & ~0xFFFUL) {
        case build_target::DBYT:
        case build_target::DWRD:
        case build_target::DLNG:
        case build_target::DQAD:
        case build_target::HBYT:
        case build_target::HWRD:
        case build_target::HLNG:
        case build_target::HQAD: {
            field_type = lexeme::integer;
            if (!m_parser.expect_any({ expectation(lexeme::integer).be_true(), expectation(lexeme::identifier).be_true(), expectation(lexeme::var).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal or symbol for field '" + m_field.name().text() + "'.");
            }
            break;
        }

        case build_target::PSTR:
        case build_target::CSTR:
        case build_target::Cnnn: {
            field_type = lexeme::string;
            if (!m_parser.expect_any({ expectation(lexeme::string).be_true(), expectation(lexeme::identifier).be_true(), expectation(lexeme::var).be_true() })) {
                auto lx = m_parser.peek();
                log::fatal_error(lx, 1, "Expected an string literal or symbol for field '" + m_field.name().text() + "'.");
            }
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
            break;
        }

        case build_target::HEXD: {
            log::fatal_error(m_parser.peek(), 1, "The 'HEXD' type is not directly supported '" + m_field.name().text() + "'.");
        }

        case build_target::INVALID: {
            log::fatal_error(m_parser.peek(), 1, "Unknown type encountered in field '" + m_field.name().text() + "'.");
        }
    }

    if ((m_binary_field.type & ~0xFFFUL) == build_target::RECT) {
        instance.write_rect(m_field, m_field_value,
                            m_parser.read().value<int16_t>(),
                            m_parser.read().value<int16_t>(),
                            m_parser.read().value<int16_t>(),
                            m_parser.read().value<int16_t>());
    }
    else {
        auto value = m_parser.read();

        if (value.is(lexeme::identifier)) {
            auto symbol_value = m_field_value.value_for(value);

            if (!symbol_value.is(field_type)) {
                auto type_desc = field_type == lexeme::string ? "string" : "integer literal";
                log::fatal_error(m_parser.peek(), 1, "The field '" + m_field.name().text() + "' expects a " + type_desc + " valued symbol.");
            }

            value = symbol_value;
        }

        // Read the next value and write it to the resource.
        switch (m_binary_field.type & ~0xFFFUL) {
            case build_target::DBYT: {
                instance.write_signed_byte(m_field, m_field_value, value.value<int8_t>());
                break;
            }
            case build_target::DWRD: {
                instance.write_signed_short(m_field, m_field_value, value.value<int16_t>());
                break;
            }
            case build_target::DLNG: {
                instance.write_signed_long(m_field, m_field_value, value.value<int32_t>());
                break;
            }
            case build_target::DQAD: {
                instance.write_signed_quad(m_field, m_field_value, value.value<int64_t>());
                break;
            }
            case build_target::HBYT: {
                instance.write_byte(m_field, m_field_value, value.value<uint8_t>());
                break;
            }
            case build_target::HWRD: {
                instance.write_short(m_field, m_field_value, value.value<uint16_t>());
                break;
            }
            case build_target::HLNG: {
                instance.write_long(m_field, m_field_value, value.value<uint32_t>());
                break;
            }
            case build_target::HQAD: {
                instance.write_quad(m_field, m_field_value, value.value<uint64_t>());
                break;
            }

            case build_target::PSTR: {
                instance.write_pstr(m_field, m_field_value, value.text());
                break;
            }
            case build_target::CSTR: {
                instance.write_cstr(m_field, m_field_value, value.text());
                break;
            }

            case build_target::Cnnn: {
                instance.write_cstr(m_field, m_field_value, value.text(), m_binary_field.type & 0xFFFUL);
                break;
            }
        }
    }
}
