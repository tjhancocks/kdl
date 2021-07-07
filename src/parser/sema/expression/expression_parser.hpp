// Copyright (c) 2021 Tom Hancocks
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

#if !defined(KDL_EXPRESSION_PARSER_HPP)
#define KDL_EXPRESSION_PARSER_HPP

#include "parser/parser.hpp"

namespace kdl::sema
{

    class expression_parser
    {
    private:
        parser& m_parser;
        std::shared_ptr<target> m_target;
        const std::map<std::string, kdl::lexeme>& m_vars;

        [[nodiscard]] auto resolve_variable_named(const kdl::lexeme& var) const -> kdl::lexeme;

    public:
        expression_parser(parser& parser, std::weak_ptr<target> target, const std::map<std::string, kdl::lexeme>& vars);

        auto parse() -> kdl::lexeme;

    };

}

#endif //KDL_EXPRESSION_PARSER_HPP
