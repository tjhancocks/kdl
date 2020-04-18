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
#include "parser/lexer.hpp"
#include "parser/sema/directives/configuration_directive_parser.hpp"

// MARK: - Parser

auto kdl::sema::configuration_directive_parser::parse(parser &parser, std::weak_ptr<target> target) -> void
{
    if (target.expired()) {
        throw std::logic_error("Build target has expired. This is a bug!");
    }
    auto t = target.lock();

    parser.ensure({ expectation(lexeme::l_brace).be_true() });

    while (parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
            log::fatal_error(parser.peek(), 1, "Expected an identifier for configuration key.");
        }
        auto key = parser.read().text();

        parser.ensure({ expectation(lexeme::equals).be_true() });

        if (key == "ScenarioBasePath") {
            if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                log::fatal_error(parser.peek(), 1, "ScenarioBasePath requires a string value.");
            }
            t->set_scenario_root(parser.read().text());
        }
        else {
            log::fatal_error(parser.peek(), 1, "Unrecognised configuration key '" + key + "'.");
        }

        parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    parser.ensure({ expectation(lexeme::r_brace).be_true() });
}