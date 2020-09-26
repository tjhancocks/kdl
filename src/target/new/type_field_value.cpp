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

#include "diagnostic/fatal.hpp"
#include "type_field_value.hpp"

kdl::build_target::type_field_value::type_field_value(kdl::lexeme base_name)
    : m_base_name(std::move(base_name))
{

}

// MARK: - Accessors

auto kdl::build_target::type_field_value::base_name() const -> lexeme
{
    return m_base_name;
}

auto kdl::build_target::type_field_value::extended_name(const std::map<std::string, lexeme>& vars) const -> lexeme
{
    std::string name(m_base_name.text());
    for (const auto& ext : m_name_extensions) {
        if (vars.find(ext.text()) != vars.end()) {
            name.append(vars.at(ext.text()).text());
        }
    }
    return lexeme(name, lexeme::identifier);
}

// MARK: - Explicit Types

auto kdl::build_target::type_field_value::set_explicit_type(const kdl::build_target::kdl_type& type) -> void
{
    m_explicit_type = type;
}

auto kdl::build_target::type_field_value::explicit_type() const -> std::optional<kdl::build_target::kdl_type>
{
    return m_explicit_type;
}

// MARK: - Default Value

auto kdl::build_target::type_field_value::set_default_value(const kdl::lexeme& default_value) -> void
{
    m_default_value = default_value;
}

auto kdl::build_target::type_field_value::default_value() const -> std::optional<lexeme>
{
    return m_default_value;
}

// MARK: - Name Extensions

auto kdl::build_target::type_field_value::set_name_extensions(const std::vector<lexeme>& name_extensions) -> void
{
    m_name_extensions = name_extensions;
}

// MARK: - Symbols

auto kdl::build_target::type_field_value::set_symbols(const std::vector<std::tuple<lexeme, lexeme>>& symbols) -> void
{
    m_symbols = symbols;
}

auto kdl::build_target::type_field_value::symbols() const -> std::vector<std::tuple<lexeme, lexeme>>
{
    return m_symbols;
}

auto kdl::build_target::type_field_value::value_for(const lexeme& symbol) const -> kdl::lexeme
{
    for (auto value : m_symbols) {
        if (std::get<0>(value).is(symbol.text())) {
            return std::get<1>(value);
        }
    }
    log::fatal_error(symbol, 1, "Unrecognised symbol name '" + symbol.text() + "'");
}

// MARK: - Conversions

auto kdl::build_target::type_field_value::set_conversion_map(const std::tuple<lexeme, lexeme>& map) -> void
{
    m_conversion_map = map;
}

auto kdl::build_target::type_field_value::has_conversion_defined() const -> bool
{
    return m_conversion_map.has_value();
}

auto kdl::build_target::type_field_value::conversion_input() const -> lexeme
{
    return std::get<0>(m_conversion_map.value());
}

auto kdl::build_target::type_field_value::conversion_output() const -> lexeme
{
    return std::get<1>(m_conversion_map.value());
}

// MARK: - Joined Values

auto kdl::build_target::type_field_value::join_value(const kdl::build_target::type_field_value& value) -> void
{
    m_joined_values.emplace_back(value);
}

auto kdl::build_target::type_field_value::joined_value_count() const -> std::size_t
{
    return m_joined_values.size();
}

auto kdl::build_target::type_field_value::joined_value_for(const kdl::lexeme& symbol) const -> std::optional<std::tuple<int, lexeme>>
{
    // Check if the symbol is in this. If it is return an empty optional.
    for (auto value : m_symbols) {
        if (std::get<0>(value).is(symbol.text())) {
            return {};
        }
    }

    // Now check through each of the joined values for the symbol, return the type_field_value instance and the value
    // of the symbol, if it is found.
    int i = 0;
    for (const auto& field_value : m_joined_values) {
        for (auto value : field_value.m_symbols) {
            if (std::get<0>(value).is(symbol.text())) {
                return std::tuple(i, std::get<1>(value));
            }
        }
        ++i;
    }

    log::fatal_error(symbol, 1, "Unrecognised symbol name '" + symbol.text() + "'");
}

auto kdl::build_target::type_field_value::joined_value_at(const int& i) -> kdl::build_target::type_field_value
{
    return m_joined_values.at(i);
}

// MARK: - Sprite Sheet Assembling

auto kdl::build_target::type_field_value::set_assemble_sprite_sheet() -> void
{
    m_assemble_sprite_sheet = true;
}

auto kdl::build_target::type_field_value::assemble_sprite_sheet() const -> bool
{
    return m_assemble_sprite_sheet;
}
