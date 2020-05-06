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
#include "disassembler/binary_parser.hpp"

// MARK: - Construction

kdl::disassembler::binary_parser::binary_parser(kdl::build_target::type_template &tmpl)
    : m_tmpl(tmpl)
{

}

// MARK: - Primary Parser

auto kdl::disassembler::binary_parser::parse(graphite::data::reader &reader) -> std::map<int, std::any>
{
    std::map<int, std::any> items;
    m_index = 0;
    for (m_index = 0; m_index < m_tmpl.binary_field_count(); ++m_index) {
        items.insert(items.end(), std::make_pair(m_index, extract_value(reader)));
    }
    return items;
}

// MARK: - Supporting

auto kdl::disassembler::binary_parser::extract_value(graphite::data::reader &reader) -> std::any
{
    auto type = m_tmpl.binary_field_at(m_index).type & ~0xFFF;
    auto len = m_tmpl.binary_field_at(m_index).type & 0xFFF;
    switch (type) {
        // Dynamic types
        case build_target::OCNT: {
            // TODO: This is using the shortcut found in the assembler - both should be done correctly.
            auto object_count = reader.read_short();
            m_index++;

            std::vector<std::any> objects;
            for (auto i = 0; i < object_count; ++i) {
                objects.emplace_back(extract_value(reader));
            }

            return objects;
        }
        case build_target::HEXD: {
            return reader.read_bytes(reader.size() - reader.position());
        }

        // Strings
        case build_target::CSTR: {
            return reader.read_cstr();
        }
        case build_target::PSTR: {
            return reader.read_pstr();
        }
        case build_target::Cnnn: {
            return reader.read_cstr(len);
        }

        // Integers
        case build_target::DBYT: {
            return reader.read_signed_byte();
        }
        case build_target::DWRD: {
            return reader.read_signed_short();
        }
        case build_target::DLNG: {
            return reader.read_signed_long();
        }
        case build_target::DQAD: {
            return reader.read_signed_quad();
        }
        case build_target::HBYT: {
            return reader.read_byte();
        }
        case build_target::HWRD: {
            return reader.read_short();
        }
        case build_target::HLNG: {
            return reader.read_long();
        }
        case build_target::HQAD: {
            return reader.read_quad();
        }

        // Special Structures
        case build_target::RECT: {
            return std::tuple(
                reader.read_signed_short(), reader.read_signed_short(),
                reader.read_signed_short(), reader.read_signed_short()
            );
        }
    }

    throw std::logic_error("Unhandled template type encountered:.");
}
