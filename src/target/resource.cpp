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

#include "target/resource.hpp"
#include "libGraphite/data/writer.hpp"

// MARK: - Constructor

kdl::resource::resource(const std::string code, const int64_t id, const std::string name, std::vector<std::tuple<lexeme, type>> tmpl)
    : m_template(tmpl), m_type_code(code), m_id(id), m_name(name)
{

}

// MARK: - Accessors

auto kdl::resource::type_code() const -> std::string
{
    return m_type_code;
}

auto kdl::resource::id() const -> int64_t
{
    return m_id;
}

auto kdl::resource::name() const -> std::string
{
    return m_name;
}

// MARK: - Field Locks

auto kdl::resource::acquire_field_value_lock(const kdl::lexeme name, const std::size_t base_value) -> int
{
    if (m_field_locks.find(name.text()) == m_field_locks.end()) {
        m_field_locks.emplace(name.text(), base_value);
        return base_value;
    }

    auto n = m_field_locks.at(name.text()) + 1;
    m_field_locks[name.text()] = n;
    return n;
}

auto kdl::resource::name_extensions_for_field(const kdl::lexeme name) -> std::map<std::string, lexeme>
{
    std::map<std::string, lexeme> vars;

    if (m_field_locks.find(name.text()) != m_field_locks.end()) {
        vars.emplace("FieldNumber", lexeme(std::to_string(m_field_locks.at(name.text())), lexeme::integer));
    }

    return vars;
}

// MARK: - Helpers

auto kdl::resource::index_of(const std::string field) const -> int
{
    for (auto i = 0; i < m_template.size(); ++i) {
        if (std::get<0>(m_template[i]).text() == field) {
            return i;
        }
    }
    throw std::logic_error("Attempting to write to resource field that does not exist.");
}

// MARK: - Field Setters

auto kdl::resource::write(const std::string field, std::any value) -> void
{
    m_values.emplace(index_of(field), value);
}

auto kdl::resource::write_byte(const field field, const field_value ref, const uint8_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_short(const field field, const field_value ref, const uint16_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_long(const field field, const field_value ref, const uint32_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_quad(const field field, const field_value ref, const uint64_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_signed_byte(const field field, const field_value ref, const int8_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_signed_short(const field field, const field_value ref, const int16_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_signed_long(const field field, const field_value ref, const int32_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_signed_quad(const field field, const field_value ref, const int64_t value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_pstr(const field field, const field_value ref, const std::string value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_cstr(const field field, const field_value ref, const std::string value, const std::size_t length) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), std::tuple(value, length));
}

auto kdl::resource::write_data(const field field, const field_value ref, const std::string value) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), value);
}

auto kdl::resource::write_rect(const field field, const field_value ref, const int16_t t, const int16_t l, const int16_t b, const int16_t r) -> void
{
    write(ref.name(name_extensions_for_field(field.name_lexeme())), std::make_tuple(t, l, b, r));
}

// MARK: - Assembly

auto kdl::resource::assemble() const -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer writer(data);

    for (auto field : m_template) {
        auto type = std::get<1>(field);
        auto field_name = std::get<0>(field);
        auto index = index_of(field_name.text());
        auto wrapped_value = m_values.at(index);

        switch (type & 0xF000) {
            case kdl::HBYT: {
                writer.write_byte(std::any_cast<uint8_t>(wrapped_value));
                break;
            }
            case kdl::HWRD: {
                writer.write_short(std::any_cast<uint16_t>(wrapped_value));
                break;
            }
            case kdl::HLNG: {
                writer.write_long(std::any_cast<uint32_t>(wrapped_value));
                break;
            }
            case kdl::HQAD: {
                writer.write_quad(std::any_cast<uint64_t>(wrapped_value));
                break;
            }
            case kdl::DBYT: {
                writer.write_signed_byte(std::any_cast<int8_t>(wrapped_value));
                break;
            }
            case kdl::DWRD: {
                writer.write_signed_short(std::any_cast<int16_t>(wrapped_value));
                break;
            }
            case kdl::DLNG: {
                writer.write_signed_long(std::any_cast<int32_t>(wrapped_value));
                break;
            }
            case kdl::DQAD: {
                writer.write_signed_quad(std::any_cast<int64_t>(wrapped_value));
                break;
            }
            case kdl::RECT: {
                auto rect = std::any_cast<std::tuple<int16_t, int16_t, int16_t, int16_t>>(wrapped_value);
                writer.write_signed_short(std::get<0>(rect));
                writer.write_signed_short(std::get<1>(rect));
                writer.write_signed_short(std::get<2>(rect));
                writer.write_signed_short(std::get<3>(rect));
                break;
            }
            case kdl::HEXD:
            case kdl::PSTR: {
                writer.write_pstr(std::any_cast<std::string>(wrapped_value));
                break;
            }
            case kdl::Cxxx:
            case kdl::CSTR: {
                auto cstr = std::any_cast<std::tuple<std::string, std::size_t>>(wrapped_value);
                writer.write_cstr(std::get<0>(cstr), std::get<1>(cstr));
                break;
            }
        }
    }

    return data;
}

// MARK: - Variables

auto kdl::resource::synthesize_variables() const -> std::map<std::string, lexeme>
{
    std::map<std::string, lexeme> vars;

    vars.emplace("id", lexeme(std::to_string(m_id), lexeme::res_id));
    vars.emplace("name", lexeme(m_name, lexeme::string));

    for (auto field : m_template) {
        auto type = std::get<1>(field);
        auto field_name = std::get<0>(field);
        auto index = index_of(field_name.text());
        auto wrapped_value = m_values.at(index);


        switch (type & 0xF000) {
            case kdl::HBYT: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<uint8_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::HWRD: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<uint16_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::HLNG: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<uint32_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::HQAD: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<uint64_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::DBYT: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<int8_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::DWRD: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<int16_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::DLNG: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<int32_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::DQAD: {
                vars.emplace(field_name.text(), lexeme(std::to_string(std::any_cast<int64_t>(wrapped_value)), lexeme::integer));
                break;
            }
            case kdl::PSTR:
            case kdl::Cxxx:
            case kdl::CSTR: {
                auto str = std::any_cast<std::tuple<std::string, std::size_t>>(wrapped_value);
                vars.emplace(field_name.text(), lexeme(std::get<0>(str), lexeme::string));
                break;
            }
        }
    }

    return vars;
}
