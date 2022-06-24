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

    class resource_instance
    {
    public:
        resource_instance(graphite::rsrc::resource::identifier id, const std::string& code, const std::string& name, kdl::build_target::type_template tmpl);
        resource_instance(graphite::rsrc::resource::identifier id, const std::string& code, const std::string& name, const std::string& contents);
        resource_instance(graphite::rsrc::resource::identifier id, const std::string& code, const std::string& name, const graphite::data::block& data);

        [[nodiscard]] auto type_code() const -> const std::string&;
        [[nodiscard]] auto id() const -> graphite::rsrc::resource::identifier;
        [[nodiscard]] auto name() const -> const std::string&;

        auto set_attributes(const std::unordered_map<std::string, std::string>& attributes) -> void;
        auto set_attribute(const std::string& name, const std::string& value) -> void;
        [[nodiscard]] auto attributes() const -> std::unordered_map<std::string, std::string>;

        [[nodiscard]] auto get_type_template() const -> const kdl::build_target::type_template&;

        [[nodiscard]] auto field_use_count(const lexeme& field) const -> int;
        [[nodiscard]] auto acquire_field(const lexeme& field, const int& initial_count = 0) -> int;
        auto reset_acquisition_locks() -> void;

        auto write_byte(const type_field& field, const type_field_value& field_value, const uint8_t& value) -> void;
        auto write_short(const type_field& field, const type_field_value& field_value, const uint16_t& value) -> void;
        auto write_long(const type_field& field, const type_field_value& field_value, const uint32_t& value) -> void;
        auto write_quad(const type_field& field, const type_field_value& field_value, const uint64_t& value) -> void;
        auto write_signed_byte(const type_field& field, const type_field_value& field_value, const int8_t& value) -> void;
        auto write_signed_short(const type_field& field, const type_field_value& field_value, const int16_t& value) -> void;
        auto write_signed_long(const type_field& field, const type_field_value& field_value, const int32_t& value) -> void;
        auto write_signed_quad(const type_field& field, const type_field_value& field_value, const int64_t& value) -> void;

        auto write_pstr(const type_field& field, const type_field_value& field_value, const std::string& value, const std::size_t& len = 0) -> void;
        auto write_wstr(const type_field& field, const type_field_value& field_value, const std::string& value) -> void;
        auto write_lstr(const type_field& field, const type_field_value& field_value, const std::string& value) -> void;
        auto write_cstr(const type_field& field, const type_field_value& field_value, const std::string& value, const std::size_t& len = 0) -> void;
        auto write_data(const type_field& field, const type_field_value& field_value, const std::vector<char>& data) -> void;
        auto write_data(const type_field& field, const type_field_value& field_value, const std::vector<uint8_t>& data) -> void;
        auto write_data(const type_field& field, const type_field_value& field_value, const graphite::data::block& data) -> void;
        auto write_rect(const type_field& field, const type_field_value& field_value, const int16_t& t, const int16_t& l, const int16_t &b, const int16_t& r) -> void;

        auto write(const int& field_index, const std::any& value) -> void;

        [[nodiscard]] auto assemble() const -> graphite::data::block;
        [[nodiscard]] auto synthesize_variables() const -> std::unordered_map<std::string, lexeme>;

    private:
        graphite::rsrc::resource::identifier m_id { INT64_MIN };
        std::string m_code { "NULL" };
        std::string m_name;
        std::map<std::string, int> m_field_counts;
        type_template m_tmpl;
        std::unordered_map<int, std::vector<std::any>> m_values;
        std::unordered_map<std::string, std::string> m_attributes;

        auto assemble_field(graphite::data::writer& writer, const enum binary_type& type, const std::any& wrapped_value) const -> void;
        auto write(const std::string& field, const std::any& value) -> void;
        [[nodiscard]] auto index_of(const std::string& field) const -> int;
        [[nodiscard]] auto available_name_extensions(const type_field& field) const -> std::unordered_map<std::string, lexeme>;

    };

};
