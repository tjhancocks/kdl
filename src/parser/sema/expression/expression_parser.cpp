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
#include "parser/sema/expression/variable_parser.hpp"
#include "parser/sema/expression/function_parser.hpp"

// MARK: - Extraction

auto kdl::sema::expression_parser::extract(parser &parser) -> std::shared_ptr<build_target::kdl_expression>
{
    std::vector<lexeme> expression_lexemes;

    if (parser.expect({
        expectation(lexeme::l_expr).be_true()
    })) {
        parser.ensure({ expectation(lexeme::l_expr).be_true() });
        while (parser.expect({ expectation(lexeme::r_expr).be_false() })) {
            expression_lexemes.emplace_back(parser.read());
        }
        parser.ensure({ expectation(lexeme::r_expr).be_true() });
    }
    else {
        while (parser.expect_any({
            expectation(lexeme::comma).be_true(),
            expectation(lexeme::semi).be_true()
        })) {
            expression_lexemes.emplace_back(parser.read());
        }
    }

    return std::make_shared<build_target::kdl_expression>(expression_lexemes);
}

// MARK: - Evaluation

auto kdl::sema::expression_parser::evaluate(std::shared_ptr<target> target, const std::vector<kdl::lexeme> &lexemes, const std::unordered_map<std::string, kdl::lexeme> &vars) -> kdl::lexeme
{
    sema::parser parser(target, lexemes);
    return evaluate(parser, target, {}, vars);
}

auto kdl::sema::expression_parser::evaluate(parser &parser, std::shared_ptr<target> target, const std::vector<lexeme>& arguments, const std::unordered_map<std::string, kdl::lexeme>& vars) -> kdl::lexeme
{
    std::unordered_map<std::string, lexeme> local_vars(target->all_global_variables());

    for (const auto& var : vars) {
        auto it = local_vars.find(var.first);
        if (it == local_vars.end()) {
            local_vars.insert(std::pair(var.first, var.second));
        }
        else {
            it->second = var.second;
        }
    }

    // Arguments need to be converted to local variables in the form of $0, $1, $2...
    for (auto i = 0; i < arguments.size(); ++i) {
        auto it = local_vars.find(std::to_string(i + 1));
        if (it == local_vars.end()) {
            local_vars.insert(local_vars.end(), std::pair(std::to_string(i + 1), arguments[i]));
        }
        else {
            it->second = arguments[i];
        }
    }

    // We want to try and shortcut for single values and just return the value directly.
    if (parser.size() == 1) {
        if (parser.expect_any({
            sema::expectation(lexeme::string).be_true(),
            sema::expectation(lexeme::integer).be_true(),
            sema::expectation(lexeme::percentage).be_true(),
            sema::expectation(lexeme::res_id).be_true(),
        })) {
            return parser.read();
        }
        else if (parser.expect_any({
            sema::expectation(lexeme::var).be_true(),
            sema::expectation(lexeme::identifier).be_true(),
        })) {
            return variable_parser::parse(parser, target);
        }

        log::fatal_error(parser.peek(), 1, "Invalid lexeme encountered in expression.");
    }

    std::vector<lexeme> output_queue;
    std::vector<lexeme> operator_stack;
    enum lexeme::type result_type = lexeme::integer;
    while (!parser.finished()) {
        // Determine what course of action needs to be taken. Variables need to be substituted out with their actual
        // values, and functions need to be called and executed.
        if (parser.expect({
            sema::expectation(lexeme::identifier).be_true(),
            sema::expectation(lexeme::l_paren).be_true()
        })) {
            parser.push({ function_parser::parse(parser, target, vars) });
        }
        else if (parser.expect_any({
            sema::expectation(lexeme::var).be_true(),
            sema::expectation(lexeme::identifier).be_true()
        })) {
            auto var_name = parser.read();
            const auto& it = local_vars.find(var_name.text());
            if (it == local_vars.end()) {
                log::fatal_error(var_name, 1, "Unrecognised variable reference.");
            }
            parser.push({ it->second });
        }

        // Read the next lexeme token, and perform the shunting yard.
        auto token = parser.read();
        if (token.is(lexeme::integer)) {
            output_queue.emplace_back(token);
        }
        else if (token.is(lexeme::res_id)) {
            if (result_type == lexeme::percentage) {
                log::fatal_error(token, 1, "Value is incompatible with current expression result type of 'percentage'");
            }
            output_queue.emplace_back(token);
            result_type = lexeme::res_id;
        }
        else if (token.is(lexeme::percentage)) {
            if (result_type == lexeme::res_id) {
                log::fatal_error(token, 1, "Value is incompatible with current expression result type of 'resource_id'");
            }
            output_queue.emplace_back(token);
            result_type = lexeme::percentage;
        }
        else if (token.is(lexeme::plus) || token.is(lexeme::minus) || token.is(lexeme::star) || token.is(lexeme::slash) || token.is(lexeme::carat)) {
            while (!operator_stack.empty()) {
                const auto& o2 = operator_stack.back();
                if ((!o2.is(lexeme::l_paren)) && (o2.value<std::int64_t>() >= token.value<std::int64_t>() && token.left_associative())) {
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

    // We're now ready to evaluate the expression.
    std::vector<std::int64_t> working_stack;
    for (const auto& o : output_queue) {
        if (o.is(lexeme::integer) || o.is(lexeme::percentage) || o.is(lexeme::res_id)) {
            working_stack.emplace_back(o.value<std::int64_t>());
        }
        else if (o.is(lexeme::plus)) {
            const auto lhs = working_stack.back(); working_stack.pop_back();
            const auto rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs + rhs);
        }
        else if (o.is(lexeme::minus)) {
            const auto lhs = working_stack.back(); working_stack.pop_back();
            const auto rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs - rhs);
        }
        else if (o.is(lexeme::star)) {
            const auto lhs = working_stack.back(); working_stack.pop_back();
            const auto rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs * rhs);
        }
        else if (o.is(lexeme::slash)) {
            const auto lhs = working_stack.back(); working_stack.pop_back();
            const auto rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs / rhs);
        }
        else if (o.is(lexeme::carat)) {
            const auto lhs = working_stack.back(); working_stack.pop_back();
            const auto rhs = working_stack.back(); working_stack.pop_back();
            working_stack.emplace_back(lhs ^ rhs);
        }
    }

    if (working_stack.empty() || working_stack.size() > 1) {
        throw std::logic_error("There was an error evaluating the expression. Stack size was " + std::to_string(working_stack.size()));
    }
    return { std::to_string(working_stack.back()), result_type };
}