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

#include "type_field.hpp"

// MARK: - Constructors

kdl::build_target::type_field::type_field(const lexeme name)
    : m_name(name)
{

}

kdl::build_target::type_field::type_field(const std::string name)
    : m_name(lexeme(name, lexeme::identifier))
{

}

// MARK: - Accessors

auto kdl::build_target::type_field::name() const -> kdl::lexeme
{
    return m_name;
}

// MARK: - Value Management

static inline auto __is_bitmask(const kdl::build_target::type_field_value& value) -> bool
{
    return value.explicit_type().has_value()
        && value.explicit_type().value().name().has_value()
        && value.explicit_type().value().name().value().is("Bitmask")
        && !value.explicit_type().value().is_reference();
}

auto kdl::build_target::type_field::add_value(const kdl::build_target::type_field_value value) -> void
{
    // Merge bitmasks together.
    if (m_values.size() > 0 && __is_bitmask(value) && __is_bitmask(m_values.back())) {
        m_values.back().join_value(value);
        return;
    }
    m_values.emplace_back(value);
}

auto kdl::build_target::type_field::expected_values() const -> std::size_t
{
    return m_values.size();
}

auto kdl::build_target::type_field::value_at(const int n) const -> kdl::build_target::type_field_value
{
    return m_values.at(n);
}

// MARK: - Repeatable

auto kdl::build_target::type_field::make_repeatable(const int lower, const int upper) -> void
{
    m_repeatable_lower = lower;
    m_repeatable_upper = upper;
    m_repeatable = true;
}

auto kdl::build_target::type_field::lower_repeat_bound() const -> int
{
    return m_repeatable_lower;
}

auto kdl::build_target::type_field::upper_repeat_bound() const -> int
{
    return m_repeatable_upper;
}

auto kdl::build_target::type_field::is_repeatable() const -> bool
{
    return m_repeatable;
}
