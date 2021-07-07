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
#include "parser/sema/type_definition/type_definition_parser.hpp"
#include "parser/sema/type_definition/template_parser.hpp"
#include "parser/sema/type_definition/field_definition_parser.hpp"
#include "parser/sema/type_definition/assertion_parser.hpp"

// MARK: - Constructor

kdl::sema::type_definition_parser::type_definition_parser(kdl::sema::parser &parser, std::weak_ptr<target> target)
    : m_parser(parser)
{
    if (target.expired()) {
        throw std::logic_error("Target has expired. This is a bug.");
    }
    m_target = target.lock();
}

// MARK: - Parser

auto kdl::sema::type_definition_parser::parse(bool directive) -> kdl::build_target::type_container
{
    if (directive) {
        m_parser.ensure({ expectation(lexeme::directive, "type").be_true() });
    }

    // Type Name
    if (!m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Type name must be an identifier");
    }
    auto name = m_parser.read();

    m_parser.ensure({ expectation(lexeme::colon).be_true() });

    // Type Code
    if (!m_parser.expect({ expectation(lexeme::string).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Type code must be a string");
    }
    auto code = m_parser.read();

    // Type Definition Body
    build_target::type_container type(name, code.text());
    m_parser.ensure({ expectation(lexeme::l_brace).be_true() });
    while (m_parser.expect({ expectation(lexeme::r_brace).be_false() })) {

        if (m_parser.expect({ expectation(lexeme::identifier, "template").be_true() })) {
            type.set_internal_template(template_parser(m_parser).parse());
        }
        else if (m_parser.expect({ expectation(lexeme::identifier, "assert").be_true() })) {
            type.add_assertions(assertion_parser::parse(m_parser));
        }
        else if (m_parser.expect({ expectation(lexeme::identifier, "field").be_true() })) {
            type.add_field(field_definition_parser(m_parser, m_target, type.internal_template()).parse());
        }
        else {
            log::fatal_error(m_parser.peek(), 1, "Unexpected lexeme found in type definition.");
        }

        m_parser.ensure({ expectation(lexeme::semi).be_true() });
    }
    m_parser.ensure({ expectation(lexeme::r_brace).be_true() });
    return type;
}

