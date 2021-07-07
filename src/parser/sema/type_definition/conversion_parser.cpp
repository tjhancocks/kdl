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
#include "parser/sema/type_definition/conversion_parser.hpp"

// MARK: - Constructor

kdl::sema::conversion_parser::conversion_parser(kdl::sema::parser &parser, std::weak_ptr<target> target)
    : m_parser(parser)
{
    if (target.expired()) {
        throw std::logic_error("Target has expired. This is a bug.");
    }
    m_target = target.lock();
}

// MARK: - Parser

auto kdl::sema::conversion_parser::parse() -> std::tuple<lexeme, lexeme>
{
    auto conversion_lx = m_parser.peek();
    m_parser.ensure({ expectation(lexeme::identifier, "__conversion").be_true() });

    list_parser list(m_parser, m_target);
    list.set_list_start(lexeme::l_paren);
    list.set_list_end(lexeme::r_paren);
    list.set_delimiter(lexeme::comma);
    list.add_valid_list_item(lexeme::identifier, "PICT");
    list.add_valid_list_item(lexeme::identifier, "TGA");
    list.add_valid_list_item(lexeme::identifier, "PNG");
    list.add_valid_list_item(lexeme::identifier, "cicn");
    list.add_valid_list_item(lexeme::identifier, "rleD");
    list.add_valid_list_item(lexeme::identifier, "ppat");
    list.add_valid_list_item(lexeme::identifier, "WAV");
    list.add_valid_list_item(lexeme::identifier, "snd");
    list.add_valid_list_item(lexeme::var, "InputFormat");
    auto v = list.parse();

    if (v.size() != 2) {
        log::fatal_error(conversion_lx, 1, "A conversion requires two arguments. An input and output.");
    }

    return std::make_tuple(v.at(0), v.at(1));
}
