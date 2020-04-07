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
#include "template_sema.hpp"

auto kdl::sema::template_sema::parse(kdl::sema::parser &parser, kdl::container &type_container) -> void
{
    // We can safely assume that the 'template' keyword was already consumed.
    parser.ensure({ expectation(lexeme::l_brace).be_true() });

    // Loop until we encounter the corresponding closing brace.
    while (parser.expect({ expectation(lexeme::r_brace).be_false() })) {

        // The first aspect of the field is the type.
        if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
            auto lx = parser.peek();
            log::fatal_error(lx, 1, "Expected an identifier for template field type.");
        }
        auto type_name = parser.read();

        // Check if the type is valid
        auto type_info = type_for_name(type_name.text());
        if (type_info == NONE) {
            log::fatal_error(type_name, 1, "Unrecognised template type '" + type_name.text() + "'");
        }

        // The second apsect of the field is the label/name.
        if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
            auto lx = parser.peek();
            log::fatal_error(lx, 1, "Expected an identifier for template field label.");
        }
        auto type_label = parser.read();

        // Add the field to the template.
        type_container.add_template_field(type_label, type_info);

        parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    parser.ensure({ expectation(lexeme::r_brace).be_true() });
}
