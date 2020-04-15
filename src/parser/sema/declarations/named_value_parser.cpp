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
#include "parser/sema/declarations/named_value_parser.hpp"
#include "parser/sema/declarations/named_types/file_type_parser.hpp"
#include "parser/sema/declarations/named_types/bitmask_parser.hpp"
#include "parser/sema/declarations/named_types/range_parser.hpp"

// MARK: - Constructor

kdl::sema::named_value_parser::named_value_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                                  kdl::build_target::type_field_value &field_value,
                                                  std::vector<kdl::build_target::type_template::binary_field> binary_fields,
                                                  kdl::build_target::kdl_type &type,
                                                  std::weak_ptr<kdl::target> target)
    : m_parser(parser),
      m_explicit_type(type),
      m_field(field),
      m_binary_fields(binary_fields),
      m_field_value(field_value),
      m_target(target)
{

}

// MARK: - Parser

auto kdl::sema::named_value_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    auto type_name = m_explicit_type.name().value();

    if (type_name.is("File")) {
        file_type_parser(m_parser, m_field, m_field_value, m_binary_fields.back(), m_explicit_type, m_target)
                .parse(instance);
    }
    else if (type_name.is("Bitmask")) {
        bitmask_parser(m_parser, m_field, m_field_value, m_binary_fields, m_explicit_type)
                .parse(instance);
    }
    else if (type_name.is("Range")) {
        range_parser(m_parser, m_field, m_field_value, m_binary_fields.back(), m_explicit_type)
                .parse(instance);
    }
    else if (type_name.is("Color")) {

    }
    else {
        log::fatal_error(type_name, 1, "Unrecognised type name '" + type_name.text() + "'");
    }
}
