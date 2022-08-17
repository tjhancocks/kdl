// Copyright (c) 2022 Tom Hancocks
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

#include <limits>
#include <utility>
#include "target/new/resource.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Construction

kdl::build_target::resource_constructor::resource_constructor(graphite::rsrc::resource::identifier id, const std::string &code, const std::string &name, class type_template tmpl)
    : m_type_code(code), m_id(id), m_name(name), m_tmpl(std::move(tmpl))
{
    construct_root_value_container();
}

kdl::build_target::resource_constructor::resource_constructor(graphite::rsrc::resource::identifier id, const std::string &code, const std::string &name, const std::string &contents)
    : m_type_code(code), m_id(id), m_name(name)
{
    construct_root_value_container();

    // We're pulling contents directly, so we need to synthesize a template to represent a singular CSTR field.
    lexeme data_lx("data", lexeme::identifier);
    type_template::binary_field data_field(data_lx, binary_type::CSTR);
    m_tmpl.add_binary_field(data_field);

    // Add the contents to the field.
    write("data", std::make_tuple(contents.size(), contents));
}

kdl::build_target::resource_constructor::resource_constructor(graphite::rsrc::resource::identifier id, const std::string &code, const std::string &name, const graphite::data::block &data)
    : m_type_code(code), m_id(id), m_name(name)
{
    construct_root_value_container();

    // We're pulling contents directly, so we need to synthesize a template to represent a singular HEXD field.
    lexeme data_lx("data", lexeme::identifier);
    type_template::binary_field data_field(data_lx, binary_type::HEXD);
    m_tmpl.add_binary_field(data_field);

    // Add the contents to the field.
    write("data", std::make_tuple(data.size(), data));
}

// MARK: - Accessors

auto kdl::build_target::resource_constructor::type_code() const -> const std::string&
{
    return m_type_code;
}

auto kdl::build_target::resource_constructor::id() const -> graphite::rsrc::resource::identifier
{
    return m_id;
}

auto kdl::build_target::resource_constructor::name() const -> const std::string &
{
    return m_name;
}

auto kdl::build_target::resource_constructor::attributes() const -> std::unordered_map<std::string, std::string>
{
    return m_attributes;
}

auto kdl::build_target::resource_constructor::set_attributes(const std::unordered_map<std::string, std::string> &attributes) -> void
{
    m_attributes = attributes;
}

auto kdl::build_target::resource_constructor::set_attribute(const std::string &name, const std::string &value) -> void
{
    m_attributes.emplace(std::pair(name, value));
}

auto kdl::build_target::resource_constructor::type_template() const -> const class type_template&
{
    return m_tmpl;
}

// MARK: - Field Counts

auto kdl::build_target::resource_constructor::field_use_count(const lexeme &field) const -> std::int32_t
{
    auto list = std::any_cast<std::vector<value_container *>>(m_values->value);
    for (auto container : list) {
        return container->field_count;
    }
    return 0;
}

auto kdl::build_target::resource_constructor::acquire_field(const lexeme &field, std::int32_t initial_count) -> std::int32_t
{
    if (auto container = value_container_at(field)) {
        if (container->field_count == 0) {
            container->field_count = initial_count - 1;
        }
        return ++container->field_count;
    }
    return initial_count;
}

auto kdl::build_target::resource_constructor::reset_acquisition_locks() -> void
{
    auto list = std::any_cast<std::vector<value_container *>>(m_values->value);
    for (auto container : list) {
        container->field_count = 0;
    }
}

// MARK: - Value Containers

auto kdl::build_target::resource_constructor::construct_root_value_container() -> void
{
    m_values = new value_container();
    m_values->type = value_type::list;
    m_values->value = std::vector<value_container *>();
    m_values->field_count = 1;
}

auto kdl::build_target::resource_constructor::value_container_at(const lexeme &field) -> value_container *
{
    return value_container_at(field.text(), m_values);
}

auto kdl::build_target::resource_constructor::value_container_at(const std::string &field_name, value_container *container) -> value_container *
{
    auto vc = container ?: (m_pushed_container ?: m_values);
    auto list = std::any_cast<std::vector<value_container *>>(vc->value);
    for (auto it : list) {
        if (it->name == field_name) {
            return it;
        }
    }

    // If we reach this point, then we didn't find the value container, so we need to create it.
    auto sub_container = new value_container();
    sub_container->field_count = 0;
    sub_container->type = value_type::single;

    if (m_tmpl.has_binary_field_named(field_name)) {
        auto bin_field = m_tmpl.binary_field_named(field_name);
        sub_container->name = bin_field.label.text();

        if (bin_field.type == binary_type::OCNT) {
            sub_container->type = value_type::list;
            sub_container->value = std::vector<value_container *>();
        }
    }
    else /* TODO: Check if this is a repeated field */ {
        sub_container->name = field_name;
        sub_container->type = value_type::list;
        sub_container->value = std::vector<value_container *>();
    }

    list.emplace_back(sub_container);
    vc->value = list;

    return sub_container;
}

auto kdl::build_target::resource_constructor::const_value_container_at(const std::string &field_name, value_container *container) const -> value_container *
{
    auto vc = container ?: (m_pushed_container ?: m_values);

    auto list = std::any_cast<std::vector<value_container *>>(vc->value);
    for (auto it : list) {
        if (it->name == field_name) {
            return it;
        }
    }

    return nullptr;
}

// MARK: - Lists

auto kdl::build_target::resource_constructor::add_list_element(const lexeme &field, const std::function<auto(resource_constructor *) -> void> &callback) -> void
{
    auto container = value_container_at(field);
    if (container->type == value_type::list) {
        auto list = std::any_cast<std::vector<value_container *>>(container->value);

        auto child = new value_container();
        child->type = value_type::list;
        child->field_count = 0;
        child->name = field.text() + "Child" + std::to_string(list.size());
        child->value = std::vector<value_container *>();

        list.emplace_back(child);
        container->value = list;

        m_pushed_container = child;
        callback(this);
        m_pushed_container = nullptr;
    }
}

// MARK: - Values

auto kdl::build_target::resource_constructor::write(const std::string &field, std::any value) -> void
{
    auto container = value_container_at(field);
    container->value = std::move(value);
}

auto kdl::build_target::resource_constructor::write_byte(const type_field &field, const type_field_value &field_value, std::uint8_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_short(const type_field &field, const type_field_value &field_value, std::uint16_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_long(const type_field &field, const type_field_value &field_value, std::uint32_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_quad(const type_field &field, const type_field_value &field_value, std::uint64_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_signed_byte(const type_field &field, const type_field_value &field_value, std::int8_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_signed_short(const type_field &field, const type_field_value &field_value, std::int16_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_signed_long(const type_field &field, const type_field_value &field_value, std::int32_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_signed_quad(const type_field &field, const type_field_value &field_value, std::int64_t value) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), value);
}

auto kdl::build_target::resource_constructor::write_pstr(const type_field &field, const type_field_value &field_value, const std::string &value, std::size_t len) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), std::tuple(len, value));
}

auto kdl::build_target::resource_constructor::write_cstr(const type_field &field, const type_field_value &field_value, const std::string &value, std::size_t len) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), std::tuple(len, value));
}

auto kdl::build_target::resource_constructor::write_data(const type_field &field, const type_field_value &field_value, const std::vector<char> &data) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), data);
}

auto kdl::build_target::resource_constructor::write_data(const type_field &field, const type_field_value &field_value, const std::vector<std::uint8_t> &data) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), data);
}

auto kdl::build_target::resource_constructor::write_data(const type_field &field, const type_field_value &field_value, const graphite::data::block &data) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), data);
}

auto kdl::build_target::resource_constructor::write_rect(const type_field &field, const type_field_value &field_value, std::int16_t t, std::int16_t l, std::int16_t b, std::int16_t r) -> void
{
    write(field_value.extended_name(available_name_extensions(field)).text(), std::tuple(t, l, b, r));
}

// MARK: - Supporting

auto kdl::build_target::resource_constructor::available_name_extensions(const type_field &field) const -> std::unordered_map<std::string, lexeme>
{
    std::unordered_map<std::string, lexeme> vars;

    auto field_name = field.name();
    if (field.has_repeatable_count_field()) {
        field_name = field.repeatable_count_field();
    }

    if (auto container = const_value_container_at(field_name.text())) {
        vars.emplace("FieldNumber", lexeme(std::to_string(container->field_count), lexeme::integer));
    }

    return vars;
}

auto kdl::build_target::resource_constructor::synthesize_variables(value_container *container) const -> std::unordered_map<std::string, lexeme>
{
    std::unordered_map<std::string, lexeme> vars;

    vars.emplace("id", lexeme(std::to_string(m_id), lexeme::res_id));
    vars.emplace("name", lexeme(m_name, lexeme::string));

    auto list = std::any_cast<std::vector<value_container *>>(container ?: m_values);
    for (auto sub_container : list) {
        if (sub_container->type == value_type::list) {
            auto result = synthesize_variables(sub_container);
            vars.insert(result.begin(), result.end());
        }
        else {
            auto bin_field = m_tmpl.binary_field_named(sub_container->name);
            auto field_name = bin_field.label.text();

            switch (bin_field.type & ~0xFFFUL) {
                case build_target::HBYT: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::uint8_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::HWRD: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::uint16_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::HLNG: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::uint32_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::HQAD: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::uint64_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::DBYT: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::int8_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::DWRD: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::int16_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::DLNG: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::int32_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::DQAD: {
                    vars.emplace(field_name, lexeme(std::to_string(std::any_cast<std::int64_t>(sub_container->value)), lexeme::integer));
                    break;
                }
                case build_target::PSTR:
                case build_target::CSTR:
                case build_target::Cnnn: {
                    auto str = std::any_cast<std::tuple<std::string, std::size_t>>(sub_container->value);
                    vars.emplace(field_name, lexeme(std::get<0>(str), lexeme::string));
                    break;
                }
            }
        }
    }

    return vars;
}

// MARK: - Assembly

auto kdl::build_target::resource_constructor::assemble() -> graphite::data::block
{
    graphite::data::writer writer(graphite::data::byte_order::msb);
    assemble_list(writer, m_values);
    return std::move(*const_cast<graphite::data::block *>(writer.data()));
}

auto kdl::build_target::resource_constructor::assemble_list(graphite::data::writer& writer, value_container *container, const type_template::binary_field* bin_field) -> void
{
    auto& fields = bin_field ? bin_field->list_fields : m_tmpl.fields();

    for (const auto& field : fields) {
        auto type = field.type;
        auto field_name = field.label;
        auto base_value = value_container_at(field_name.text(), container);

        if (!base_value) {
            writer.write_byte(0, build_target::binary_type_base_size(type));
        }
        else if (!base_value->value.has_value()) {
            log::fatal_error(field_name, 1, "Missing value for field '" + field_name.text() + "'.");
        }
        else if (((type & ~0xFFFUL) == build_target::OCNT) && (base_value->type == value_type::list)) {
            auto list = std::any_cast<std::vector<value_container *>>(base_value->value);
            assemble_field(writer, build_target::HWRD, static_cast<std::uint16_t>(list.size()));
            for (auto element : list) {
                assemble_list(writer, element, &field);
            }
        }
        else if (type == build_target::LSTC) {
            continue;
        }
        else if (type == build_target::LSTE) {
            return;
        }
        else {
            // This is a single value...
            assemble_field(writer, type, base_value->value);
        }
    }
}

auto kdl::build_target::resource_constructor::assemble_field(graphite::data::writer &writer, enum binary_type type, const std::any &value) const -> void
{
    switch (type & ~0xFFFUL) {
        case build_target::HBYT: {
            writer.write_byte(std::any_cast<std::uint8_t>(value));
            break;
        }
        case build_target::HWRD: {
            writer.write_short(std::any_cast<std::uint16_t>(value));
            break;
        }
        case build_target::HLNG: {
            writer.write_long(std::any_cast<std::uint32_t>(value));
            break;
        }
        case build_target::HQAD: {
            writer.write_quad(std::any_cast<std::uint64_t>(value));
            break;
        }
        case build_target::DBYT: {
            writer.write_signed_byte(std::any_cast<std::int8_t>(value));
            break;
        }
        case build_target::DWRD: {
            writer.write_signed_short(std::any_cast<std::int16_t>(value));
            break;
        }
        case build_target::DLNG: {
            writer.write_signed_long(std::any_cast<std::int32_t>(value));
            break;
        }
        case build_target::DQAD: {
            writer.write_signed_quad(std::any_cast<std::int64_t>(value));
            break;
        }
        case build_target::RECT: {
            auto rect = std::any_cast<std::tuple<std::int16_t, std::int16_t, std::int16_t, std::int16_t>>(value);
            writer.write_signed_short(std::get<0>(rect));
            writer.write_signed_short(std::get<1>(rect));
            writer.write_signed_short(std::get<2>(rect));
            writer.write_signed_short(std::get<3>(rect));
            break;
        }
        case build_target::HEXD: {
            if (value.type() == typeid(std::vector<char>)) {
                auto bytes = std::any_cast<std::vector<char>>(value);
                writer.write_bytes(bytes);
            }
            else if (value.type() == typeid(std::vector<std::uint8_t>)) {
                auto bytes = std::any_cast<std::vector<std::uint8_t>>(value);
                writer.write_bytes(bytes);
            }
            else if (value.type() == typeid(graphite::data::block)) {
                auto data = std::any_cast<graphite::data::block>(value);
                writer.write_data(&data);
            }
            break;
        }
        case build_target::PSTR: {
            auto pstr = std::any_cast<std::tuple<std::size_t, std::string>>(value);
            writer.write_pstr(std::get<1>(pstr));
            break;
        }
        case build_target::Cnnn:
        case build_target::CSTR: {
            auto cstr = std::any_cast<std::tuple<std::size_t, std::string>>(value);
            writer.write_cstr(std::get<1>(cstr), std::get<0>(cstr));
            break;
        }
        default: {
            throw std::logic_error("Type not handled");
        }
    }
}