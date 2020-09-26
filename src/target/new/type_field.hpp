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

#if !defined(KDL_TYPE_FIELD_HPP)
#define KDL_TYPE_FIELD_HPP

#include <optional>
#include <tuple>
#include <vector>
#include <string>
#include "target/new/kdl_type.hpp"
#include "target/new/type_field_value.hpp"
#include "parser/lexeme.hpp"

namespace kdl::build_target {

    struct type_field
    {
    private:
        lexeme m_name;
        std::vector<type_field_value> m_values;
        bool m_repeatable { false };
        int m_repeatable_lower { 0 };
        int m_repeatable_upper { 0 };

    public:
        explicit type_field(lexeme name);
        explicit type_field(const std::string& name);

        [[nodiscard]] auto name() const -> lexeme;

        auto add_value(const type_field_value& value) -> void;
        [[nodiscard]] auto expected_values() const -> std::size_t;
        [[nodiscard]] auto value_at(const int& n) const -> type_field_value;

        auto make_repeatable(const int& lower, const int& upper) -> void;
        [[nodiscard]] auto lower_repeat_bound() const -> int;
        [[nodiscard]] auto upper_repeat_bound() const -> int;
        [[nodiscard]] auto is_repeatable() const -> bool;
    };

};

#endif //KDL_TYPE_FIELD_HPP
