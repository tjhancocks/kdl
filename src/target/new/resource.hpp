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

#pragma once

#include <any>
#include <string>
#include <optional>
#include <string>
#include <unordered_map>
#include "parser/lexeme.hpp"
#include "target/new/type_template.hpp"
#include "target/new/type_field.hpp"
#include "target/new/binary_type.hpp"
#include "libGraphite/data/data.hpp"
#include "libGraphite/data/writer.hpp"

namespace kdl::build_target
{
    class resource_constructor
    {
    public:
        resource_constructor(graphite::rsrc::resource::identifier id, const std::string& code, const std::string& name, type_template tmpl);
        resource_constructor(graphite::rsrc::resource::identifier id, const std::string& code, const std::string& name, const std::string& contents);
        resource_constructor(graphite::rsrc::resource::identifier id, const std::string& code, const std::string& name, const graphite::data::block& data);

    private:
        enum class value_type { single, list };

        struct value_container {
            std::string name;
            enum value_type type { value_type::single };
            std::any value;
            std::int32_t field_count { 0 };
        };

    public:
        [[nodiscard]] auto type_code() const -> const std::string&;
        [[nodiscard]] auto id() const -> graphite::rsrc::resource::identifier;
        [[nodiscard]] auto name() const -> const std::string&;

        auto value_container_at(const std::string& path, value_container *container = nullptr) -> value_container *;
        auto value_container_at(const lexeme& field) -> value_container *;

        auto add_list_element(const lexeme& field, const std::function<auto(resource_constructor *)->void>& callback) -> void;

        auto write_byte(const type_field& field, const type_field_value& field_value, std::uint8_t value) -> void;
        auto write_short(const type_field& field, const type_field_value& field_value, std::uint16_t value) -> void;
        auto write_long(const type_field& field, const type_field_value& field_value, std::uint32_t value) -> void;
        auto write_quad(const type_field& field, const type_field_value& field_value, std::uint64_t value) -> void;
        auto write_signed_byte(const type_field& field, const type_field_value& field_value, std::int8_t value) -> void;
        auto write_signed_short(const type_field& field, const type_field_value& field_value, std::int16_t value) -> void;
        auto write_signed_long(const type_field& field, const type_field_value& field_value, std::int32_t value) -> void;
        auto write_signed_quad(const type_field& field, const type_field_value& field_value, std::int64_t value) -> void;

        auto write_pstr(const type_field& field, const type_field_value& field_value, const std::string& value, std::size_t len = 0) -> void;
        auto write_cstr(const type_field& field, const type_field_value& field_value, const std::string& value, std::size_t len = 0) -> void;
        auto write_data(const type_field& field, const type_field_value& field_value, const std::vector<char>& data) -> void;
        auto write_data(const type_field& field, const type_field_value& field_value, const std::vector<std::uint8_t>& data) -> void;
        auto write_data(const type_field& field, const type_field_value& field_value, const graphite::data::block& data) -> void;
        auto write_rect(const type_field& field, const type_field_value& field_value, std::int16_t t, std::int16_t l, std::int16_t b, std::int16_t r) -> void;

        auto write(const std::string& field, std::any value) -> void;

        auto assemble() -> graphite::data::block;
        [[nodiscard]] auto synthesize_variables(value_container *container = nullptr) const -> std::unordered_map<std::string, lexeme>;

        auto set_attributes(const std::unordered_map<std::string, std::string>& attributes) -> void;
        auto set_attribute(const std::string& name, const std::string& value) -> void;
        [[nodiscard]] auto attributes() const -> std::unordered_map<std::string, std::string>;

        [[nodiscard]] auto field_use_count(const lexeme& field) const -> std::int32_t;
        [[nodiscard]] auto acquire_field(const lexeme& field, std::int32_t initial_count = 0) -> std::int32_t;
        auto reset_acquisition_locks() -> void;

        [[nodiscard]] auto type_template() const -> const type_template&;

    private:
        value_container *m_values { nullptr };
        value_container *m_pushed_container { nullptr };
        std::string m_type_code;
        graphite::rsrc::resource::identifier m_id;
        std::string m_name;
        class type_template m_tmpl;
        std::unordered_map<std::string, std::string> m_attributes;

        auto construct_root_value_container() -> void;
        [[nodiscard]] auto available_name_extensions(const type_field& field) const -> std::unordered_map<std::string, lexeme>;

        [[nodiscard]] auto const_value_container_at(const std::string& path, value_container *container = nullptr) const -> value_container *;

        auto assemble_list(graphite::data::writer& writer, value_container *container, const type_template::binary_field* bin_field = nullptr) -> void;
        auto assemble_field(graphite::data::writer& writer, enum binary_type type, const std::any& value) const -> void;
    };
}
