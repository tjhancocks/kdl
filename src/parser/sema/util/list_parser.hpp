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

#include <tuple>
#include <optional>
#include <vector>
#include <string>
#include <unordered_map>
#include "parser/parser.hpp"

namespace kdl::sema
{

    class list_parser
    {
    public:
        explicit list_parser(parser& parser, std::weak_ptr<target> target);

        auto set_list_start(enum lexeme::type lx) -> void;
        auto set_list_end(enum lexeme::type lx) -> void;
        auto set_delimiter(enum lexeme::type lx) -> void;

        auto add_valid_list_item(enum lexeme::type lx) -> void;
        auto add_valid_list_item(enum lexeme::type lx, const std::string& text) -> void;

        auto parse(const std::unordered_map<std::string, kdl::lexeme> vars = {}) -> std::vector<lexeme>;

    private:
        parser& m_parser;
        std::shared_ptr<target> m_target;
        enum lexeme::type m_list_start { lexeme::l_angle };
        enum lexeme::type m_list_end { lexeme::r_angle };
        enum lexeme::type m_delimit { lexeme::comma };
        std::vector<std::tuple<enum lexeme::type, std::optional<std::string>>> m_valid_lexemes;

    };

}
