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

#pragma once

#include <functional>
#include <string>
#include "parser/lexeme.hpp"

namespace kdl::sema
{

    struct expectation
    {
    public:
        typedef std::function<auto(lexeme) -> bool> function;

    public:
        explicit expectation(enum lexeme::type Ty);
        explicit expectation(const std::string& Tx);
        expectation(enum lexeme::type Ty, const std::string& Tx);

        [[nodiscard]] auto to_be( bool r) const -> function;

        [[nodiscard]] auto be_true() const -> function;
        [[nodiscard]] auto be_false() const -> function;

    private:
        enum lexeme::type m_Ty;
        std::string m_Tx;

    };

}