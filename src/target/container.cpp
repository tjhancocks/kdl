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

#include "diagnostic/fatal.hpp"
#include "target/container.hpp"

// MARK: - Constructor

kdl::container::container(const lexeme name, const std::string code)
    : m_name(name), m_code(code)
{

}

// MARK: - Accessors

auto kdl::container::name() const -> std::string
{
    return m_name.text();
}

auto kdl::container::code() const -> std::string
{
    return m_code;
}

// MARK: - Template Management

auto kdl::container::add_template_field(const lexeme label, kdl::type type) -> void
{
    // Quickly scan for duplicate names.
    for (auto t : m_template) {
        if (std::get<0>(t).text() == label.text()) {
            log::fatal_error(label, 1, "Template field is using a duplicated name: '" + label.text() + "'");
        }
    }

    m_template.emplace_back(std::make_tuple(label, type));
}

auto kdl::container::template_field_count() const -> std::size_t
{
    return m_template.size();
}

auto kdl::container::template_field_at(const int i) const -> std::tuple<std::string, type>
{
    auto t = m_template[i];
    return std::make_tuple(std::get<0>(t).text(), std::get<1>(t));
}

auto kdl::container::template_field_named(const lexeme name) const -> std::tuple<std::string, type>
{
    for (auto t : m_template) {
        if (std::get<0>(t).text() == name.text()) {
            return std::make_tuple(std::get<0>(t).text(), std::get<1>(t));
        }
    }
    log::fatal_error(name, 1, "Missing template field named '" + name.text() + "'");
}

// MARK: - Field Management

auto kdl::container::add_field(const kdl::field field) -> void
{
    for (auto f : m_fields) {
        if (f.name() == field.name()) {
            log::fatal_error(field.name_lexeme(), 1, "Field is using a duplicated name '" + field.name() + "'");
        }
    }
    m_fields.emplace_back(field);
}

auto kdl::container::field_count() const -> std::size_t
{
    return m_fields.size();
}

auto kdl::container::field_at(const int i) const -> kdl::field
{
    return m_fields[i];
}

auto kdl::container::field_named(const kdl::lexeme name) -> kdl::field
{
    for (auto f : m_fields) {
        if (f.name() == name.text()) {
            return f;
        }
    }
    log::fatal_error(name, 1, "Missing field named '" + name.text() + "'");
}
