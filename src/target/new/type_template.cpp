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

#include "type_template.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Binary Field Management

auto kdl::target::type_template::add_binary_field(const kdl::target::type_template::binary_field field) -> void
{
    m_fields.emplace_back(field);
}

auto kdl::target::type_template::add_binary_field(const std::string label, const kdl::target::binary_type type) -> void
{
    binary_field field;
    field.label = label;
    field.type = type;
    add_binary_field(field);
}

// MARK: - Field Look Up

auto kdl::target::type_template::binary_field_count() const -> std::size_t
{
    return m_fields.size();
}

auto kdl::target::type_template::binary_field_at(const std::size_t n) const -> kdl::target::type_template::binary_field
{
    return m_fields.at(n);
}

auto kdl::target::type_template::binary_field_named(const std::string name) const -> kdl::target::type_template::binary_field
{
    binary_field_named(lexeme(name, lexeme::identifier));
}

auto kdl::target::type_template::binary_field_named(const kdl::lexeme lx) -> kdl::target::type_template::binary_field
{
    for (auto f : m_fields) {
        if (lx.is(f.label)) {
            return f;
        }
    }
    log::fatal_error(lx, 1, "Could not find binary field '" + lx.text() + "' inside template.");
}
