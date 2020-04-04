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

#if !defined(KDL_TEMPLATE_REFERENCE_HPP)
#define KDL_TEMPLATE_REFERENCE_HPP

#include <string>
#include <vector>
#include <optional>
#include "parser/lexeme.hpp"

namespace kdl
{

    struct template_reference
    {
    private:
        lexeme m_name;
        std::optional<lexeme> m_default;
        std::vector<std::tuple<lexeme, lexeme>> m_symbols;

    public:
        template_reference(const lexeme name, std::optional<lexeme> default_value);

        auto name() const -> std::string;
        auto default_value() const -> std::optional<lexeme>;

        auto add_symbol(const lexeme symbol, const lexeme value) -> void;
        auto value_for(const lexeme symbol) const -> lexeme;
        auto symbol_count() const -> std::size_t;
        auto symbol_at(const int i) -> std::tuple<lexeme, lexeme>;
    };

};

#endif //KDL_TEMPLATE_REFERENCE_HPP
