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

kdl::field::field(const kdl::lexeme name, const std::string type, const bool is_reference)
    : m_name(name), m_type(type, is_reference)
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

auto kdl::field::type() const -> std::tuple<std::string, bool>
{
    return m_type;
}

// MARK: - Value Management

auto kdl::field::add_value(const template_reference value) -> void
{
    m_values.emplace_back(value);
}

auto kdl::field::value_count() const -> std::size_t
{
    return m_values.size();
}

auto kdl::field::value_at(const int i) -> template_reference
{
    return m_values[i];
}