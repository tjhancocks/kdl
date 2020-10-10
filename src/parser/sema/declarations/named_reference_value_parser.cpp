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

#include <utility>
#include "diagnostic/fatal.hpp"
#include "parser/sema/declarations/named_reference_value_parser.hpp"
#include "parser/sema/declarations/resource_instance_parser.hpp"

// MARK: - Constructor

kdl::sema::named_reference_value_parser::named_reference_value_parser(kdl::sema::parser &parser,
                                                                      build_target::type_field& field,
                                                                      build_target::type_field_value& field_value,
                                                                      build_target::type_template::binary_field binary_field,
                                                                      kdl::build_target::kdl_type &type,
                                                                      std::weak_ptr<target> target)
    : m_parser(parser),
      m_explicit_type(type),
      m_field(field),
      m_binary_field(std::move(binary_field)),
      m_field_value(field_value),
      m_target(std::move(target))
{

}

// MARK: - Parser

auto kdl::sema::named_reference_value_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    if (m_target.expired()) {
        throw std::logic_error("Expired target found in declaration parser.");
    }
    auto target = m_target.lock();

    if (!m_parser.expect_any({ expectation(lexeme::identifier).be_true(), expectation(lexeme::res_id).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "The field '" + m_field.name().text() + "' expects a either a nested resource or a resource id.");
    }

    auto ref = m_parser.peek();

    if (ref.is(lexeme::identifier, "new")) {
        if (!m_field_value.explicit_type().has_value() && !m_field_value.explicit_type()->name().has_value()) {
            throw std::logic_error("Reached a nested resource with no explicitly named reference type.");
        }
        auto type_container = target->type_container_named(m_field_value.explicit_type()->name().value());

        kdl::sema::resource_instance_parser instance_parser(m_parser, type_container, target);
        instance_parser.set_keyword("new");
        auto nested_instance = instance_parser.parse();

        // TODO: Propagate this correctly, and don't short circuit.
        target->add_resource(nested_instance);

        // Replace the nesting with an ID.
        ref = lexeme(std::to_string(nested_instance.id()), lexeme::res_id);
    }
    else if (ref.is(lexeme::identifier)) {
        m_parser.advance();
        auto symbol_value = m_field_value.value_for(ref);

        if (!symbol_value.is(lexeme::res_id)) {
            log::fatal_error(m_parser.peek(), 1, "The field '" + m_field.name().text() + "' expects a resource id valued symbol.");
        }

        ref = symbol_value;
    }
    else {
        // It's a resource id, so just advance.
        m_parser.advance();
    }

    // Ensure that the underlying type is correct for a reference.
    switch (m_binary_field.type & ~0xFFFUL) {
        case build_target::DWRD: {
            instance.write_signed_short(m_field, m_field_value, ref.value<int16_t>());
            break;
        }
        case build_target::DLNG: {
            instance.write_signed_long(m_field, m_field_value, ref.value<int32_t>());
            break;
        }
        case build_target::DQAD: {
            instance.write_signed_quad(m_field, m_field_value, ref.value<int64_t>());
            break;
        }
        default: {
            log::fatal_error(m_field.name(), 1, "Resource reference value should be backed by either a DWRD, DLNG or DQAD");
        }
    }
}