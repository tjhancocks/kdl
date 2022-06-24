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

#include <string>
#include <optional>
#include <vector>
#include "parser/lexeme.hpp"

namespace kdl::build_target {


    struct kdl_type
    {
    public:
        kdl_type() = default;
        kdl_type(bool is_reference, const lexeme& name);

        auto set_reference(bool reference) -> void;
        [[nodiscard]] auto is_reference() const -> bool;

        auto set_name(const lexeme& name) -> void;
        [[nodiscard]] auto name() const -> std::optional<lexeme>;

        auto set_type_hints(const std::vector<lexeme>& hints) -> void;
        [[nodiscard]] auto type_hints() const -> std::vector<lexeme>;

    private:
        bool m_reference { false };
        std::optional<lexeme> m_type_name {};
        std::vector<lexeme> m_type_hints {};

    };

};
