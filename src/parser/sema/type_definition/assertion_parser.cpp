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
#include "parser/sema/type_definition/assertion_parser.hpp"

auto kdl::sema::assertion_parser::parse(parser &parser) -> std::vector<kdl::assertion>
{
    std::vector<kdl::assertion> assertions;
    parser.ensure({
        expectation(lexeme::identifier, "assert").be_true(),
        expectation(lexeme::l_paren).be_true()
    });

    while (parser.expect({ expectation(lexeme::r_paren).be_false() })) {

        // Either side of an assertion operation should be a literal or a variable.
        if (!parser.expect_any({
                expectation(lexeme::var).be_true(),
                expectation(lexeme::integer).be_true(), expectation(lexeme::res_id).be_true(), expectation(lexeme::percentage).be_true(),
                expectation(lexeme::string).be_true()
        })) {
            log::fatal_error(parser.peek(), 1, "Expected either a variable, string literal or a numeric literal.");
        }
        auto lhs = parser.read();

        // The operator may consist of multiple lexemes. Merge them.
        std::string op;
        auto op_first_lx = parser.peek();
        while (parser.expect_any({
                 expectation(lexeme::l_angle).be_true(), expectation(lexeme::r_angle).be_true(),
                 expectation(lexeme::equals).be_true(), expectation(lexeme::exclaim).be_true(),
         })) {
            op.append(parser.read().text());
        }

        // Validate the operator.
        if (op != "<" && op != "<=" && op != "==" && op != "!=" && op != ">=" && op != ">") {
            log::fatal_error(op_first_lx, 1, "Unrecognised operator '" + op + "' specified in assertion.");
        }

        // Either side of an assertion operation should be a literal or a variable.
        if (!parser.expect_any({
                expectation(lexeme::var).be_true(),
                expectation(lexeme::integer).be_true(), expectation(lexeme::res_id).be_true(), expectation(lexeme::percentage).be_true(),
                expectation(lexeme::string).be_true()
        })) {
            auto lx = parser.peek();
            log::fatal_error(lx, 1, "Expected either a variable, string literal or a numeric literal.");
        }
        auto rhs = parser.read();

        // Build the assertion
        if (op == "<") {
            assertions.emplace_back(assertion(lhs, assertion::lt, rhs));
        }
        else if (op == "<=") {
            assertions.emplace_back(assertion(lhs, assertion::lteq, rhs));
        }
        else if (op == "==") {
            assertions.emplace_back(assertion(lhs, assertion::eq, rhs));
        }
        else if (op == "!=") {
            assertions.emplace_back(assertion(lhs, assertion::neq, rhs));
        }
        else if (op == ">=") {
            assertions.emplace_back(assertion(lhs, assertion::gteq, rhs));
        }
        else if (op == ">") {
            assertions.emplace_back(assertion(lhs, assertion::gt, rhs));
        }
        else {
            throw std::logic_error("Encountered an unexpected operator type after validating the operator: " + op);
        }

        // Check if we're specifying another assertion.
        if (!parser.expect({ expectation(lexeme::r_paren).be_true() })) {
            parser.ensure({ expectation(lexeme::comma).be_true() });
        }
    }

    parser.ensure({ expectation(lexeme::r_paren).be_true() });
    return assertions;
}
