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
#include "parser/sema/declarations/named_types/bitmask_parser.hpp"

// MARK: - Constructor

kdl::sema::bitmask_parser::bitmask_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                          kdl::build_target::type_field_value &field_value,
                                          kdl::build_target::type_template::binary_field binary_field,
                                          kdl::build_target::kdl_type &type)
    : m_parser(parser),
      m_field(field),
      m_field_value(field_value),
      m_binary_field(binary_field),
      m_explicit_type(type)
{

}

// MARK: - Parser

auto kdl::sema::bitmask_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    if (m_field.expected_values() != 1) {
        log::fatal_error(m_parser.peek(-1), 1, "The field '" + m_field.name().text() + "' should have only one value due to it being a 'Bitmask'.");
    }

    if (m_binary_field.type != build_target::HBYT && m_binary_field.type != build_target::HWRD && m_binary_field.type != build_target::HLNG && m_binary_field.type != build_target::HQAD) {
        log::fatal_error(m_parser.peek(-1), 1, "The field '" + m_field.name().text() + "' must be backed by either a HBYT, HWRD, HLNG or HQAD value.");
    }

    uint64_t mask = 0;

    while (m_parser.expect({ expectation(lexeme::semi).be_false() })) {
        if (m_parser.expect({ expectation(lexeme::integer).be_true() })) {
            mask |= m_parser.read().value<uint64_t>();
        }
        else if (m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
            auto symbol = m_parser.read();
            auto symbol_value = m_field_value.value_for(symbol);

            if (!symbol_value.is(lexeme::integer)) {
                log::fatal_error(symbol, 1, "Type mismatch for '" + symbol.text() + "' in bitmask.");
            }

            mask |= symbol_value.value<uint64_t>();
        }
        else {
            auto lx = m_parser.peek();
            log::fatal_error(lx, 1, "Unexpected lexeme encountered in bitmask: '" + lx.text() + "'");
        }

        if (m_parser.expect({ expectation(lexeme::semi).be_false() })) {
            m_parser.ensure({ expectation(lexeme::pipe).be_true() });
        }
    }

    switch (m_binary_field.type & ~0xFFF) {
        case build_target::HBYT: {
            instance.write_byte(m_field, m_field_value, static_cast<uint8_t>(mask & 0xFF));
            break;
        }
        case build_target::HWRD: {
            instance.write_short(m_field, m_field_value, static_cast<uint16_t>(mask & 0xFFFF));
            break;
        }
        case build_target::HLNG: {
            instance.write_long(m_field, m_field_value, static_cast<uint32_t>(mask & 0xFFFFFFFF));
            break;
        }
        case build_target::HQAD: {
            instance.write_quad(m_field, m_field_value, mask);
            break;
        }
        default: {
            throw std::logic_error("Unexpected bitmask type encountered.");
        }
    }
}
