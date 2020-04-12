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
#include "parser/sema/type_definition/symbol_list_parser.hpp"

// MARK: - Parser

auto kdl::sema::symbol_list_parser::parse(kdl::sema::parser &parser) -> std::vector<std::tuple<lexeme, lexeme>>
{
    std::vector<std::tuple<lexeme, lexeme>> symbols;
    parser.ensure({ expectation(lexeme::l_bracket).be_true() });

    while (parser.expect({ expectation(lexeme::r_bracket).be_false() })) {
        if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
            log::fatal_error(parser.peek(), 1, "Symbol name must be an identifier.");
        }
        auto symbol = parser.read();

        parser.ensure({ expectation(lexeme::equals).be_true() });

        if (parser.expect_any({
            expectation(lexeme::integer).be_true(), expectation(lexeme::percentage).be_true(),
            expectation(lexeme::res_id).be_true(), expectation(lexeme::string).be_true()
        })) {
            symbols.emplace_back(std::make_tuple(symbol, parser.read()));
        }
        else {
            log::fatal_error(parser.peek(), 1, "Symbol value must be a string, integer, resource id or percentage.");
        }

        if (parser.expect({ expectation(lexeme::r_bracket).be_false() })) {
            parser.ensure({ expectation(lexeme::comma).be_true() });
        }
    }

    parser.ensure({ expectation(lexeme::r_bracket).be_true() });
    return symbols;
}
