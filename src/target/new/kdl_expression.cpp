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
#include "parser/sema/expression/expression_parser.hpp"
#include "diagnostic/fatal.hpp"
#include "target/target.hpp"
#include <optional>
#include <utility>

// MARK: - Construction

kdl::build_target::kdl_expression::kdl_expression(const std::vector<lexeme> &lexemes)
    : m_lexemes(std::move(lexemes))
{}

// MARK: - Evaluation

auto kdl::build_target::kdl_expression::evaluate(std::weak_ptr<target> target, const std::vector<lexeme> &arguments, const std::unordered_map<std::string, kdl::lexeme>& vars) const -> lexeme
{
    std::unordered_map<std::string, lexeme> local_vars;

    // Capture global variables if they are available.
    if (auto t = target.lock()) {
        local_vars = t->all_global_variables();
    }

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

    if (auto t = target.lock()) {
        return sema::expression_parser::evaluate(t, m_lexemes, local_vars);
    }
    else {
        throw std::runtime_error("Missing build target. This is a bug.");
    }
}
