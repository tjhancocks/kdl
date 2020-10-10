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

#if !defined(KDL_FIELD_DEFINITION_PARSER_HPP)
#define KDL_FIELD_DEFINITION_PARSER_HPP

#include "parser/parser.hpp"
#include "target/new/resource_instance.hpp"

namespace kdl::sema {

    class field_parser
    {
    private:
        parser& m_parser;
        build_target::type_container& m_type;
        build_target::resource_instance& m_instance;
        std::weak_ptr<target> m_target;

    public:
        field_parser(parser& parser, build_target::type_container& type, build_target::resource_instance& instance, std::weak_ptr<target> target);

        auto parse() -> void;
        auto apply_defaults_for_field(const lexeme& field_name) -> void;
    };

}

#endif //KDL_FIELD_PARSER_HPP
