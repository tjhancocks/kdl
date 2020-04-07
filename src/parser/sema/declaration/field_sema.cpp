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
#include "field_sema.hpp"
#include "parser/sema/declaration/value_sema.hpp"
#include "parser/file.hpp"
#include "target/resource.hpp"

auto kdl::sema::field_sema::test(kdl::sema::parser &parser) -> bool
{
    return parser.expect({
        expectation(lexeme::identifier).be_true()
    });
}

auto kdl::sema::field_sema::parse(kdl::sema::parser &parser, kdl::container &type_container, resource& resource_data, std::weak_ptr<kdl::target> target) -> void
{
    if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Expected an identifier for the field name.");
    }
    auto field_name = parser.read();
    auto field_definition = type_container.field_named(field_name);

    parser.ensure({ expectation(lexeme::equals).be_true() });

    // Iterate over the expected values, and check the type of each. If the value has an explicit type provided then
    // handle it accordingly. If the value infers a type from the template, then determine the best course of action.
    for (auto i = 0; i < field_definition.value_count(); ++i) {
        auto value_definition = field_definition.value_at(i);

        // Before proceeding, check if the user has supplied a value here - we do not care about type just yet.
        if (!parser.expect({ expectation(lexeme::semi).be_false() })) {
            // We do not have a value provided for this value - take the default one if it exists.
            if (value_definition.default_value().has_value()) {
                // Push the default value onto the parser and use it.
                parser.push(value_definition.default_value().value());
            }
            else {
                log::fatal_error(parser.peek(), 1, "Incorrect number of values provided to field '" + field_name.text() + "'");
            }
        }

        if (value_definition.type().has_value()) {
            // There is a explicit type provided.
            auto value_type = value_definition.type().value();
            auto tmpl = type_container.template_field_named(value_definition.name_lexeme());

            // There are several forms an explicit type can take, and the way in which they are handled varies.
            if (value_type.name_lexeme().has_value() && value_type.is_reference()) {
                // TODO: Proper implementation of named reference types.
                value_sema::parse_any_reference(parser, field_definition, value_definition, std::get<1>(tmpl), resource_data);
            }
            else if (value_type.name_lexeme().has_value()) {
                value_sema::parse_named_type(parser, value_type, field_definition, value_definition, std::get<1>(tmpl), target, resource_data);
            }
            else if (value_type.is_reference()) {
                value_sema::parse_any_reference(parser, field_definition, value_definition, std::get<1>(tmpl), resource_data);
            }
            else {
                throw std::logic_error("Impossible explicit type encountered - Any.");
            }
        }
        else {
            // There is no explicit type provided... infer the type.
            auto tmpl = type_container.template_field_named(value_definition.name_lexeme());
            value_sema::parse_value(parser, field_definition, value_definition, std::get<1>(tmpl), resource_data);
        }
    }
}
