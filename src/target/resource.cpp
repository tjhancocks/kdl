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

kdl::resource::resource(const std::string code, std::vector<std::tuple<lexeme, type>> tmpl)
    : m_template(tmpl), m_type_code(code)
{

}

// MARK: - Helpers

auto kdl::resource::index_of(const lexeme field) const -> int
{
    for (auto i = 0; i < m_template.size(); ++i) {
        if (std::get<0>(m_template[i]).text() == field.text()) {
            return i;
        }
    }
    throw std::logic_error("Attempting to write to resource field that does not exist.");
}

// MARK: - Field Setters

auto kdl::resource::write(const lexeme field, std::any value) -> void
{
    m_values.emplace(index_of(field), value);
}

auto kdl::resource::write_byte(const template_reference ref, const uint8_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_short(const template_reference ref, const uint16_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_long(const template_reference ref, const uint32_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_quad(const template_reference ref, const uint64_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_signed_byte(const template_reference ref, const int8_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_signed_short(const template_reference ref, const int16_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_signed_long(const template_reference ref, const int32_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_signed_quad(const template_reference ref, const int64_t value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_pstr(const template_reference ref, const std::string value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_cstr(const template_reference ref, const std::string value, const std::size_t length) -> void
{
    write(ref.name_lexeme(), std::tuple(value, length));
}

auto kdl::resource::write_data(const template_reference ref, const std::string value) -> void
{
    write(ref.name_lexeme(), value);
}

auto kdl::resource::write_rect(const template_reference ref, const int16_t t, const int16_t l, const int16_t b, const int16_t r) -> void
{
    write(ref.name_lexeme(), std::make_tuple(t, l, b, r));
}

// MARK: - Assembly

auto kdl::resource::assemble() const -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer writer(data);

    for (auto field : m_template) {
        auto type = std::get<1>(field);
        auto field_name = std::get<0>(field);
        auto index = index_of(field_name);
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
