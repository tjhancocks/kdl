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

#if !defined(KDL_RESOURCE_INSTANCE_PARSER_HPP)
#define KDL_RESOURCE_INSTANCE_PARSER_HPP

#include "parser/parser.hpp"

namespace kdl::sema {

    class resource_instance_parser
    {
    private:
        parser& m_parser;
        int64_t m_id { INT64_MIN };
        std::optional<std::string> m_name;
        build_target::type_container& m_type;
        std::string m_keyword { "new" };
        std::weak_ptr<target> m_target;

    public:
        resource_instance_parser(parser& parser, build_target::type_container& type, std::weak_ptr<target> target);

        auto set_keyword(const std::string& keyword) -> void;
        auto set_id(const int64_t& id) -> void;
        auto set_name(const std::string& name) -> void;

        auto parse() -> build_target::resource_instance;
    };

}

#endif //KDL_RESOURCE_INSTANCE_PARSER_HPP
