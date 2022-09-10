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

#include "parser/sema/expression/variable_parser.hpp"
#include "diagnostic/fatal.hpp"

auto kdl::sema::variable_parser::parse(parser &parser, std::shared_ptr<target> target, const std::unordered_map<std::string, kdl::lexeme> vars) -> kdl::lexeme
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

    auto var_name = parser.read();
    auto it = local_vars.find(var_name.text());
    if (it == local_vars.end()) {
        log::fatal_error(var_name, 1, "Unrecognised variable referenced.");
    }
    return it->second;
}