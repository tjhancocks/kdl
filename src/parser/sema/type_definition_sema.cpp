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

#include "diagnostic/fatal.hpp"
#include "parser/sema/type_definition_sema.hpp"
#include "parser/sema/template_sema.hpp"
#include "parser/sema/field_definition_sema.hpp"
#include "target/container.hpp"

auto kdl::sema::type_definition_sema::parse(kdl::sema::parser& parser, std::weak_ptr<kdl::target> target) -> void
{
    // We can safely assume that the '@type' lexeme has been consumed at this point.
    // The next sequence of lexemes are:
    //     TYPE_NAME : "TYPE_CODE"
    if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Expected an identifier for type name.");
    }
    auto type_name = parser.read();

    parser.ensure({ expectation(lexeme::colon).be_true() });

    if (!parser.expect({ expectation(lexeme::string).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Expected a string for type code.");
    }
    auto type_code = parser.read();
//    if (type_code.text().length() != 4) {
//        log::fatal_error(type_code, 1, "A type code must be exactly 4 characters long.");
//    }

    // Construct the basic type container, before proceeding to populate the type.
    kdl::container type_container(type_name, type_code.text());

    // The contents of the type definition are enclosed in '{' braces '}'.
    // Ensure the opening brace is present.
    parser.ensure({ expectation(lexeme::l_brace).be_true() });

    // Keep looping until the matching closing brace is found.
    while (parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        auto lx = parser.read();
        if (lx.text() == "template") {
            template_sema::parse(parser, type_container);
        }
        else if (lx.text() == "field") {
            field_definition_sema::parse(parser, type_container);
        }
        else {
            log::fatal_error(lx, 1, "Unknown lexeme '" + lx.text() + "' encountered in type definition");
        }

        // Each item in the type definition should be terminated by a semi-colon
        parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    // Wrap up the type definition.
    parser.ensure({
        expectation(lexeme::r_brace).be_true(),
        expectation(lexeme::semi).be_true()
    });

    // Register the type with the target.
    if (auto t = target.lock()) {
        t->add_container(type_container);
    }
}
