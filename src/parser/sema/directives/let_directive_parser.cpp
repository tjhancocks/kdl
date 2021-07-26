// Copyright (c) 2021 Tom Hancocks
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
#include <iostream>
#include "diagnostic/fatal.hpp"
#include "parser/sema/directives/let_directive_parser.hpp"
#include "parser/sema/expression/expression_parser.hpp"

auto kdl::sema::let_directive_parser::parse(kdl::sema::parser &parser, std::weak_ptr<target> target) -> void
{
    if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
        log::fatal_error(parser.peek(), 1, "Variable name must be an identifier.");
    }
    auto var_name = parser.read();

    parser.ensure({ expectation(lexeme::equals).be_true() });

    if (!parser.expect_any({
        expectation(lexeme::string).be_true(),
        expectation(lexeme::res_id).be_true(),
        expectation(lexeme::integer).be_true(),
        expectation(lexeme::l_expr).be_true()
    })) {
        log::fatal_error(parser.peek(), 1, "Variable value must be an expression, an integer, string or resource id literal.");
    }

    // We need to pass the variable into the target as a global now.
    if (target.expired()) {
        throw std::logic_error("Build target has expired. This is a bug!");
    }
    auto t = target.lock();

    if (parser.peek().is(lexeme::l_expr)) {
        // We're dealing with an expression that needs evaluating into a single value before assigning the variable.
        expression_parser expr(parser, target, {});
        t->set_global_variable(var_name.text(), expr.parse());
    }
    else {
        t->set_global_variable(var_name.text(), parser.read());
    }

}
