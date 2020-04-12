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

#include "parser/sema/type_definition/template_parser.hpp"
#include "parser/sema/type_definition/binary_field.hpp"

// MARK: - Constructor

kdl::sema::template_parser::template_parser(kdl::sema::parser &parser)
    : m_parser(parser)
{

}

// MARK: - Template Parsing

auto kdl::sema::template_parser::parse() -> kdl::build_target::type_template
{
    kdl::build_target::type_template tmpl;

    m_parser.ensure({
        expectation(lexeme::identifier, "template").be_true(),
        expectation(lexeme::l_brace).be_true()
    });

    binary_field field_parser(m_parser);
    while (m_parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        tmpl.add_binary_field(field_parser.parse());
        m_parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    m_parser.ensure({
        expectation(lexeme::r_brace).be_true()
    });

    return tmpl;
}