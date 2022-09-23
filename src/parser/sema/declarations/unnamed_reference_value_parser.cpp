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
#include "parser/sema/declarations/unnamed_reference_value_parser.hpp"
#include "parser/sema/expression/expression_parser.hpp"
#include "parser/sema/expression/variable_parser.hpp"
#include "parser/sema/expression/function_parser.hpp"

// MARK: - Constructor

kdl::sema::unnamed_reference_value_parser::unnamed_reference_value_parser(kdl::sema::parser &parser,
                                                                          std::weak_ptr<target> target,
                                                                          build_target::type_field& field,
                                                                          build_target::type_field_value& field_value,
                                                                          build_target::type_template::binary_field binary_field,
                                                                          kdl::build_target::kdl_type &type)
    : m_parser(parser), m_explicit_type(type), m_field(field), m_binary_field(std::move(binary_field)), m_field_value(field_value)
{
    if (target.expired()) {
        throw std::logic_error("Target has expired. This is a bug.");
    }
    m_target = target.lock();
}

// MARK: - Parser

auto kdl::sema::unnamed_reference_value_parser::parse(kdl::build_target::resource_constructor &instance) -> void
{
    if (m_parser.expect({
        expectation(lexeme::identifier).be_true(),
        expectation(lexeme::l_paren).be_true()
    })) {
        m_parser.push({
            function_parser::parse(m_parser, m_target, {
                std::pair("id", kdl::lexeme(std::to_string(instance.id()), lexeme::res_id)),
                std::pair("name", kdl::lexeme(instance.name(), lexeme::string))
            })
        });
    }
    else if (m_parser.expect({
        expectation(lexeme::var).be_true()
    })) {
        m_parser.push({
           variable_parser::parse(m_parser, m_target, {})
        });
    }

    if (!m_parser.expect_any({
        expectation(lexeme::identifier).be_true(),
        expectation(lexeme::res_id).be_true(),
        expectation(lexeme::l_expr).be_true()
    })) {
        log::fatal_error(m_parser.peek(), 1, "The field '" + m_field.name().text() + "' expects a symbol, resource id or expression.");
    }

    auto ref = m_parser.peek();
    if (ref.is(lexeme::identifier)) {
        ref = m_parser.read();
        auto symbol_value = m_field_value.value_for(ref);

        if (symbol_value.is(lexeme::identifier, "new")) {
            log::fatal_error(m_parser.peek(), 1, "You can not use nested resources on unnamed reference types.");
        }
        else if (!symbol_value.is(lexeme::res_id)) {
            log::fatal_error(m_parser.peek(), 1, "The field '" + m_field.name().text() + "' expects a resource id valued symbol.");
        }

        ref = symbol_value;
    }
    else if (ref.is(lexeme::res_id)) {
        ref = m_parser.read();
    }
    else if (ref.is(lexeme::l_expr)) {
        auto expr = expression_parser::extract(m_parser);
        ref = expr->evaluate(m_target, {}, {
            std::pair("id", kdl::lexeme(std::to_string(instance.id()), lexeme::res_id)),
            std::pair("name", kdl::lexeme(instance.name(), lexeme::string))
        });
    }
    else if (m_parser.expect({ expectation(lexeme::var).be_true() })) {
        m_parser.push({
            variable_parser::parse(m_parser, m_target, {
                std::pair("id", kdl::lexeme(std::to_string(instance.id()), lexeme::res_id)),
                std::pair("name", kdl::lexeme(instance.name(), lexeme::string))
            })
        });
    }
    else {
        // Make sure we read the lexeme from the input stream.
        m_parser.advance();
    }

    // Ensure that the underlying type is correct for a reference.
    switch (m_binary_field.type & ~0xFFFUL) {
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
        case build_target::RSRC: {
            instance.write_resource_reference(m_field, m_field_value, ref);
            break;
        }
        default: {
            log::fatal_error(m_field.name(), 1, "Resource reference value should be backed by either a DWRD, DLNG or DQAD");
        }
    }
}


