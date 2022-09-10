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

#include <vector>
#include "parser/sema/expression/argument_parser.hpp"
#include "parser/sema/expression/function_parser.hpp"
#include "parser/sema/expression/expression_parser.hpp"
#include "target/new/kdl_expression.hpp"

auto kdl::sema::argument_parser::parse(parser& parser, std::shared_ptr<target> target, const std::unordered_map<std::string, kdl::lexeme> vars) -> kdl::lexeme
{
    std::vector<lexeme> argument_expression;
    while (!parser.expect_any({ expectation(lexeme::comma).be_true(), expectation(lexeme::r_paren).be_true() })) {
        if (parser.expect({ expectation(lexeme::identifier).be_true(), expectation(lexeme::l_paren).be_true() })) {
            argument_expression.emplace_back(function_parser::parse(parser, target, vars));
        }
        else {
            argument_expression.emplace_back(parser.read());
        }
    }
    return expression_parser::evaluate(target, argument_expression, vars);
}