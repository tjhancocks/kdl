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
#include "parser/sema/declarations/named_types/file_type_parser.hpp"

// MARK: - Constructor

kdl::sema::file_type_parser::file_type_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                              kdl::build_target::type_field_value &field_value,
                                              kdl::build_target::type_template::binary_field binary_field,
                                              kdl::build_target::kdl_type &type,
                                              std::weak_ptr<kdl::target> target)
    : m_parser(parser),
      m_explicit_type(type),
      m_field(field),
      m_binary_field(binary_field),
      m_field_value(field_value),
      m_target(target)
{

}

// MARK: - Parser

auto kdl::sema::file_type_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    auto target = m_target.lock();
    auto import_file = false;
    if (m_parser.expect({ expectation(lexeme::identifier).be_true() })) {
        m_parser.advance();
        import_file = true;
    }

    if (!m_parser.expect({ expectation(lexeme::string).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Fields with the 'File' type expect a string.");
    }
    auto string_lx = m_parser.read();
    auto string_value = string_lx.text();

    if (import_file) {
        auto path = target->resolve_src_path(string_value);
        string_value = kdl::file(path).contents();
    }

    // Get the value type for the field, and the set it.
    switch (m_binary_field.type & ~0xFFF) {
        case build_target::PSTR: {
            if (string_value.size() > 255) {
                log::fatal_error(string_lx, 1, "String too large for value type.");
            }
            instance.write_pstr(m_field, m_field_value, string_value, 0);
            break;
        }
        case build_target::CSTR: {
            instance.write_cstr(m_field, m_field_value, string_value);
            break;
        }
        case build_target::Cnnn: {
            auto size = static_cast<std::size_t>(m_binary_field.type) & 0xFFF;
            if (string_value.size() > size) {
                log::fatal_error(string_lx, 1, "String too large for value type.");
            }
            instance.write_cstr(m_field, m_field_value, string_value, size);
            break;
        }
//        case build_target::P0nn: {
//            auto size = static_cast<std::size_t>(m_binary_field.type) & 0x0FF;
//            if (string_value.size() > size) {
//                log::fatal_error(string_lx, 1, "String too large for value type.");
//            }
//            instance.write_pstr(m_field, m_field_value, string_value, size);
//            break;
//        }
        case build_target::HEXD: {
            instance.write_data(m_field, m_field_value, std::vector<char>(string_value.begin(), string_value.end()));
            break;
        }
        default: {
            log::fatal_error(string_lx, 1, "Unsupported value type for field '" + m_field.name().text() + "' with a type 'File'.");
        }
    }
}
