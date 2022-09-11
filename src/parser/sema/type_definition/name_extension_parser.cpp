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
#include "diagnostic/fatal.hpp"
#include "parser/sema/util/list_parser.hpp"
#include "parser/sema/type_definition/name_extension_parser.hpp"

auto kdl::sema::name_extension_parser::parse(kdl::sema::parser &parser, std::weak_ptr<target> target) -> std::vector<lexeme>
{
    list_parser list(parser, target);
    list.set_list_start(lexeme::l_angle);
    list.set_list_end(lexeme::r_angle);
    list.set_delimiter(lexeme::comma);
    list.add_valid_list_item(lexeme::var);
    return list.parse({
        std::pair("FieldNumber", lexeme("FieldNumber", lexeme::var)) // TODO: Do this properly... this is a hack
    });
}
