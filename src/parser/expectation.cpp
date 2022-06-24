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

#include "parser/expectation.hpp"

// MARK: - Constructors


kdl::sema::expectation::expectation(enum lexeme::type Ty)
    : m_Ty(Ty), m_Tx("")
{

}

kdl::sema::expectation::expectation(const std::string& Tx)
    : m_Ty(lexeme::any), m_Tx(Tx)
{

}

kdl::sema::expectation::expectation(enum kdl::lexeme::type Ty, const std::string& Tx)
    : m_Ty(Ty), m_Tx(Tx)
{

}

// MARK: - Conditions

auto kdl::sema::expectation::to_be(bool r) const -> function
{
    auto& Tx = m_Tx;
    auto& Ty = m_Ty;

    return [Tx, Ty, r] (kdl::lexeme lx) -> bool {
        auto outcome = true;

        if (!Tx.empty() && !lx.is(Tx)) {
            outcome = false;
        }

        if (Ty != lexeme::any && !lx.is(Ty)) {
            outcome = false;
        }

        return (outcome == r);
    };
}

auto kdl::sema::expectation::be_true() const -> function
{
    return to_be(true);
}

auto kdl::sema::expectation::be_false() const -> function
{
    return to_be(false);
}
