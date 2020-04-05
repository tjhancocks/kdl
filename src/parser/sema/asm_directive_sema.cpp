// Copyright (c) 2019-2020 Tom Hancocks
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

#include <map>
#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include "parser/sema/asm_directive_sema.hpp"
#include "parser/sema/type_definition_sema.hpp"
#include "parser/parser.hpp"
#include "parser/expectation.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Directive Implementations

typedef std::tuple<std::string, std::vector<enum kdl::lexeme::type>> directive_symbol;
typedef std::function<auto(std::vector<kdl::lexeme>) -> void> directive_function;

static inline auto directives() -> std::map<directive_symbol, directive_function>
{
    decltype(directives()) f;

    // @out string;
    f.emplace(std::make_tuple("out", std::vector<enum kdl::lexeme::type>({kdl::lexeme::string}) ), [] (std::vector<kdl::lexeme> lx) {
        std::cout << lx.at(0).text() << std::endl;
    });

    return f;
}

// MARK: - Parser

auto kdl::sema::asm_directive_sema::test(parser& parser) -> bool
{
    return parser.expect({
        expectation(lexeme::directive).be_true()
    });
}

auto kdl::sema::asm_directive_sema::parse(parser &parser, std::weak_ptr<kdl::target> target) -> void
{
    if (parser.expect({ expectation(lexeme::directive).be_true()}) == false) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Unexpected lexeme '" + lx.text() + "' encountered whilst parsing directive.");
    }

    // Get the directive names and all of the arguments associated with it (all lexemes before the next semi-colon).
    auto directive = parser.read();
    auto directive_name = directive.text();

    // Before proceeding with reading the directive, check for the @type directive. This needs to go through a separate
    // distinct parser.
    if (directive_name == "type") {
        type_definition_sema::parse(parser, target);
        return;
    }

    auto directives_args = parser.consume(expectation(lexeme::semi).be_false());
    parser.ensure({ expectation(lexeme::semi).be_true() });

    auto all_directives = directives();
    auto i = all_directives.begin();
    while (i != all_directives.end()) {
        // Get the directive symbol
        auto symbol = i->first;

        // Check the name of the directive first. If it is not a match then continue to the next directive.
        if (std::get<0>(symbol) != directive_name) {
            ++i;
            continue;
        }

        // Check the expected arguments. Do we have the correct number, and are they the correct types?
        auto expected_args = std::get<1>(symbol);
        if (expected_args.size() != directives_args.size()) {
            log::fatal_error(directive, 1, "Incorrect number of arguments given to directive '" + directive_name + "'");
        }

        // Check the types of each argument.
        auto a = directives_args.begin();
        for (auto type : expected_args) {
            if (!a->is(type)) {
                log::fatal_error(*a, 1, "Incorrect argument type encountered for directive.");
            }
        }

        // At this point we have confirmed that we are checking the correct directive. Return from the parser.
        auto fn = i->second;
        fn(directives_args);

        return;
    }

    // If we reach this point then we are looking at an unrecognised directive.
    log::fatal_error(directive, 1, "Unrecognised directive '" + directive_name + "' encountered.");
}