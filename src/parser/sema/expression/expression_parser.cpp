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
    // Before proceeding with the primary expression parser, we want to try and short circuit
    // the parse by identifying the variable shorthand.
    if (m_parser.expect({ expectation(lexeme::var).be_true() })) {
        auto var_name = m_parser.read();
        return resolve_variable_named(var_name);
    }

    kdl::lexeme result { "?", lexeme::string };

    // Ensure that we're at the start of an expression.
    m_parser.ensure({ expectation(lexeme::l_expr).be_true() });

    bool is_res_id { false };
    std::vector<lexeme> output_queue {};
    std::vector<lexeme> operator_stack {};
    while (m_parser.expect({ expectation(lexeme::r_expr).be_false() })) {
        // Read the next token. If it is an identifier then attempt to resolve it as a variable.
        // TODO: Functions?
        auto token = m_parser.read();
        if (token.is(lexeme::identifier)) {
            token = resolve_variable_named(token);
        }

        if (token.is(lexeme::res_id)) {
            is_res_id = true;
        }

        // Shunting yard...
        // If any token is a res_id, then the result of the expression will become a res_id.
        if (token.is(lexeme::integer) || token.is(lexeme::res_id)) {
            output_queue.emplace_back(token);
        }
        else if (token.is(lexeme::plus) || token.is(lexeme::minus) || token.is(lexeme::star) || token.is(lexeme::slash) || token.is(lexeme::carat)) {
            while (!operator_stack.empty()) {
                const auto& o2 = operator_stack.back();
                if ((!o2.is(lexeme::l_paren)) && (o2.value<int64_t>() >= token.value<int64_t>() && token.left_associative())) {
                    output_queue.emplace_back(operator_stack.back());
                    operator_stack.pop_back();
                }
                else {
                    break;
                }
            }
            operator_stack.emplace_back(token);
        }
        else if (token.is(lexeme::l_paren)) {
            operator_stack.emplace_back(token);
        }
        else if (token.is(lexeme::r_paren)) {
            while (!operator_stack.empty() && !operator_stack.back().is(lexeme::l_paren)) {
                output_queue.emplace_back(operator_stack.back());
                operator_stack.pop_back();
            }

            if (!operator_stack.back().is(lexeme::l_paren)) {
                log::fatal_error(operator_stack.back(), 1, "Expected a '(' token.");
            }
            operator_stack.pop_back();
        }
    }

    while (!operator_stack.empty()) {
        output_queue.emplace_back(operator_stack.back());
        operator_stack.pop_back();
    }

    // Finally ensure that we're actually at the end of the expression.
    m_parser.ensure({ expectation(lexeme::r_expr).be_true() });

    // We're now ready to evaluate the expression.
    std::vector<int64_t> working_stack {};
    for (const auto& o : output_queue) {
        if (o.is(lexeme::integer) || o.is(lexeme::res_id)) {
            working_stack.emplace_back(o.value<int64_t>());
        }
        else if (o.is(lexeme::plus)) {
            const auto& lhs = working_stack.back(); working_stack.pop_back();
            const auto& rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs + rhs);
        }
        else if (o.is(lexeme::minus)) {
            const auto& lhs = working_stack.back(); working_stack.pop_back();
            const auto& rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs - rhs);
        }
        else if (o.is(lexeme::star)) {
            const auto& lhs = working_stack.back(); working_stack.pop_back();
            const auto& rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs * rhs);
        }
        else if (o.is(lexeme::slash)) {
            const auto& lhs = working_stack.back(); working_stack.pop_back();
            const auto& rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs / rhs);
        }
        else if (o.is(lexeme::carat)) {
            const auto& lhs = working_stack.back(); working_stack.pop_back();
            const auto& rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs ^ rhs);
        }
    }

    if (working_stack.empty() || working_stack.size() > 1) {
        throw std::logic_error("There was an error evaluating the expression. Stack size was " + std::to_string(working_stack.size()));
    }
    result = kdl::lexeme(std::to_string(working_stack.back()), is_res_id ? lexeme::res_id : lexeme::integer);

    return result;
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
