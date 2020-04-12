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

#include "parser/sema/util/list_parser.hpp"
#include "parser/sema/type_definition/kdl_type_parser.hpp"

// MARK: - Parser

auto kdl::sema::kdl_type_parser::parse(kdl::sema::parser& parser) -> kdl::build_target::kdl_type
{
    build_target::kdl_type type;

    // Check for a type name.
    if (parser.expect({ expectation(lexeme::identifier).be_true() })) {
        type.set_name(parser.read());

        // Check for associated values / type hints.
        if (parser.expect({ expectation(lexeme::l_angle).be_true() })) {
            list_parser list(parser);
            list.set_list_start(lexeme::l_angle);
            list.set_list_end(lexeme::r_angle);
            list.set_delimiter(lexeme::comma);
            list.add_valid_list_item(lexeme::integer);
            list.add_valid_list_item(lexeme::percentage);
            list.add_valid_list_item(lexeme::res_id);
            list.add_valid_list_item(lexeme::identifier);
            list.add_valid_list_item(lexeme::string);
            list.add_valid_list_item(lexeme::var);
            type.set_type_hints(list.parse());
        }
    }

    // Check for a reference indicator
    if (parser.expect({ expectation(lexeme::amp).be_true() })) {
        parser.advance();
        type.set_reference(true);
    }

    return type;
}
