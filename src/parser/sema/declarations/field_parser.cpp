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
#include "parser/sema/directives/hint_directive_parser.hpp"
#include "parser/sema/declarations/field_parser.hpp"
#include "parser/sema/declarations/unnamed_reference_value_parser.hpp"
#include "parser/sema/declarations/named_reference_value_parser.hpp"
#include "parser/sema/declarations/named_value_parser.hpp"
#include "implicit_value_parser.hpp"

// MARK: - Constructor

kdl::sema::field_parser::field_parser(kdl::sema::parser &parser, build_target::type_container& type, build_target::resource_constructor& instance, std::weak_ptr<target> target)
    : m_parser(parser), m_instance(instance), m_type(type), m_target(std::move(target))
{

}

// MARK: - Parser

auto kdl::sema::field_parser::parse() -> void
{
    if (m_parser.expect({ expectation(lexeme::directive, "hint").be_true() })) {
        sema::hint_directive_parser::parse(m_parser, m_target);
    }

    if (!m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Expected an identifier for the field name.");
    }
    auto field_name = m_parser.read();
    auto field = m_type.field_named(field_name);

    if (field.has_repeatable_count_field()) {
        field_name = field.repeatable_count_field();
    }

    // Lock the field and handle repeated instances.
    auto lock = m_instance.acquire_field(field_name, field.lower_repeat_bound());
    if (field.is_repeatable() && lock > field.upper_repeat_bound()) {
        log::fatal_error(field_name, 1, "Attempted to reference '" + field_name.text() + "' more than its allowed maximum.");
    }
    else if (!field.is_repeatable() && lock > 0) {
        log::fatal_error(field_name, 1, "Attempted to reference field '" + field_name.text() + "' more than once.");
    }

    m_parser.ensure({ expectation(lexeme::equals).be_true() });

    if (field.expected_values() > 1 && m_parser.expect({ expectation(lexeme::l_brace).be_true() })) {
        // We're looking at multi value field, and its being provided as an explicit object.
        m_parser.advance();

        m_instance.add_list_element(field_name, [&] (build_target::resource_constructor *resource) {
            // Make sure we apply the defaults for the new list element
            apply_defaults_for_field(field);
            m_parser.clear_pushed_lexemes();

            while (m_parser.expect({ expectation(lexeme::r_brace).be_false() })) {
                if (!m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
                    log::fatal_error(m_parser.peek(), 1, "Expected an identifier for the field name.");
                }
                auto sub_field_name = m_parser.read();
                auto field_value = field.value_named(sub_field_name);

                m_parser.ensure({ expectation(lexeme::equals).be_true() });

                parse_value(field, field_value, lock);

                m_parser.ensure({ expectation(lexeme::semi).be_true() });
            }
        });

        m_parser.ensure({ expectation(lexeme::r_brace).be_true() });
    }
    else if (field.has_repeatable_count_field()) {
        m_instance.add_list_element(field_name, [&] (build_target::resource_constructor *resource) {
            // Make sure we apply the defaults for the new list element
            apply_defaults_for_field(field);
            m_parser.clear_pushed_lexemes();

            auto value = field.value_at(0);
            parse_value(field, value, lock);
        });
    }
    else {
        for (auto n = 0; n < field.expected_values(); ++n) {
            auto value = field.value_at(n);
            parse_value(field, value, lock);
        }
    }
}

auto kdl::sema::field_parser::parse_value(kdl::build_target::type_field& field, kdl::build_target::type_field_value& field_value, std::int32_t field_number) -> void
{
    const auto& field_name = field.name();

    // TODO: Fix this hack...
    std::vector<build_target::type_template::binary_field> binary_fields;
    for (auto i = 0; i <= field_value.joined_value_count(); ++i) {
        auto extended_name = (i == 0 ? field_value : field_value.joined_value_at(i - 1)).extended_name({
            std::pair("FieldNumber", lexeme(std::to_string(field_number), lexeme::integer))
        });
        binary_fields.emplace_back(m_type.internal_template().binary_field_named(extended_name));
    }

    if (m_parser.expect({ expectation(lexeme::semi).be_true() })) {
        // There are no more values provided for the field, so we need to use default values.
        if (field_value.default_value().has_value()) {
            m_parser.push({
                field_value.default_value().value()
            });
        }
        else {
            log::fatal_error(m_parser.peek(), 1, "Unexpected ';' found. Too few values provided to field '" + field_name.text() + "'");
        }
    }

    // Is the value a pre-defined symbol.
    if (m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
        auto symbol = m_parser.peek();
        for (const auto& it : field_value.symbols()) {
            if (std::get<0>(it).is(symbol.text())) {
                m_parser.advance();
                m_parser.push({ std::get<1>(it) });
                break;
            }
        }
    }

    // Are we looking at an explicitly provided type?
    if (field_value.explicit_type().has_value()) {
        parse_explicit_typed_value(field, field_value, binary_fields);
    }
    else {
        parse_implicitly_typed_value(field, field_value, binary_fields);
    }
}

auto kdl::sema::field_parser::parse_explicit_typed_value(kdl::build_target::type_field &field,
                                                         kdl::build_target::type_field_value &field_value,
                                                         std::vector<build_target::type_template::binary_field>& binary_fields) -> void
{
    auto explicit_type = field_value.explicit_type().value();

    // There are several forms an explicit type can take.
    if (explicit_type.name().has_value() && explicit_type.is_reference()) {
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
        log::fatal_error(field_value.base_name(), 1, "Unrecognised explicit type encountered.");
    }
}

auto kdl::sema::field_parser::parse_implicitly_typed_value(kdl::build_target::type_field& field,
                                                           kdl::build_target::type_field_value& field_value,
                                                           std::vector<build_target::type_template::binary_field>& binary_fields) -> void
{
    implicit_value_parser(m_parser, m_target, field, field_value, binary_fields.back())
        .parse(m_instance);
}

auto kdl::sema::field_parser::apply_defaults_for_field(const build_target::type_field& type_field) -> void
{
    auto field_name = type_field.name();
    if (type_field.has_repeatable_count_field()) {
        field_name = type_field.repeatable_count_field();
    }
    auto field = m_type.field_named(type_field.name());

    auto lower = field.lower_repeat_bound();
    auto upper = type_field.has_repeatable_count_field() ? lower : field.upper_repeat_bound();

    for (auto field_number = lower; field_number <= upper; ++field_number) {
        auto lock = 0;
        if (!type_field.has_repeatable_count_field()) {
            lock = m_instance.acquire_field(field_name, lower);
        }

        // Iterate over the expected values for the field.
        for (auto n = 0; n < field.expected_values(); ++n) {
            auto field_value = field.value_at(n);

            // Do we have a default value - if not break out of this value and move to the next field.
            if (!field_value.default_value().has_value()) {
                continue;
            }

            // Handle joined/merged values
            std::vector<build_target::type_template::binary_field> binary_fields;
            for (auto i = 0; i <= field_value.joined_value_count(); ++i) {
                auto extended_name = (i == 0 ? field_value : field_value.joined_value_at(i - 1)).extended_name({
                    std::pair("FieldNumber", lexeme(std::to_string(lock), lexeme::integer))
                });
                binary_fields.emplace_back(m_type.internal_template().binary_field_named(extended_name));
            }

            // Push a default value in preparation for inserting a default value. Push a trailing semi colon as well,
            // due to some value checking for it.
            m_parser.push({
                field_value.default_value().value(),
                lexeme(";", lexeme::semi)
            });

            if (field_value.explicit_type().has_value()) {
                parse_explicit_typed_value(field, field_value, binary_fields);
            }
            else {
                parse_implicitly_typed_value(field, field_value, binary_fields);
            }
        }

    }
}