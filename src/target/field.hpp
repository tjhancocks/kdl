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

#if !defined(KDL_FIELD_HPP)
#define KDL_FIELD_HPP

#include <string>
#include <vector>
#include <tuple>
#include <optional>
#include "parser/lexeme.hpp"
#include "target/template_reference.hpp"

namespace kdl
{

    struct field
    {
    private:
        lexeme m_name;
        std::optional<std::tuple<std::string, bool>> m_type;
        std::vector<template_reference> m_values;

    public:
        field(const lexeme name);
        field(const lexeme name, const std::string type, const bool is_reference);

        auto name() const -> std::string;
        auto name_lexeme() const -> lexeme;
        auto type() const -> std::optional<std::tuple<std::string, bool>>;

        auto set_type(const std::string type, const bool is_reference) -> void;

        auto add_value(const template_reference value) -> void;
        auto value_count() const -> std::size_t;
        auto value_at(const int i) -> template_reference;
    };

};

#endif //KDL_FIELD_HPP
