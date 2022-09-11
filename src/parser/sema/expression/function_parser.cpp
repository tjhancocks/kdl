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

#include "parser/sema/expression/function_parser.hpp"
#include "parser/sema/expression/argument_parser.hpp"
#include "parser/sema/expression/variable_parser.hpp"
#include "target/new/kdl_expression.hpp"
#include "diagnostic/fatal.hpp"

auto kdl::sema::function_parser::parse(parser &parser, std::shared_ptr<target> target, const std::unordered_map<std::string, kdl::lexeme> vars) -> kdl::lexeme
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
        function_name == "__preDecrement" ||
        function_name == "__integer" ||
        function_name == "__string" ||
        function_name == "__percentage" ||
        function_name == "__resource_id"
    ) {
        // We now _require_ a variable to be specified.
        if (!parser.expect({ sema::expectation(lexeme::var).be_true() })) {
            log::fatal_error(parser.peek(), 1, "The built-in function '" + function_name + "' requires a variable name argument.");
        }
        auto var_name_lx = parser.read();
        auto var = vars.find(var_name_lx.text());
        if (var == vars.end()) {
            log::fatal_error(var_name_lx, 1, "Unrecognised variable name referenced.");
        }

        auto value = var->second;
        if (!value.is(lexeme::integer) && !value.is(lexeme::percentage) && !value.is(lexeme::res_id)) {
            log::fatal_error(value, 1, "The built-in function '" + function_name + "' requires a variable for a numeric value to be specified as an argument.");
        }

        auto v = value.value<std::int64_t>();
        if (function_name == "__postIncrement") {
            v++;
            auto new_value = lexeme(std::to_string(v), value.type());
            target->set_global_variable(var_name_lx.text(), new_value);
        }
        else if (function_name == "__preIncrement") {
            v++;
            auto new_value = lexeme(std::to_string(v), value.type());
            target->set_global_variable(var_name_lx.text(), new_value);
            value = new_value;
        }
        else if (function_name == "__postDecrement") {
            v--;
            auto new_value = lexeme(std::to_string(v), value.type());
            target->set_global_variable(var_name_lx.text(), new_value);
        }
        else if (function_name == "__preDecrement") {
            v--;
            auto new_value = lexeme(std::to_string(v), value.type());
            target->set_global_variable(var_name_lx.text(), new_value);
            value = new_value;
        }
        else if (function_name == "__integer") {
            value = lexeme(value.text(), lexeme::integer);
        }
        else if (function_name == "__string") {
            value = lexeme(value.text(), lexeme::string);
        }
        else if (function_name == "__percentage") {
            value = lexeme(value.text(), lexeme::percentage);
        }
        else if (function_name == "__resource_id") {
            value = lexeme(value.text(), lexeme::res_id);
        }

        parser.ensure({ sema::expectation(lexeme::r_paren).be_true() });
        return value;
    }

    while (parser.expect({ expectation(lexeme::r_paren).be_false() })) {
        if (parser.expect({ expectation(lexeme::identifier).be_true(), expectation(lexeme::l_paren).be_true() })) {
            args.emplace_back( parse(parser, target, vars) );
        }
        else if (parser.expect_any({ expectation(lexeme::var).be_true(), expectation(lexeme::identifier).be_true() })) {
            args.emplace_back(variable_parser::parse(parser, target, vars));
        }
        else {
            args.emplace_back(argument_parser::parse(parser, target, vars));
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
    auto function_expression = target->function_expression(function_name);
    return function_expression->evaluate(target, args);
}