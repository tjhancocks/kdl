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
#include "parser/sema/declarations/named_types/bitmask_parser.hpp"

// MARK: - Constructor

kdl::sema::bitmask_parser::bitmask_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                          kdl::build_target::type_field_value &field_value,
                                          std::vector<kdl::build_target::type_template::binary_field> binary_fields,
                                          kdl::build_target::kdl_type &type)
    : m_parser(parser),
      m_field(field),
      m_field_value(field_value),
      m_binary_fields(std::move(binary_fields)),
      m_explicit_type(type)
{

}

// MARK: - Parser

auto kdl::sema::bitmask_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    if (m_field.expected_values() != 1) {
        log::fatal_error(m_parser.peek(-1), 1, "The field '" + m_field.name().text() + "' should have only one value due to it being a 'Bitmask'.");
    }

    for (const auto& binary_field : m_binary_fields) {
        if (binary_field.type != build_target::HBYT && binary_field.type != build_target::HWRD && binary_field.type != build_target::HLNG && binary_field.type != build_target::HQAD) {
            log::fatal_error(m_parser.peek(-1), 1, "The field '" + m_field.name().text() + "' must be backed by either HBYT, HWRD, HLNG or HQAD values.");
        }
    }

    uint64_t mask = 0;
    std::vector<std::tuple<uint64_t, build_target::type_field_value, build_target::type_template::binary_field>> merged_masks;
    for (auto i = 0; i < m_field_value.joined_value_count(); ++i) {
        merged_masks.emplace_back(std::tuple(0ULL, m_field_value.joined_value_at(i), m_binary_fields.at(i + 1)));
    }

    while (m_parser.expect({ expectation(lexeme::semi).be_false() })) {
        if (m_parser.expect({ expectation(lexeme::integer).be_true() })) {
            mask |= m_parser.read().value<uint64_t>();
        }
        else if (m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
            auto symbol = m_parser.read();
            auto symbol_ref = m_field_value.joined_value_for(symbol);

            if (!symbol_ref.has_value()) {
                // We're looking a symbol for the current field value.
                auto symbol_value = m_field_value.value_for(symbol);

                if (!symbol_value.is(lexeme::integer)) {
                    log::fatal_error(symbol, 1, "Type mismatch for '" + symbol.text() + "' in bitmask.");
                }

                mask |= symbol_value.value<uint64_t>();
            }
            else {
                // We're looking at a symbol for a joined/merged field value.
                auto symbol_value = std::get<1>(symbol_ref.value());
                auto merged_field_index = std::get<0>(symbol_ref.value());

                if (!symbol_value.is(lexeme::integer)) {
                    log::fatal_error(symbol, 1, "Type mismatch for '" + symbol.text() + "' in bitmask.");
                }

                // TODO: Check for a better way of doing this...
                auto t = merged_masks.at(merged_field_index);
                auto t_mask = std::get<0>(t);
                t_mask |= symbol_value.value<uint64_t>();
                merged_masks[merged_field_index] = std::tuple(t_mask, std::get<1>(t), std::get<2>(t));
            }
        }
        else {
            auto lx = m_parser.peek();
            log::fatal_error(lx, 1, "Unexpected lexeme encountered in bitmask: '" + lx.text() + "'");
        }

        if (m_parser.expect({ expectation(lexeme::semi).be_false() })) {
            m_parser.ensure({ expectation(lexeme::pipe).be_true() });
        }
    }

    // Add the primary value as a merge one for the purpose of this part...
    merged_masks.emplace_back(std::tuple(mask, m_field_value, m_binary_fields.at(0)));
    for (const auto& merged_mask : merged_masks) {
        auto value = std::get<0>(merged_mask);
        const auto& field_value = std::get<1>(merged_mask);

        switch (std::get<2>(merged_mask).type & ~0xFFFUL) {
            case build_target::HBYT: {
                instance.write_byte(m_field, field_value, static_cast<uint8_t>(value & 0xFFUL));
                break;
            }
            case build_target::HWRD: {
                instance.write_short(m_field, field_value, static_cast<uint16_t>(value & 0xFFFFUL));
                break;
            }
            case build_target::HLNG: {
                instance.write_long(m_field, field_value, static_cast<uint32_t>(value & 0xFFFFFFFFUL));
                break;
            }
            case build_target::HQAD: {
                instance.write_quad(m_field, field_value, value);
                break;
            }
            default: {
                throw std::logic_error("Unexpected bitmask type encountered.");
            }
        }
    }
}
