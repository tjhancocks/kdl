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

#if !defined(KDL_RANGE_PARSER_HPP)
#define KDL_RANGE_PARSER_HPP

#include "parser/parser.hpp"

namespace kdl::sema {

    class range_parser
    {
    private:
        parser& m_parser;
        build_target::kdl_type& m_explicit_type;
        build_target::type_field& m_field;
        build_target::type_field_value& m_field_value;
        build_target::type_template::binary_field m_binary_field;
    public:
        range_parser(parser& parser, build_target::type_field& field,
                     build_target::type_field_value& field_value,
                     build_target::type_template::binary_field binary_field,
                     build_target::kdl_type& type);

        auto parse(build_target::resource_instance& instance) -> void;
    };

}

#endif //KDL_RANGE_PARSER_HPP
