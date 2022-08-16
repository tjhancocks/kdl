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

#include <utility>
#include "diagnostic/fatal.hpp"

// MARK: - Binary Field Management

auto kdl::build_target::type_template::add_binary_field(const kdl::build_target::type_template::binary_field& field) -> void
{
    m_fields.emplace_back(field);
}

// MARK: - Field Look Up

auto kdl::build_target::type_template::binary_field_count() const -> std::size_t
{
    return m_fields.size();
}

auto kdl::build_target::type_template::binary_field_at(const std::size_t& n) const -> kdl::build_target::type_template::binary_field
{
    return m_fields.at(n);
}

auto kdl::build_target::type_template::binary_field_named(const std::string& name) const -> kdl::build_target::type_template::binary_field
{
    return binary_field_named(lexeme(name, lexeme::identifier));
}

auto kdl::build_target::type_template::binary_field_named(const kdl::lexeme& lx) const -> kdl::build_target::type_template::binary_field
{
    auto i = 0;
    for (const auto& f : m_fields) {
        if (lx.is(f.label.text())) {
            return f;
        }
        else {
            for (const auto& lf : f.list_fields) {
                if (lx.is(lf.label.text())) {
                    return lf;
                }
            }
        }
        ++i;
    }
    log::fatal_error(lx, 1, "Could not find binary field '" + lx.text() + "' inside template.");
}

auto kdl::build_target::type_template::binary_field_index(const std::string& name) const -> int
{
    return binary_field_index(lexeme(name, lexeme::identifier));
}

auto kdl::build_target::type_template::binary_field_index(const kdl::lexeme& lx) const -> int
{
    auto i = 0;
    for (const auto& f : m_fields) {
        if (lx.is(f.label.text())) {
            return i;
        }
        ++i;
    }
    log::fatal_error(lx, 1, "Could not find binary field '" + lx.text() + "' inside template.");
}

auto kdl::build_target::type_template::fields() const -> const std::vector<binary_field>&
{
    return m_fields;
}


auto kdl::build_target::type_template::has_binary_field_named(const lexeme& lx) const -> bool
{
    auto i = 0;
    for (const auto& f : m_fields) {
        if (lx.is(f.label.text())) {
            return true;
        }
        else {
            for (const auto& lf : f.list_fields) {
                if (lx.is(lf.label.text())) {
                    return true;
                }
            }
        }
        ++i;
    }
    return false;
}

auto kdl::build_target::type_template::has_binary_field_named(const std::string& name) const -> bool
{
    return has_binary_field_named(lexeme(name, lexeme::identifier));
}

// MARK: - Binary Field Construction

kdl::build_target::type_template::binary_field::binary_field(const kdl::lexeme& label, const kdl::build_target::binary_type& type)
    : label(label), type(type)
{

}
