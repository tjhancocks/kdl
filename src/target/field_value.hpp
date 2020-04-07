// Copyright (c) 2019-2020 Tom Hancocks
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

#if !defined(KDL_FIELD_VALUE_HPP)
#define KDL_FIELD_VALUE_HPP

#include <string>
#include <vector>
#include <optional>
#include <map>
#include "parser/lexeme.hpp"
#include "target/field_value_type.hpp"

namespace kdl
{

    struct field_value
    {
    private:
        lexeme m_name;
        std::vector<lexeme> m_name_extensions;
        std::optional<field_value_type> m_type;
        std::optional<lexeme> m_default;
        std::vector<std::tuple<lexeme, lexeme>> m_symbols;

    public:
        field_value(const lexeme name, std::optional<field_value_type> type, std::optional<lexeme> default_value);

        auto name(std::optional<std::map<std::string, lexeme>> extension_vars = {}) const -> std::string;
        auto base_name_lexeme() const -> lexeme;

        auto add_name_extension(const lexeme var) -> void;

        auto type() const -> std::optional<field_value_type>;
        auto set_type(const field_value_type type) -> void;

        auto default_value() const -> std::optional<lexeme>;
        auto set_default_value(std::optional<lexeme> default_value) -> void;

        auto add_symbol(const lexeme symbol, const lexeme value) -> void;
        auto value_for(const lexeme symbol) const -> lexeme;
        auto symbol_count() const -> std::size_t;
        auto symbol_at(const int i) -> std::tuple<lexeme, lexeme>;
    };

};

#endif //KDL_FIELD_VALUE_HPP
