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

#if !defined(KDL_CONTAINER_HPP)
#define KDL_CONTAINER_HPP

#include <string>
#include <tuple>
#include <vector>
#include <map>
#include "target/types.hpp"
#include "target/field.hpp"

namespace kdl
{

    class container
    {
    private:
        lexeme m_name;
        std::string m_code;
        std::vector<std::tuple<lexeme, type>> m_template;
        std::vector<field> m_fields;

    public:
        container(const lexeme name, const std::string code);

        auto name() const -> std::string;
        auto code() const -> std::string;

        auto add_template_field(const lexeme label, enum type type) -> void;
        auto template_field_count() const -> std::size_t;
        auto template_field_at(const int i) const -> std::tuple<std::string, type>;
        auto template_field_named(const lexeme name) const -> std::tuple<std::string, type>;

        auto add_field(const field field) -> void;
        auto field_count() const -> std::size_t;
        auto field_at(const int i) const -> field;
        auto field_named(const lexeme name) -> field;
    };

};

#endif //KDL_CONTAINER_HPP
