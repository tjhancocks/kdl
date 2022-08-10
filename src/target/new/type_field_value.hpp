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

#include <vector>
#include <optional>
#include <tuple>
#include <map>
#include <unordered_map>
#include "parser/lexeme.hpp"
#include "target/new/kdl_type.hpp"
#include "target/new/binary_type.hpp"
#include "target/new/type_template.hpp"

namespace kdl::build_target
{

    struct type_field_value
    {
    public:
        explicit type_field_value(const lexeme& base_name);

        [[nodiscard]] auto base_name() const -> lexeme;
        [[nodiscard]] auto extended_name(const std::unordered_map<std::string, lexeme>& vars) const -> lexeme;

        [[nodiscard]] auto export_name() const -> std::optional<lexeme>;
        auto set_export_name(const lexeme& name) -> void;

        auto set_explicit_type(const kdl_type& type) -> void;
        [[nodiscard]] auto explicit_type() const -> std::optional<kdl::build_target::kdl_type>;

        auto set_default_value(const lexeme& default_value) -> void;
        [[nodiscard]] auto default_value() const -> std::optional<lexeme>;

        auto set_symbols(const std::vector<std::tuple<lexeme, lexeme>>& symbols) -> void;
        [[nodiscard]] auto symbols() const -> std::vector<std::tuple<lexeme, lexeme>>;
        [[nodiscard]] auto value_for(const lexeme& symbol) const -> lexeme;

        auto set_name_extensions(const std::vector<lexeme>& name_extensions) -> void;

        auto set_conversion_map(const std::tuple<lexeme, lexeme>& map) -> void;
        [[nodiscard]] auto has_conversion_defined() const -> bool;
        [[nodiscard]] auto conversion_input() const -> lexeme;
        [[nodiscard]] auto conversion_output() const -> lexeme;

        auto join_value(const type_field_value& value) -> void;
        [[nodiscard]] auto joined_value_count() const -> std::size_t;
        [[nodiscard]] auto joined_value_at(int i) -> type_field_value;
        [[nodiscard]] auto joined_value_for(const lexeme& symbol) const -> std::optional<std::tuple<int, lexeme>>;

        auto set_assemble_sprite_sheet() -> void;
        [[nodiscard]] auto assemble_sprite_sheet() const -> bool;

    private:
        std::optional<lexeme> m_export_name;
        lexeme m_base_name;
        std::optional<kdl_type> m_explicit_type;
        std::optional<lexeme> m_default_value;
        std::vector<std::tuple<lexeme, lexeme>> m_symbols;
        std::vector<lexeme> m_name_extensions;
        std::optional<std::tuple<lexeme, lexeme>> m_conversion_map;
        std::vector<type_field_value> m_joined_values;
        bool m_assemble_sprite_sheet { false };

    };

}
