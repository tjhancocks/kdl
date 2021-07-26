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

#include <stdexcept>
#include <utility>
#include "parser/sema/type_definition/binary_field.hpp"
#include "diagnostic/fatal.hpp"
#include "parser/sema/declarations/field_parser.hpp"
#include "parser/sema/declarations/unnamed_reference_value_parser.hpp"
#include "parser/sema/declarations/named_reference_value_parser.hpp"
#include "parser/sema/declarations/named_value_parser.hpp"
#include "implicit_value_parser.hpp"

// MARK: - Constructor

kdl::sema::field_parser::field_parser(kdl::sema::parser &parser, build_target::type_container& type, build_target::resource_instance& instance, std::weak_ptr<target> target)
    : m_parser(parser), m_instance(instance), m_type(type), m_target(std::move(target))
{

}

// MARK: - Parser

auto kdl::sema::field_parser::parse() -> void
{
    if (!m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Expected an identifier for the field name.");
    }
    auto field_name = m_parser.read();
    auto field = m_type.field_named(field_name);

    // Lock the field, and handle repeated instances.
    auto lock = m_instance.acquire_field(field_name, field.lower_repeat_bound());
    if (field.is_repeatable() && lock > field.upper_repeat_bound()) {
        log::fatal_error(field_name, 1, "Attempted to reference field '" + field_name.text() + "' more than its allowed maximum.");
    }
    else if (!field.is_repeatable() && lock > 0) {
        log::fatal_error(field_name, 1, "Attempted to reference field '" + field_name.text() + "' more than once.");
    }

    m_parser.ensure({ expectation(lexeme::equals).be_true() });

    // Start investigating the value.
    for (auto n = 0; n < field.expected_values(); ++n) {
        auto field_value = field.value_at(n);

        // TODO: This is a slight hack, but it works for now.
        std::vector<build_target::type_template::binary_field> binary_fields;
        for (auto i = 0; i < field_value.joined_value_count() + 1; ++i) {
            auto extended_name = (i == 0 ? field_value : field_value.joined_value_at(i - 1)).extended_name({
                std::make_pair("FieldNumber", lexeme(std::to_string(lock), lexeme::integer))
            });
            binary_fields.emplace_back(m_type.internal_template().binary_field_named(extended_name));
        }

        if (m_parser.expect({ expectation(lexeme::semi).be_true() })) {
            // There are no more values provided for the field, so we need to use default values.
            if (field_value.default_value().has_value()) {
                m_parser.push({ field_value.default_value().value() });
            }
            else {
                log::fatal_error(m_parser.peek(), 1, "Incorrect number of values provided to field '" + field_name.text() + "'");
            }
        }

        // Are we looking at an explicitly provided type?
        if (field_value.explicit_type().has_value()) {
            auto explicit_type = field_value.explicit_type().value();

            // There are several forms an explicit type can take.
            if (explicit_type.name().has_value() && explicit_type.is_reference()) {
                // TODO: This should be a named reference.
                named_reference_value_parser(m_parser, field, field_value, binary_fields.back(), explicit_type, m_target)
                    .parse(m_instance);
            }
            else if (explicit_type.name().has_value()) {
                named_value_parser(m_parser, field, field_value, binary_fields, explicit_type, m_target)
                    .parse(m_instance);
            }
            else if (explicit_type.is_reference()) {
                unnamed_reference_value_parser(m_parser, m_target, field, field_value, binary_fields.back(), explicit_type)
                    .parse(m_instance);
            }
            else {
                throw std::logic_error("Impossible explicit type encountered - Any");
            }
        }

        // Or are we looking at an implicitly provided type.
        else {
            implicit_value_parser(m_parser,m_target, field, field_value, binary_fields.back())
                    .parse(m_instance);
        }
    }
}

auto kdl::sema::field_parser::apply_defaults_for_field(const lexeme& field_name) -> void
{
    auto field = m_type.field_named(field_name);

    // Iterate over the number of expected instances of the field
    for (auto field_number = field.lower_repeat_bound(); field_number <= field.upper_repeat_bound(); ++field_number) {
        auto lock = m_instance.acquire_field(field_name, field.lower_repeat_bound());

        // Iterate over the expected values for the field.
        for (auto n = 0; n < field.expected_values(); ++n) {
            auto field_value = field.value_at(n);

            // Do we have a default value - if not break out of this value and move to the next field
            if (!field_value.default_value().has_value()) {
                break;
            }

            // Handle joined/merged values.
            std::vector<build_target::type_template::binary_field> binary_fields;
            for (auto i = 0; i < field_value.joined_value_count() + 1; ++i) {
                auto extended_name = (i == 0 ? field_value : field_value.joined_value_at(i - 1)).extended_name({
                    std::make_pair("FieldNumber", lexeme(std::to_string(lock), lexeme::integer))
                });
                binary_fields.emplace_back(m_type.internal_template().binary_field_named(extended_name));
            }

            // Push a default value in preparation for inserting a default value. Push a trailing semi colon as well,
            // due to some values checking for it.
            m_parser.push({
                field_value.default_value().value(),
                lexeme(";", lexeme::semi)
            });

            // Are we looking at an explicitly provided type?
            if (field_value.explicit_type().has_value()) {
                auto explicit_type = field_value.explicit_type().value();

                // There are several forms an explicit type can take.
                if (explicit_type.name().has_value() && explicit_type.is_reference()) {
                    // TODO: This should be a named reference.
                    unnamed_reference_value_parser(m_parser, m_target, field, field_value, binary_fields.back(), explicit_type)
                            .parse(m_instance);
                }
                else if (explicit_type.name().has_value()) {
                    named_value_parser(m_parser, field, field_value, binary_fields, explicit_type, m_target)
                            .parse(m_instance);
                }
                else if (explicit_type.is_reference()) {
                    unnamed_reference_value_parser(m_parser, m_target, field, field_value, binary_fields.back(), explicit_type)
                            .parse(m_instance);
                }
                else {
                    throw std::logic_error("Impossible explicit type encountered on default field values - Any");
                }
            }

                // Or are we looking at an implicitly provided type.
            else {
                implicit_value_parser(m_parser, m_target, field, field_value, binary_fields.back())
                        .parse(m_instance);
            }
        }
    }
}
