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

#include "target/field_value_type.hpp"

// MARK: - Constructors

kdl::field_value_type::field_value_type(const bool reference)
    : m_is_reference(reference)
{

};

kdl::field_value_type::field_value_type(const lexeme name, const bool reference)
    : m_name(name), m_is_reference(reference)
{

}

// MARK: - Accessors

auto kdl::field_value_type::name() const -> std::string
{
    return m_name.has_value() ? m_name->text() : "Any";
}

auto kdl::field_value_type::is_reference() const -> bool
{
    return m_is_reference;
}

auto kdl::field_value_type::name_lexeme() const -> std::optional<lexeme>
{
    return m_name;
}

