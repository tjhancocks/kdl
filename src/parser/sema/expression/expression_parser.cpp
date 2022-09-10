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
#include "diagnostic/fatal.hpp"
#include "expression_parser.hpp"
#include "target/new/kdl_expression.hpp"

// MARK: - Constructor

kdl::sema::expression_parser::expression_parser(kdl::sema::parser &parser,
                                                std::weak_ptr<target> target,
                                                std::map<std::string, kdl::lexeme> vars)
    : m_parser(parser), m_vars(std::move(vars))
{
    if (target.expired()) {
        throw std::logic_error("Build target has expired. This is a bug.");
    }
    m_target = target.lock();
}


// MARK: - Parser

auto kdl::sema::expression_parser::parse() -> kdl::lexeme
{
    if (m_parser.expect({ expectation(lexeme::var).be_true() })) {
        const auto& var_name = m_parser.read();
        const auto var = m_target->global_variable(var_name.text());
        if (var.has_value()) {
            return var.value();
        }
        log::fatal_error(var_name, 1, "Unrecognised variable name encountered in expression.");
    }
    else if (m_parser.expect({ expectation(lexeme::l_expr).be_true() })) {
        // Ensure that we're at the start of an expression.
        m_parser.ensure({ expectation(lexeme::l_expr).be_true() });

        std::vector<lexeme> expression_lexemes;
        while (m_parser.expect({ expectation(lexeme::r_expr).be_false() })) {
            expression_lexemes.emplace_back(m_parser.read());
        }

        m_parser.ensure({ expectation(lexeme::r_expr).be_true() });

        build_target::kdl_expression expression(expression_lexemes);
        return expression.evaluate(m_target);
    }
    else {
        log::fatal_error(m_parser.peek(), 1, "Unexpected lexeme encountered in expression.");
    }


}

// MARK: - Variables

auto kdl::sema::expression_parser::resolve_variable_named(const kdl::lexeme& var) const -> kdl::lexeme
{
    // We need to do two passes here... search our local variables that we were supplied with, and then
    // search the global variable scope.
    for (const auto& local_var : m_vars) {
        if (local_var.first == var.text()) {
            return local_var.second;
        }
    }

    const auto& global = m_target->global_variable(var.text());
    if (global.has_value()) {
        return global.value();
    }

    log::fatal_error(var, 1, "Unrecognised variable name '" + var.text() + "'.");
}
