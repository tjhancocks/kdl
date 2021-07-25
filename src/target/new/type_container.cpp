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
#include "type_container.hpp"

#include <utility>

// MARK: - Constructors

kdl::build_target::type_container::type_container(const std::string& code)
    : m_code(code), m_name(code)
{

}

kdl::build_target::type_container::type_container(const kdl::lexeme& name, std::string code)
    : m_code(std::move(code)), m_name(name.text())
{

}

kdl::build_target::type_container::type_container(std::string name, std::string code)
    : m_code(std::move(code)), m_name(std::move(name))
{

}

auto kdl::build_target::type_container::empty_clone_of(const type_container& source, const std::map<std::string, std::string>& attributes) -> kdl::build_target::type_container
{
    type_container clone(source.m_name, source.m_code);
    clone.m_fields = source.m_fields;
    clone.m_tmpl = source.m_tmpl;
    clone.m_assertions = source.m_assertions;
    clone.m_attributes = attributes;
    return clone;
}

// MARK: - Accessors

auto kdl::build_target::type_container::name() const -> std::string
{
    return m_name;
}

auto kdl::build_target::type_container::code() const -> std::string
{
    return m_code;
}

// MARK: - Type Template Management

auto kdl::build_target::type_container::internal_template() -> kdl::build_target::type_template&
{
    return m_tmpl;
}

auto kdl::build_target::type_container::set_internal_template(const type_template& tmpl) -> void
{
    m_tmpl = tmpl;
}

// MARK: - Type Field Management

auto kdl::build_target::type_container::add_field(const kdl::lexeme& name) -> kdl::build_target::type_field&
{
    m_fields.emplace_back(type_field(name));
    return m_fields.at(m_fields.size() - 1);
}

auto kdl::build_target::type_container::add_field(const kdl::build_target::type_field& field) -> void
{
    m_fields.emplace_back(field);
}

auto kdl::build_target::type_container::field_named(const kdl::lexeme& name) -> kdl::build_target::type_field
{
    for (auto field : m_fields) {
        if (field.name().text() == name.text()) {
            return field;
        }
    }
    log::fatal_error(name, 1, "The field '" + name.text() + "' could not be found in type '" + m_name + "'");
}

auto kdl::build_target::type_container::all_fields() const -> std::vector<type_field>
{
    return m_fields;
}

// MARK: - Instance

auto kdl::build_target::type_container::new_instance(const int64_t& id, std::optional<std::string> name) -> resource_instance
{
    return resource_instance(id, m_code, name.has_value() ? name.value() : "", m_tmpl);
}

// MARK: - Assertions

auto kdl::build_target::type_container::assertions() const -> std::vector<assertion>
{
    return m_assertions;
}

auto kdl::build_target::type_container::add_assertions(const std::vector<assertion>& assertions) -> void
{
    m_assertions.insert(m_assertions.end(), assertions.begin(), assertions.end());
}

// MARK: - Attributes

auto kdl::build_target::type_container::set_attribute(const std::string& name, const std::string& value) -> void
{
    m_attributes[name] = value;
}

auto kdl::build_target::type_container::attributes() const -> std::map<std::string, std::string>
{
    return m_attributes;
}
