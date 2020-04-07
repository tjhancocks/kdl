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

#if !defined(KDL_RESOURCE_HPP)
#define KDL_RESOURCE_HPP

#include <tuple>
#include <vector>
#include <string>
#include <map>
#include <any>
#include "parser/lexeme.hpp"
#include "target/types.hpp"
#include "target/field_value.hpp"
#include "libGraphite/data/data.hpp"
#include "target/field.hpp"

namespace kdl
{

    class resource
    {
    private:
        int64_t m_id;
        std::string m_name;
        std::string m_type_code;
        std::vector<std::tuple<lexeme, type>> m_template;
        std::map<int, std::any> m_values;
        std::map<std::string, int> m_field_locks;

        auto write(const std::string field, std::any value) -> void;
        auto index_of(const std::string field) const -> int;

    public:
        resource(const std::string code, const int64_t id, const std::string name, std::vector<std::tuple<lexeme, type>> tmpl);

        auto type_code() const -> std::string;
        auto id() const -> int64_t;
        auto name() const -> std::string;

        auto synthesize_variables() const -> std::map<std::string, lexeme>;

        auto acquire_field_value_lock(const lexeme name, const std::size_t base_value = 0) -> int;
        auto name_extensions_for_field(const lexeme name) -> std::map<std::string, lexeme>;

        auto write_byte(const field field, const field_value ref, const uint8_t value) -> void;
        auto write_short(const field field, const field_value ref, const uint16_t value) -> void;
        auto write_long(const field field, const field_value ref, const uint32_t value) -> void;
        auto write_quad(const field field, const field_value ref, const uint64_t value) -> void;
        auto write_signed_byte(const field field, const field_value ref, const int8_t value) -> void;
        auto write_signed_short(const field field, const field_value ref, const int16_t value) -> void;
        auto write_signed_long(const field field, const field_value ref, const int32_t value) -> void;
        auto write_signed_quad(const field field, const field_value ref, const int64_t value) -> void;
        auto write_pstr(const field field, const field_value ref, const std::string value) -> void;
        auto write_cstr(const field field, const field_value ref, const std::string value, const std::size_t length = 0) -> void;
        auto write_data(const field field, const field_value ref, const std::string value) -> void;
        auto write_rect(const field field, const field_value ref, const int16_t t, const int16_t l, const int16_t b, const int16_t r) -> void;

        auto assemble() const -> std::shared_ptr<graphite::data::data>;
    };

};

#endif //KDL_RESOURCE_HPP
