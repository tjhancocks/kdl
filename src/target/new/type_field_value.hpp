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

#if !defined(KDL_TYPE_FIELD_VALUE_HPP)
#define KDL_TYPE_FIELD_VALUE_HPP

#include <vector>
#include <optional>
#include <tuple>
#include <map>
#include "parser/lexeme.hpp"
#include "target/new/kdl_type.hpp"
#include "target/new/binary_type.hpp"
#include "target/new/type_template.hpp"

namespace kdl { namespace build_target {

    struct type_field_value
    {
    private:
        lexeme m_base_name;
        std::optional<kdl_type> m_explicit_type;
        std::optional<lexeme> m_default_value;
        std::vector<std::tuple<lexeme, lexeme>> m_symbols;
        std::vector<lexeme> m_name_extensions;
        std::optional<std::tuple<lexeme, lexeme>> m_conversion_map;
        std::vector<type_field_value> m_joined_values;
        bool m_assemble_sprite_sheet;

    public:
        type_field_value(const lexeme base_name);

        auto base_name() const -> lexeme;
        auto extended_name(const std::map<std::string, lexeme> vars) const -> lexeme;

        auto set_explicit_type(const kdl_type type) -> void;
        auto explicit_type() const -> std::optional<kdl::build_target::kdl_type>;

        auto set_default_value(const lexeme default_value) -> void;
        auto default_value() const -> std::optional<lexeme>;

        auto set_symbols(const std::vector<std::tuple<lexeme, lexeme>> symbols) -> void;
        auto add_symbol(const lexeme symbol, const lexeme value) -> void;
        auto symbols() const -> std::vector<std::tuple<lexeme, lexeme>>;
        auto value_for(const lexeme symbol) const -> lexeme;

        auto set_name_extensions(const std::vector<lexeme> name_extensions) -> void;
        auto add_name_extension(const lexeme ext) -> void;

        auto set_conversion_map(const std::tuple<lexeme, lexeme> map) -> void;
        auto set_conversion_map(const lexeme input, const lexeme output) -> void;
        auto has_conversion_defined() const -> bool;
        auto conversion_input() const -> lexeme;
        auto conversion_output() const -> lexeme;

        auto join_value(const type_field_value value) -> void;
        auto joined_value_count() const -> std::size_t;
        auto joined_value_at(const int i) -> type_field_value;
        auto joined_value_for(const lexeme symbol) const -> std::optional<std::tuple<int, lexeme>>;

        auto set_assemble_sprite_sheet() -> void;
        auto assemble_sprite_sheet() const -> bool;
    };

}};

#endif //KDL_TYPE_FIELD_VALUE_HPP
