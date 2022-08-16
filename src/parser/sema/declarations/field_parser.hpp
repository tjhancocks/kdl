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

#pragma once

#include "parser/parser.hpp"

namespace kdl::sema
{

    class field_parser
    {
    public:
        field_parser(parser& parser, build_target::type_container& type, build_target::resource_constructor& instance, std::weak_ptr<target> target);

        auto parse() -> void;

        auto apply_defaults_for_field(const build_target::type_field& field_name) -> void;

    private:
        parser& m_parser;
        build_target::type_container& m_type;
        build_target::resource_constructor& m_instance;
        std::weak_ptr<target> m_target;

        auto parse_value(kdl::build_target::type_field& field, kdl::build_target::type_field_value& field_value, std::int32_t field_number = -1) -> void;
        auto parse_explicit_typed_value(kdl::build_target::type_field& field, kdl::build_target::type_field_value& field_value, std::vector<build_target::type_template::binary_field>&) -> void;
        auto parse_implicitly_typed_value(kdl::build_target::type_field& field, kdl::build_target::type_field_value& field_value, std::vector<build_target::type_template::binary_field>&) -> void;
    };

}
