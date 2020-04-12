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
#include "parser/sema/type_definition/field_definition_parser.hpp"
#include "parser/sema/type_definition/value_reference_parser.hpp"
#include "parser/sema/util/list_parser.hpp"

// MARK: - Constructor

kdl::sema::field_definition_parser::field_definition_parser(kdl::sema::parser &parser, kdl::build_target::type_template tmpl)
    : m_parser(parser), m_tmpl(tmpl)
{

}

// MARK: - Parser

auto kdl::sema::field_definition_parser::parse() -> kdl::build_target::type_field
{
    // Field name.
    m_parser.ensure({
        expectation(lexeme::identifier, "field").be_true(),
        expectation(lexeme::l_paren).be_true()
    });
    if (!m_parser.expect({ expectation(lexeme::string).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Field name must be a string");
    }
    kdl::build_target::type_field field(m_parser.read());
    m_parser.ensure({ expectation(lexeme::r_paren).be_true() });

    // Does the body specify a repeatable?
    if (m_parser.expect({ expectation(lexeme::identifier, "repeatable").be_true() })) {
        auto lx = m_parser.read();

        list_parser list(m_parser);
        list.set_list_start(lexeme::l_angle);
        list.set_list_end(lexeme::r_angle);
        list.set_delimiter(lexeme::comma);
        list.add_valid_list_item(lexeme::integer);
        auto items = list.parse();

        if (items.size() == 2) {
            field.make_repeatable(items[0].value<int>(), items[1].value<int>());
        }
        else if (items.size() == 1) {
            field.make_repeatable(0, items[0].value<int>());
        }
        else {
            log::fatal_error(lx, 1, "Field repeatable clause has incorrect number of arguments.");
        }
    }

    // The body of the field is enclosed in '{' braces '}'
    value_reference_parser field_value_parser(m_parser, m_tmpl);

    m_parser.ensure({ expectation(lexeme::l_brace).be_true() });
    while (m_parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        field.add_value(field_value_parser.parse());
        m_parser.ensure({ expectation(lexeme::semi).be_true() });
    }
    m_parser.ensure({ expectation(lexeme::r_brace).be_true() });

    return field;
}
