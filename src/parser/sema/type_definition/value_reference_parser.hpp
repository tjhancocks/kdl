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

#if !defined(KDL_VALUE_REFERENCE_PARSER_HPP)
#define KDL_VALUE_REFERENCE_PARSER_HPP

#include <stdexcept>
#include "parser/parser.hpp"
#include "target/new/type_field_value.hpp"
#include "target/new/type_container.hpp"

namespace kdl::sema
{

    class value_reference_parser
    {
    private:
        parser& m_parser;
        std::shared_ptr<target> m_target;
        build_target::type_template m_tmpl;

    public:
        value_reference_parser(parser& parser, std::weak_ptr<target> target, build_target::type_template tmpl);

        auto parse() -> build_target::type_field_value;
    };

}

#endif //KDL_VALUE_REFERENCE_PARSER_HPP
