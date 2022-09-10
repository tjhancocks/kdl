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

#include "target/new/kdl_expression.hpp"
#include "diagnostic/fatal.hpp"
#include "parser/parser.hpp"
#include "target/target.hpp"
#include <optional>
#include <utility>

// MARK: - Construction

kdl::build_target::kdl_expression::kdl_expression(const std::vector<lexeme> &lexemes)
    : m_lexemes(std::move(lexemes))
{}

// MARK: - Evaluation

namespace kdl
{
    static auto parse_function(sema::parser& parser, std::weak_ptr<target> target, const std::unordered_map<std::string, lexeme>& vars) -> lexeme;

    static auto parse_argument(sema::parser &parser, std::weak_ptr<target> target, const std::unordered_map<std::string, lexeme>& vars)-> lexeme
    {
        std::vector<lexeme> argument_expression;

        while (!parser.expect_any({ sema::expectation(lexeme::comma).be_true(), sema::expectation(lexeme::r_paren).be_true() })) {
            if (parser.expect({ sema::expectation(lexeme::identifier).be_true(), sema::expectation(lexeme::l_paren).be_true() })) {
                argument_expression.emplace_back(parse_function(parser, target, vars));
            }
            else {
                argument_expression.emplace_back(parser.read());
            }
        }

        build_target::kdl_expression expr(argument_expression);
        return expr.evaluate(target);
    }

    static auto parse_function(sema::parser& parser, std::weak_ptr<target> target, const std::unordered_map<std::string, lexeme>& vars) -> lexeme
    {
        std::vector<lexeme> args;
        const auto& function_name_lx = parser.read();
        const auto& function_name = function_name_lx.text();
        parser.ensure({ sema::expectation(lexeme::l_paren).be_true() });

        // Handle special built in functions
        if (
            function_name == "__postIncrement" ||
            function_name == "__preIncrement" ||
            function_name == "__postDecrement" ||
            function_name == "__preDecrement"
        ) {
            // We now _require_ a variable to be specified.
            if (!parser.expect({ sema::expectation(lexeme::var).be_true() })) {
                log::fatal_error(parser.peek(), 1, "The built-in function '" + function_name + "' requires a variable name argument.");
            }
            auto var_name_lx = parser.read();

            if (auto t = target.lock()) {
                auto var = t->global_variable(var_name_lx.text());
                if (!var.has_value()) {
                    log::fatal_error(var_name_lx, 1, "Unrecognised variable name referenced.");
                }

                auto value = var.value();
                if (!value.is(lexeme::integer) && !value.is(lexeme::percentage) && !value.is(lexeme::res_id)) {
                    log::fatal_error(value, 1, "The built-in function '" + function_name + "' requires a variable for a numeric value to be specified as an argument.");
                }

                auto v = value.value<std::int64_t>();
                if (function_name == "__postIncrement") {
                    v++;
                    auto new_value = lexeme(std::to_string(v), value.type());
                    t->set_global_variable(var_name_lx.text(), new_value);
                }
                else if (function_name == "__preIncrement") {
                    v++;
                    auto new_value = lexeme(std::to_string(v), value.type());
                    t->set_global_variable(var_name_lx.text(), new_value);
                    value = new_value;
                }
                else if (function_name == "__postDecrement") {
                    v--;
                    auto new_value = lexeme(std::to_string(v), value.type());
                    t->set_global_variable(var_name_lx.text(), new_value);
                }
                else if (function_name == "__preDecrement") {
                    v--;
                    auto new_value = lexeme(std::to_string(v), value.type());
                    t->set_global_variable(var_name_lx.text(), new_value);
                    value = new_value;
                }

                parser.ensure({ sema::expectation(lexeme::r_paren).be_true() });
                return value;
            }
            else {
                log::fatal_error(function_name_lx, 1, "Unexpected error whilst calling function.");
            }
        }

        while (parser.expect({ sema::expectation(lexeme::r_paren).be_false() })) {
            if (parser.expect({ sema::expectation(lexeme::identifier).be_true(), sema::expectation(lexeme::l_paren).be_true() })) {
                args.emplace_back(parse_function(parser, target, vars));
            }
            else if (parser.expect({ sema::expectation(lexeme::var).be_true() })) {
                auto var_name = parser.read();
                auto it = vars.find(var_name.text());
                if (it == vars.end()) {
                    log::fatal_error(var_name, 1, "Unrecognised variable referenced.");
                }
                args.emplace_back(it->second);
            }
            else {
                args.emplace_back(parse_argument(parser, target, vars));
            }

            if (parser.expect({ sema::expectation(lexeme::comma).be_true() })) {
                parser.advance();
                continue;
            }
            else if (parser.expect({ sema::expectation(lexeme::r_paren).be_true() })) {
                break;
            }
            else {
                log::fatal_error(parser.peek(), 1, "Unexpected lexeme encountered in expression. Expected ',' or ')'.");
            }
        }

        parser.ensure({ sema::expectation(lexeme::r_paren).be_true() });

        // Prepare to call the function.
        if (auto t = target.lock()) {
            auto function_expression = t->function_expression(function_name);
            return function_expression->evaluate(target, args);
        }

        log::fatal_error(function_name_lx, 1, "Unable to evaluate function.");
    }
}

auto kdl::build_target::kdl_expression::evaluate(std::weak_ptr<target> target, const std::vector<lexeme> &arguments) const -> lexeme
{
    std::unordered_map<std::string, lexeme> local_vars;

    // Capture global variables if they are available.
    if (auto t = target.lock()) {
        local_vars = t->all_global_variables();
    }

    // Arguments need to be converted to local variables in the form of $0, $1, $2...
    for (auto i = 0; i < arguments.size(); ++i) {
        local_vars.emplace(std::pair(std::to_string(i + 1), arguments[i]));
    }

    // We evaluate the expression with the shunting yard algorithm. We can explore alterations to this in the future.
    // Setup a parser with the lexeme's that are in the expression.
    sema::parser parser(target, m_lexemes);

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
        else if (parser.expect({
            sema::expectation(lexeme::var).be_true(),
        })) {
            auto var_name = parser.read();
            auto it = local_vars.find(var_name.text());
            if (it == local_vars.end()) {
                log::fatal_error(var_name, 1, "Unrecognised variable referenced.");
            }
            return it->second;
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
            parser.push({ parse_function(parser, target, local_vars) });
        }
        else if (parser.expect({
            sema::expectation(lexeme::var).be_true()
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
