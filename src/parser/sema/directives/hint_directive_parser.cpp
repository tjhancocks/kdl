// Copyright (c) 2022 Tom Hancocks
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
#include "parser/sema/directives/hint_directive_parser.hpp"

auto kdl::sema::hint_directive_parser::parse(parser &parser, std::weak_ptr<target> target) -> void
{
    if (target.expired()) {
        throw std::logic_error("Build target has expired. This is a bug!");
    }
    auto t = target.lock();

    parser.ensure({
        expectation(lexeme::directive, "hint").be_true(),
        expectation(lexeme::l_paren).be_true()
    });

    if (!parser.expect_any({
        expectation(lexeme::string).be_true(),
        expectation(lexeme::integer).be_true(),
        expectation(lexeme::percentage).be_true(),
        expectation(lexeme::identifier).be_true()
    })) {
        log::fatal_error(parser.peek(), 1, "Invalid lexeme encountered as value for hint.");
    }
    parser.advance();

    parser.ensure({ expectation(lexeme::r_paren).be_true() });
}
