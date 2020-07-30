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
#include "parser/sema/type_definition/value_reference_parser.hpp"
#include "parser/sema/type_definition/name_extension_parser.hpp"
#include "parser/sema/type_definition/kdl_type_parser.hpp"
#include "parser/sema/type_definition/symbol_list_parser.hpp"
#include "parser/sema/type_definition/conversion_parser.hpp"

// MARK: - Constructor

kdl::sema::value_reference_parser::value_reference_parser(kdl::sema::parser &parser, build_target::type_template tmpl)
    : m_parser(parser), m_tmpl(tmpl)
{

}

// MARK: - Parser

auto kdl::sema::value_reference_parser::parse() -> kdl::build_target::type_field_value
{
    if (!m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Expected an identifier to define a field value.");
    }
    auto tmpl_field_base_name = m_parser.read();
    build_target::type_field_value ref(tmpl_field_base_name);

    // Check for any name extensions.
    if (m_parser.expect({ expectation(lexeme::l_angle).be_true() })) {
        ref.set_name_extensions(name_extension_parser::parse(m_parser));
    }

    // Check for an explicit type.
    if (m_parser.expect({ expectation(lexeme::identifier, "as").be_true() })) {
        m_parser.advance();
        ref.set_explicit_type(kdl_type_parser::parse(m_parser));
    }

    // Check for a default value.
    if (m_parser.expect({ expectation(lexeme::equals).be_true() })) {
        m_parser.advance();

        // TODO: Perform type checking here...

        ref.set_default_value(m_parser.read());
    }

    // Check for a type/data conversion.
    if (m_parser.expect({ expectation(lexeme::identifier, "__conversion").be_true() })) {
        ref.set_conversion_map(conversion_parser(m_parser).parse());
    }

    // Check for the sprite sheet assembler function
    if (m_parser.expect({ expectation(lexeme::identifier, "__assemble_sprite_sheet").be_true() })) {
        ref.set_assemble_sprite_sheet();
        m_parser.advance();
    }

    // Check for a symbol list.
    if (m_parser.expect({ expectation(lexeme::l_bracket).be_true() })) {
        ref.set_symbols(symbol_list_parser::parse(m_parser));
    }

    return ref;
}
