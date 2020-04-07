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

#include "target/field.hpp"

// MARK: - Constructor

kdl::field::field(const kdl::lexeme name)
    : m_name(name)
{

}

// MARK: - Accessors

auto kdl::field::name() const -> std::string
{
    return m_name.text();
}

auto kdl::field::name_lexeme() const -> kdl::lexeme
{
    return m_name;
}

// MARK: - Value Management

auto kdl::field::add_value(const field_value value) -> void
{
    m_values.emplace_back(value);
}

auto kdl::field::value_count() const -> std::size_t
{
    return m_values.size();
}

auto kdl::field::value_at(const int i) -> field_value
{
    return m_values[i];
}

// MARK: - Repeatable

auto kdl::field::make_repeatable(const std::size_t lower, const std::size_t upper) -> void
{
    m_repeatable = true;
    m_repeat_lower = lower;
    m_repeat_upper = upper;
}

auto kdl::field::is_repeatable() const -> bool
{
    return m_repeatable;
}

auto kdl::field::repeat_lower_bound() const -> std::size_t
{
    return m_repeat_lower;
}

auto kdl::field::repeat_upper_bound() const -> std::size_t
{
    return m_repeat_upper;
}
